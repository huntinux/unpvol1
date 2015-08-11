#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define MAXLINE 1024
#define SERVERPORT 8888

void do_it(int cfd, struct sockaddr_in *sa, int sa_len)
{
	char buff[MAXLINE];
	int n;
	struct sockaddr_in ca;
	int ca_len = sizeof(ca);

	sprintf(buff, "hello world.");
	sendto(cfd, buff, strlen(buff) + 1, 0, (struct sockaddr *)sa, sa_len);
	n = recvfrom(cfd, buff, MAXLINE, 0, NULL, NULL);
	printf("%s\n", buff);
}


int main(int argc, char *argv[])
{

	int cfd;
	struct sockaddr_in sa;
	if( argc != 2) {
		fprintf(stderr,"Usage: %s ip\n", argv[0]);
		exit(1);
	}

	cfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port   = htons(SERVERPORT);
	inet_pton(AF_INET, argv[1], &sa.sin_addr);

	do_it(cfd, &sa, sizeof(sa));
	return 0;
}
