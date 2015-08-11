#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define MAXLINE 1024
#define SERVERPORT 8888

void srv_do_it(int sfd)
{
	char buff[MAXLINE];
	char addr[MAXLINE];
	struct sockaddr_in ca;
	int ca_len = sizeof(ca);
	int n = recvfrom(sfd, buff, MAXLINE, 0, (struct sockaddr *)&ca, &ca_len);
	printf("new client[ %s:%d ]\n", 
				inet_ntop(AF_INET, &ca.sin_addr, addr, MAXLINE),
				ntohs(ca.sin_port));
	sendto(sfd, buff, n, 0,(struct sockaddr *)&ca, ca_len);
}

int main()
{
	int sfd;
	struct sockaddr_in sa;

	sfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&sa, sizeof(sa));
	sa.sin_family		= AF_INET;
	sa.sin_port			= htons(SERVERPORT);
	sa.sin_addr.s_addr	= htonl(INADDR_ANY);

	if(bind(sfd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		perror("bind");
		exit(1);
	}

	srv_do_it(sfd);
	return 0;
}
