
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
	int sfd, infd;
	struct sockaddr_in sa, ca;
	int ca_len;
	int ret;

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


	ca_len = sizeof(ca); // 必须要初始化! 看man 2 accept
	infd = accept(sfd, (struct sockaddr *)&ca, &ca_len);
	if(infd == -1) {
		perror("accept");
		exit(-1);
	}

	int n,s;
	char buff[1024];
	n = read(infd, buff, 1024);
	printf("receive %d bytes: %s\n", n, buff);


	s = write(infd, buff, n);
	printf("write %d bytes: %s\n", s, buff);


	return 0;
}
