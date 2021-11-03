#define _GNU_SOURCE

/* Ease development on macOS */
#ifdef __APPLE__
# define PTRACE_SETOPTIONS 0
# define PTRACE_TRACEME 0
# define PTRACE_SYSCALL 0
# define PTRACE_PEEKUSER 0
# define PTRACE_POKEUSER 0
# define PTRACE_PEEKTEXT 0
# define PTRACE_POKETEXT 0
# define PTRACE_GETREGS 0
# define PTRACE_SETREGS 0
# define PTRACE_O_TRACESYSGOOD 0
# define PTRACE_O_EXITKILL 0
# define ORIG_RAX 0
# define RAX 0
# define RSP 0
# define RDI 0
# define RSI 0
# define RDX 0
# define R10 0
# define R8 0
# define R9 0

typedef struct user_regs_struct { };

#else
# include <sys/reg.h>
#endif

#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>

#define MAP_HIDE_SUFFIX "hide.so"

#define WIFSYSCALLED(st) (WIFSTOPPED(st) && WSTOPSIG(st) & (SIGTRAP | 0x80))

#ifdef __i386__

typedef uint32_t	reg_t;

# define PRIdREG PRId32
# define PRIiREG PRIi32
# define PRIuREG PRIu32
# define PRIoREG PRIo32
# define PRIxREG PRIx32
# define PRIXREG PRIX32

# define SYSCALL ORIG_EAX
# define RET EAX
# define SP UESP
# define ARG0 EBX
# define ARG1 ECX
# define ARG2 EDX
# define ARG3 ESI
# define ARG4 EDI
# define ARG5 EBP

#else

typedef uint64_t	reg_t;

# define PRIdREG PRId64
# define PRIiREG PRIi64
# define PRIuREG PRIu64
# define PRIoREG PRIo64
# define PRIxREG PRIx64
# define PRIXREG PRIX64

# define RET RAX
# define SP RSP
# define ARG0 RDI
# define ARG1 RSI
# define ARG2 RDX
# define ARG3 R10
# define ARG4 R8
# define ARG5 R9
# define SYSCALL ORIG_RAX

#endif

#define REDZONE_SIZE 128

#ifndef TRACEE_OPTIONS
# define TRACEE_OPTIONS (PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL)
#endif

#ifndef O_TMPFILE
# define O_TMPFILE 0
#endif

#ifndef PTRACE_O_EXITKILL
# define PTRACE_O_EXITKILL 0
#endif

struct syscall_regs
{
	reg_t	ret;
	reg_t	av[6];
	reg_t	no;
};

static unsigned	word_len(reg_t word)
{
	unsigned	len;

	for (len = 0; len < sizeof(word) && ((char*)&word)[len] != '\0'; len++)
		;

	return len;
}

static int	text_len(pid_t pid, reg_t addr)
{
	reg_t	word;
	int		w_len = sizeof(word);
	int		len = 0;

	do
	{
		word = ptrace(PTRACE_PEEKTEXT, pid, addr + len, NULL);

		w_len = -(errno != 0);
		if (w_len == 0)
		{
			w_len = (int)word_len(word);
			len += w_len;
		}
		else
			perror("ptrace: peektext");
	}
	while (w_len == sizeof(word));

	return len;
}

static char	*text_dup(pid_t pid, reg_t addr)
{
	size_t		len = text_len(pid, addr);
	char		*str = malloc(len + 1);
	char		*dst;
	reg_t		word;
	unsigned	w_len = sizeof(word);

	if (str != NULL)
	{
		dst = str;
		do
		{
			word = ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);

			if (errno == 0)
			{
				if (len < w_len)
					w_len = len;

				memcpy(dst, &word, w_len);

				len -= w_len;
				dst += w_len;
				addr += w_len;
			}
			else
				len = -1;
		} while (len > 0);

		if (len == 0)
			*dst = '\0';
		else
		{
			perror("ptrace: peektext");
			free(str);
			str = NULL;
		}
	}
	else
		perror("malloc");

	return str;
}

static int	text_poke(pid_t pid, reg_t addr, const void *data, size_t size)
{
	reg_t		word;
	size_t		i;
	unsigned	w_len = sizeof(word);

	i = 0;
	do
	{
		if (size < w_len)
		{
			memset(&word, 0, sizeof(word));
			w_len = size;
		}


		memcpy(&word, (reg_t *)data + i, w_len);

		//fprintf(stderr, "Writing: '%.*s'\n to %p\n", (int)w_len, (char*)&word, (reg_t *)addr + i);

		ptrace(PTRACE_POKETEXT, pid, (reg_t *)addr + i, word);

		size -= w_len;
		i++;
	} while (size > 0);

	return (int)size;
}

static int	syscall_regs_get(pid_t pid, struct syscall_regs *regs)
{
	struct user_regs_struct	uregs;
	int						ret;

	ret = ptrace(PTRACE_GETREGS, pid, NULL, &uregs);

	if (ret == 0)
	{
		*regs = (struct syscall_regs)
		{
			((reg_t*)&uregs)[RET],
			{
				((reg_t*)&uregs)[ARG0], ((reg_t*)&uregs)[ARG1],
				((reg_t*)&uregs)[ARG2], ((reg_t*)&uregs)[ARG3],
				((reg_t*)&uregs)[ARG4], ((reg_t*)&uregs)[ARG5]
			},
			((reg_t*)&uregs)[SYSCALL]
		};
	}
	else
		perror("ptrace: getregs");

	return ret;
}

