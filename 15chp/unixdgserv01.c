#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define MAXLINE 1024
#define UNIXDG_PATH "/tmp/unix.dg"

void dg_echo(int sfd, struct sockaddr *cliaddr, int addrlen)
{
	char buff[MAXLINE];
	int n;

	while(1) {
		n = recvfrom(sfd, buff, MAXLINE, 0, cliaddr, &addrlen);
		sendto(sfd, buff, n, 0, cliaddr, addrlen);
	}

}


int main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_un	servaddr, cliaddr;

	sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);

	unlink(UNIXDG_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXDG_PATH);

	bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	dg_echo(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
}
