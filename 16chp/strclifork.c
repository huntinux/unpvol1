#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	pid_t	pid;
	char	sendline[MAXLINE], recvline[MAXLINE];

	if ( (pid = Fork()) == 0) {		/* child: server -> stdout */
		while (Readline(sockfd, recvline, MAXLINE) > 0)
			Fputs(recvline, stdout);

		kill(getppid(), SIGTERM);	/* in case parent still running */
		exit(0);
	}

		/* parent: stdin -> server */
	while (Fgets(sendline, MAXLINE, fp) != NULL)
		Writen(sockfd, sendline, strlen(sendline));

	Shutdown(sockfd, SHUT_WR);	/* EOF on stdin, send FIN */
	pause();
	return;
}

/*
	使用shutdown的原因：
	fork之后，父进程和子进程共享sockfd，sockfd的引用计数为2.
	如果此处使用close，仅仅是将sockfd的引用计数减1，那么sockfd的引用计数变为1，仍然不为0。
	不会发送FIN分节。
	而shutdown一定会发送FIN分节。

	使用kill的原因：
    有可能服务器提前终止了，子进程将在套接字上读到EOF，这样子进程必须告诉父进程：不要再往套接字写入数据了。
	所以通过kill，杀死父进程。

*/
