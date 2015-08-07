
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <wait.h>


void do_it(int infd)
{
	int n,s;
	char buff[1024];
	printf("child:%6d\n", getpid());

	n = read(infd, buff, 1024);
	printf("receive %d bytes: %s\n", n, buff);
	s = write(infd, buff, n);
	printf("write %d bytes: %s\n", s, buff);
}

void sigchld_handler(int signum)
{
	int stat;
	pid_t pid;

	pid = wait(&stat);
	printf("child %d terminated.\n", pid);

}

int main(int argc, char* argv[])
{
	int sfd, infd;
	struct sockaddr_in sa, ca;
	int ca_len;
	int ret;
	pid_t pid;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1) {
		perror("socket");
		exit(-1);
	}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(8888);
	sa.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sfd, (struct sockaddr *)&sa, sizeof(sa));
	if(ret == -1) {
		perror("bind");
		exit(-1);
	}

	ret = listen(sfd, 5);
	if(ret == -1) {
		perror("listen");
		exit(-1);
	}

	signal(SIGCHLD, sigchld_handler);
	while(1) {
		memset(&ca, 0, sizeof(ca));
		ca_len = sizeof(ca); // 必须要初始化! 看man 2 accept
		infd = accept(sfd, (struct sockaddr *)&ca, &ca_len);
		if(infd == -1) {
			perror("accept");
			exit(-1);
		}

		pid = fork();
		if(pid == -1){
			perror("fork");
			exit(-1);
		}else if(pid == 0) { // child
			close(sfd);  // 子进程关闭监听套接字
			do_it(infd);
		 	exit(0); // 没有这个exit,会出错  accept: Bad file descriptor

		}else{ // parent
			close(infd); // 父进程关闭已连接套接字
		}

	}

	return 0;
}
