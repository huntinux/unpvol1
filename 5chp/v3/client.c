
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int sfd;
	struct sockaddr_in sa;
	int ret;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1) {
		perror("socket");
		exit(-1);
	}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(8888);
	inet_pton(AF_INET, "192.168.151.111", &sa.sin_addr);

	ret = connect(sfd, (struct sockaddr *)&sa, sizeof(sa));
	if(ret == -1) {
		perror("connect");
		exit(-1);
	}

	int n;
	char buff[1024] = "hello world";
	n = write(sfd, buff, strlen(buff) + 1);
	printf("write %d bytes: %s\n", n, buff);

	n = read(sfd, buff, 1024);
	printf("read %d bytes: %s\n", n, buff);


	return 0;
}
