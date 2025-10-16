#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
	cmd1 | cmd2 | cmd3
*/
int picoshell(char **cmds[])
{
	int i;
	pid_t pid;
	int fd[2];
	int prev_fd = -1;

	i = 0;
	while (cmds[i] != NULL)
	{
		if (cmds[i + 1] != NULL)
		{
			if (pipe(fd) == -1)
				return 1;
		}
		pid = fork();
		if (pid == -1)
		{
			if (prev_fd != -1)
				close(prev_fd);
			close(fd[0]);
			close(fd[1]);
			return 1;
		}
		if (pid == 0)
		{	
			if (i > 0)
			{
				if (dup2(prev_fd, 0) == -1)
				{
					close(prev_fd);
					if (cmds[i + 1] != NULL)
					{
						close(fd[1]);
						close(fd[0]);
					}
					exit(1);
				}
				close(prev_fd);
			}
			if (cmds[i + 1] != NULL)
			{
				if (dup2(fd[1], 1) == -1)
				{
					if(prev_fd != -1)
						close(prev_fd);
					close(fd[0]);
					close(fd[1]);
					exit(1);
				}
				close(fd[1]);
				close(fd[0]);
			}
			execvp(cmds[i][0], cmds[i]);
			exit(1);
		}
		else 
		{
			if (prev_fd != -1)
			{
				close(prev_fd);
			}
			if (cmds[i + 1] != NULL)
			{
				prev_fd = fd[0];
				close(fd[1]);
			}
		}
		i++;
	}
	while (wait(NULL) != -1)
	{

	}
	return 0;
}

// int main(int argc, char *argv[])
// {
//     int cmds_size = 1;
//     for (int i = 1; i < argc; i++)
//     {
//         if (!strcmp(argv[i], "|"))
//             cmds_size++;
//     }

//     char ***cmds = calloc(cmds_size + 1, sizeof(char **));
//     if (!cmds)
//     {
//         dprintf(2, "Malloc error : %m\n");
//         return 1;
//     }

//     cmds[0] = argv + 1;
//     int cmds_i = 1;
//     for (int i = 1; i < argc; i++)
//     {
//         if (!strcmp(argv[i], "|"))
//         {
//             cmds[cmds_i] = argv + i + 1;
//             argv[i] = NULL;
//             cmds_i++;
//         }
//     }

//     int ret = picoshell(cmds);
//     if (ret)
//         perror("picoshell");

//     free(cmds);
//     return ret;
// }