#include <unistd.h>

int	main(void)
{
	unsigned char	buff[512];
	ssize_t			len = read(STDIN_FILENO, buff, sizeof(buff) - 1);
	int				status = (len == -1);

	if (status == 0 && len != 0)
	{
		buff[--len] = '\0';
		for (ssize_t i = 0; i < len; i++)
			buff[i] -= i;
		buff[len++] = '\n';
		buff[len] = '\0';
		status = (write(STDOUT_FILENO, buff, len) == -1) * 2;
	}
	return status;
}