static int	syscall_get(pid_t pid)
{
	return ptrace(PTRACE_PEEKUSER, pid, SYSCALL * sizeof(reg_t), NULL);
}

static int	syscall_open_print(pid_t pid)
{
	struct syscall_regs	regs;
	char				*str;
	int					ret;
	mode_t				mode;

	syscall_regs_get(pid, &regs);

	str = text_dup(pid, regs.av[0]);

	if (str != NULL)
	{
		if ((unsigned)regs.av[1] & (O_CREAT | O_TMPFILE))
			mode = (mode_t)regs.av[2];
		else
			mode = 0;
		ret = fprintf(stderr, "open(%s, %u, %u)\n",
			str, (unsigned)regs.av[1], mode);
		free(str);
	}
	else
		ret = -1;

	return ret;
}

static int	syscall_wait(pid_t pid, int no)
{
	int	status;

	do
	{
		ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
		waitpid(pid, &status, 0);
	}
	while (!((WIFSYSCALLED(status) && syscall_get(pid) == no)
	|| WIFEXITED(status) || WIFSIGNALED(status)));

	return status;
}

static int	syscall_wait_open(int pid, const char *filename)
{
	int		status;
	int		syscalled;
	reg_t	arg0;
	char	*orig_filename = NULL;

	do
	{
		free(orig_filename);
		orig_filename = NULL;

		status = syscall_wait(pid, SYS_open);
		syscalled = WIFSYSCALLED(status);

		if (syscalled)
		{
			arg0 = ptrace(PTRACE_PEEKUSER, pid, ARG0 * sizeof(reg_t), NULL);
			if (errno == 0)
				orig_filename = text_dup(pid, arg0);
			else
				perror("ptrace: peekuser");
		}

	} while (syscalled > 0 && strcmp(filename, orig_filename));

	free(orig_filename);

	return status;
}

static reg_t	stack_poke(pid_t pid, const void *data, size_t size)
{
	reg_t	stack = ptrace(PTRACE_PEEKUSER, pid, SP * sizeof(reg_t), NULL);
	reg_t	room = stack - REDZONE_SIZE;
	reg_t	local = room - size;

	fprintf(stderr, "local var start: 0x%"PRIxREG", end: 0x%"PRIxREG"\n", local, room);

	if (text_poke(pid, local, data, size) != 0)
		local = 0;

	return local;
}

static int maps_hide(pid_t pid, const char *dst)
{
	char    buff[PATH_MAX];
	FILE    *in;
	FILE    *out;

	sprintf(buff, "/proc/%d/maps", pid);

	in = fopen(buff, "r");
	if (in == NULL)
	{
		perror(buff);
		return -1;
	}
	out = fopen(dst, "w");
	if (out == NULL)
	{
		perror(dst);
		fclose(in);
		return -1;
	}

	while (fgets(buff, sizeof(buff), in))
		if (strstr(buff, "hide.so") == NULL)
			fputs(buff, out);

	fclose(in);
	fclose(out);

	return 0;
}

static int	tracee(char** av)
{
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);

	execvp(av[0], av);

	perror("execlp");

	return -1;
}

static int	tracer(pid_t pid)
{
	static const char	fake_maps[] = "/tmp/fakemaps";
	int					exited = 0;
	int					status;
	reg_t				filename;

	fprintf(stderr, "Attaching to %d...\n", pid);

	ptrace(PTRACE_SETOPTIONS, pid, NULL, TRACEE_OPTIONS);

	while (exited == 0)
	{
		status = syscall_wait_open(pid, "/proc/self/maps");
		exited = (WIFEXITED(status) != 0) + ((WIFSIGNALED(status) != 0) << 1);
		if (!exited)
		{
			syscall_open_print(pid);
			fprintf(stderr, "Hiding maps to %s...\n", fake_maps);
			maps_hide(pid, fake_maps);
			fprintf(stderr, "Poking stack...\n");
			filename = stack_poke(pid, fake_maps, sizeof(fake_maps) - 1);
			fprintf(stderr, "Poking argument 0x%"PRIxREG" to ARG0...\n", filename);
			ptrace(PTRACE_POKEUSER, pid, ARG0 * sizeof(reg_t), filename);
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			waitpid(pid, 0, 0);
			//syscall_open_print(pid);
		}
	}

	if (exited == 1)
		fprintf(stderr, "%d exited with status %d!\n", pid, WEXITSTATUS(status));
	else if (exited == 2)
		fprintf(stderr, "%d was killed by signal %d!\n", pid, WTERMSIG(status));

	return status;
}

static int	invalid_arguments(const char *name)
{
	fprintf(stderr, "Usage: %s [VAR=VAL]... command [args]\n", name);

	return 1;
}

static int	fatal_error(const char *name)
{
	fprintf(stderr, "%s: fatal error\n", name);
	return -1;
}

static int	env_load(char **av)
{
	int		index;
	char	*val;

	for (index = 1; av[index] != NULL && (val = strchr(av[index], '=')) != NULL; index++)
	{
		*val++ = '\0';
		fprintf(stderr, "Setting %s=%s\n", av[index], val);
		setenv(av[index], val, 1);
	}
	return index;
}

int	main(int ac, char **av)
{
	pid_t	tracee_pid;
	int		command_index;

	if (ac < 2)
		return invalid_arguments(av[0]);

	command_index = env_load(av);

	if (command_index == ac)
		return invalid_arguments(av[0]);

	tracee_pid = fork();

	if (tracee_pid == -1)
		return fatal_error(av[0]);

	if (tracee_pid == 0)
		return tracee(av + command_index);

	return tracer(tracee_pid);
}
