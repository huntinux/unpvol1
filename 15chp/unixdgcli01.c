#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define MAXLINE 1024

#define UNIXDG_PATH "/tmp/unix.dg"

void dg_cli(FILE *fp, int sfd, struct sockaddr * servaddr, int addrlen)
{
	char buff[MAXLINE];
	int n;

	while(fgets(buff, MAXLINE, fp)) {
		n = strlen(buff);
		sendto(sfd, buff, n, 0, servaddr, addrlen);
		n = recvfrom(sfd, buff, MAXLINE, 0, NULL, NULL);
		printf("%s\n", buff);
	}

}


int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_un	cliaddr, servaddr;

	sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);

	bzero(&cliaddr, sizeof(cliaddr));		/* bind an address for us */
	cliaddr.sun_family = AF_LOCAL;
	strcpy(cliaddr.sun_path, tmpnam(NULL));
	//strcpy(cliaddr.sun_path, mkstmp(NULL));

	bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr));

	bzero(&servaddr, sizeof(servaddr));	/* fill in server's address */
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXDG_PATH);

	dg_cli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	exit(0);
}
