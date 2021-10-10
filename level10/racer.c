#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#define SHARED_PROTS (PROT_READ | PROT_WRITE)
#define SHARED_FLAGS (MAP_SHARED | MAP_ANONYMOUS)

#define LISTENER_ADDR INADDR_LOOPBACK
#define LISTENER_PORT 6969

#define LISTENER_BUFFSIZE 1024U

#define TMP_TEMP "/tmp/racer.XXXXXX"

#ifdef DEBUG
# define debug(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
# define debug(fmt, ...) ((void)0)
#endif

#define error(fmt, ...) fprintf(stderr, "Error: "fmt, ##__VA_ARGS__)

typedef struct
{
	pthread_mutex_t	listening;
	pthread_mutex_t	connecting;
	pthread_mutex_t	accepting;
}	shared_mem;

static shared_mem	*shared;

int		invalid_arguments(const char *name)
{
	fprintf(stderr, "Usage: %s executable filepath\n", name);
	return 1;
}

void	terminate()
{
		pthread_mutex_destroy(&shared->listening);
		pthread_mutex_destroy(&shared->connecting);
		pthread_mutex_destroy(&shared->accepting);
}

int		init()
{
	int	status;

	shared = mmap(NULL, sizeof(*shared), SHARED_PROTS, SHARED_FLAGS, -1, 0);
	status = -(shared == NULL);

	if (status == 0)
	{
		bzero(shared, sizeof(*shared));
		status = pthread_mutex_init(&shared->listening, NULL);
		status += pthread_mutex_init(&shared->connecting, NULL);
		status += pthread_mutex_init(&shared->accepting, NULL);
		if (status == 0)
		{
			pthread_mutex_lock(&shared->listening);
			pthread_mutex_lock(&shared->connecting);
			pthread_mutex_lock(&shared->accepting);
		}
		else
		{
			perror("pthread_mutex_init");
			terminate();
			status = -1;
		}
	}
	return status;
}

int		listener_new(uint32_t ip_addr, uint16_t ip_port)
{
	struct sockaddr_in	addr = (struct sockaddr_in)
	{
		.sin_family = AF_INET,
		.sin_port = htons(ip_port),
		.sin_addr = { htonl(ip_addr) },
		.sin_zero = { 0 },
	};
	int					lfd;

	lfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (lfd != -1
	&& (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) != 0
		|| listen(lfd, 1) != 0))
	{
		perror("listener_new");
		close(lfd);
		lfd = -1;
	}
	return lfd;
}

void	*recv_file()
{
	char				buff[LISTENER_BUFFSIZE];
	ssize_t				buff_len;
	ssize_t				status;
	fd_set				readable;
	struct sockaddr_in	addr;
	socklen_t			addr_len;
	int					lfd;
	int					cfd;

	lfd = listener_new(LISTENER_ADDR, LISTENER_PORT);
	if (lfd == -1)
		return (void*)1;

	pthread_mutex_unlock(&shared->listening);
	debug("Listening...\n");

	FD_ZERO(&readable);
	FD_SET(lfd, &readable);

	status = select(lfd + 1, &readable, NULL, NULL, NULL);
	if (status == 1)
	{
		pthread_mutex_unlock(&shared->connecting);
		debug("Incoming connection!\n");

		pthread_mutex_lock(&shared->accepting);
		debug("Accepting connection...\n");

		addr_len = sizeof(addr);
		cfd = accept(lfd, (struct sockaddr*)&addr, &addr_len);
		if (cfd != -1)
		{
			while ((buff_len = recv(cfd, &buff, sizeof(buff), 0)) > 0)
				write(STDOUT_FILENO, buff, buff_len);
			status = buff_len;
			close(cfd);
		}
	}
	else
		perror("select");
	close(lfd);
	return (void*)status;
}

void	run_executable(const char *path, const char *arg)
{
	execl(path, path, arg, "127.0.0.1", NULL);
	perror("execl");
	exit(1);
}

int		wait_executable(const char *name, int pid)
{
	int	executable_status;
	int	status;

	status = -(waitpid(pid, &executable_status, 0) == -1);
	if (status == 0)
	{
		if (WIFEXITED(executable_status))
		{
			status = WEXITSTATUS(executable_status);
			if (status != 12)
				error("%s exited with abnormal status code %d!\n",
					name, status);
		}
		else if (WIFSIGNALED(executable_status))
		{
			status = -WTERMSIG(executable_status);
			error("%s was killed by signal %s!\n", name, strsignal(-status));
		}
	}
	else
		perror("waitpid");
	return status;
}

int		wait_thread(const char *name, pthread_t thread)
{
	ssize_t	status;

	if (pthread_join(thread, (void*)&status) != 0)
	{
		status = -1;
		perror("pthread_join");
	}
	else if (status != 0)
		error("%s exited with abnormal status code %zd!\n",
			name, status);

	return (int)status;
}

int		touchtemp(char *template, mode_t mode)
{
	int	fd;

	fd = mkstemp(template);
	if (fd != -1)
	{
		if (fchmod(fd, mode))
			perror("fchmod");
		close(fd);
		fd = 0;
	}
	else
		perror("mkstemp");
	return fd;
}

int		main(int ac, char **av)
{
	char		tmp_name[] = TMP_TEMP;
	int			status;
	pthread_t	listener_thread;
	int			executable_pid;

	if (ac != 3)
		status = invalid_arguments(av[0]);
	else
	{
		status = init();
		if (status == 0)
		{
			status = pthread_create(&listener_thread, NULL, &recv_file, NULL);
			if (status == 0)
			{
				// Wait for listener to be ready
				pthread_mutex_lock(&shared->listening);

				// Create readable temporary file
				if (touchtemp(tmp_name, S_IRUSR | S_IRGRP | S_IROTH) == 0)
				{
					debug("Created '%s'!\n", tmp_name);
					// Launch executable
					executable_pid = fork();
					if (executable_pid == 0)
					{
						dup2(2, 1);
						run_executable(av[1], tmp_name);
					}
					// Wait for incoming connection
					pthread_mutex_lock(&shared->connecting);

					// Replace file with symlink to target
					unlink(tmp_name);
					symlink(av[2], tmp_name);
					debug("Replaced %s with a link to %s!\n", tmp_name, av[2]);

					// Accept connection
					pthread_mutex_unlock(&shared->accepting);

					// Wait for process
					status = wait_executable(av[1], executable_pid);
					if (status != 0)
						pthread_kill(listener_thread, SIGINT);

					// Join thread
					status += wait_thread("listener", listener_thread);

					// Remove temporary file
					unlink(tmp_name);
				}
			}
			else
				perror("pthread_create");
			terminate();
		}
	}
	return status;
}

