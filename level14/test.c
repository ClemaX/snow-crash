#include <unistd.h>
#include <fcntl.h>
#include <syscall.h>

int	main(int ac, const char **av)
{
	char	buff[1024];
	ssize_t	len;
	int		fd;

	if (ac < 2)
		return 1;

	fd = syscall(SYS_open, av[1], O_RDONLY);

	while ((len = read(fd, buff, sizeof(buff))) > 0)
		write(STDOUT_FILENO, buff, len);

	return close(fd);
}
