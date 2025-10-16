#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
 #include <string.h>

void alarm_handler(int sig)
{
	(void)sig;
}

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
	struct sigaction sa;
	pid_t pid;
	int status;
	int exit_code;
	
	sa.sa_handler = alarm_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	pid = fork();
	if (pid == -1)
		return -1;
	if (pid == 0)
	{
		f();
		exit(0);
	}
	alarm(timeout);
	if (waitpid(pid, &status, 0) == -1)
	{
		if (errno == EINTR)
		{
			kill(pid, 9);
			waitpid(pid, NULL, 0);
			if (verbose == true)
				printf("Bad function: timed out after %d seconds\n", timeout);
			return 0;
		}
		return -1;
	}
	alarm(0);
	if (WIFEXITED(status) != 0)
	{
		exit_code = WEXITSTATUS(status);
		if (exit_code != 0)
		{
			if (verbose == true)
				printf("Bad function: exited with code %d\n", exit_code);
			return 0;
		}
		else if(exit_code == 0)
		{
			if (verbose == true)
				printf("Nice function!\n");
			return 1;
		}
	}
	if (WIFSIGNALED(status) != 0)
	{
		if (verbose == true)
			printf("Bad function: %s\n", strsignal(WTERMSIG(status)));
		return 0;
	}
	return -1;
}


// for testing only
// int main ()
// {
// 	sandbox(f, 4, true);

// 	return 0;
// }