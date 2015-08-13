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
	ʹ��shutdown��ԭ��
	fork֮�󣬸����̺��ӽ��̹���sockfd��sockfd�����ü���Ϊ2.
	����˴�ʹ��close�������ǽ�sockfd�����ü�����1����ôsockfd�����ü�����Ϊ1����Ȼ��Ϊ0��
	���ᷢ��FIN�ֽڡ�
	��shutdownһ���ᷢ��FIN�ֽڡ�

	ʹ��kill��ԭ��
    �п��ܷ�������ǰ��ֹ�ˣ��ӽ��̽����׽����϶���EOF�������ӽ��̱�����߸����̣���Ҫ�����׽���д�������ˡ�
	����ͨ��kill��ɱ�������̡�

*/
