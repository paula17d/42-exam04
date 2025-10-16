#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int ft_popen(const char *file, char *const argv[], char type)
{
	pid_t pid;
	int fd[2];

	if (file == NULL || argv == NULL || (type != 'r' && type != 'w'))
		return (-1);

	if (pipe(fd) == -1)
		return (-1);
	if ((pid = fork()) == -1)
	{
		close(fd[0]);
		close(fd[1]);
		return (-1);
	}

	if (pid == 0) 
	{
		if (type == 'r')
		{
			if (dup2(fd[1], STDOUT_FILENO) == -1)
			{
				close(fd[1]);
				close(fd[0]);
				return (-1);
			}
		}
		else
		{
			if (dup2(fd[0], STDIN_FILENO) == -1)
			{
				close(fd[1]);
				close(fd[0]);
				return (-1);
			}
		}
		close(fd[1]);
		close(fd[0]);
		execvp(file, argv);
		exit(1);
	}
	else 
	{
		if (type == 'r')
		{
			close(fd[1]);
			return (fd[0]);
		}
		else
		{
			close(fd[0]);
			return(fd[1]);
		}
	}
}

// include getnextline for testing
// void	ft_putstr(char *s)
// {
// 	int	i;

// 	i = 0;
// 	while (s[i] != '\0')
// 	{
// 		write (1, &s[i], 1);
// 		i++;
// 	}
// }

// void	ft_putstr_fd(char *s, int fd)
// {
// 	int	i;

// 	i = 0;
// 	while (s[i] != '\0')
// 	{
// 		write (fd, &s[i], 1);
// 		i++;
// 	}

// }

// void	ft_readstr_fd(int fd)
// {
// 	int	i;
// 	char BUFFER[1024];

// 	i = 0;
// 	while (read(fd, &BUFFER[i], 1) > 0)
// 	{
// 		i++;
// 	}

// 	printf("%s", BUFFER);
// }

// int main ()
// {
// 	int  fd;
// 	char *line;

// 	fd = ft_popen("ls", (char *const []){"ls", "-a", NULL}, 'r');
// 	ft_readstr_fd(fd);
// 	return (0);
// }

// int main ()
// {
// 	int  fd;
// 	char *line;

// 	fd = ft_popen("wc", (char *const []){"wc", "-l", NULL}, 'w');
// 	ft_putstr_fd("hii", fd);
// 	return (0);
// }
