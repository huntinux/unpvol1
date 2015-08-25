#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <malloc.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define MAXLINE 1024

#define	MAXN	16384		/* max # bytes to request from server */

int tcp_connect(const char *host, const char *serv)
{
	int				sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		fprintf(stderr, "tcp_connect error for %s, %s: %s",
				 host, serv, gai_strerror(n));
		exit(-1);
	}
	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			continue;	/* ignore this one */

		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;		/* success */

		close(sockfd);	/* ignore this one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL){	/* errno set from final connect() */
		fprintf(stderr, "tcp_connect error for %s, %s\n", host, serv);
		exit(1);
	}

	freeaddrinfo(ressave);

	return(sockfd);
}

ssize_t readn(int fd, void * buff, size_t len)
{
	char * p;
	size_t nread, nleft;

	nleft = len;
	p = (char *)buff;
	while(nleft){
		if ((nread = read(fd, p, nleft)) <0 ) {
			if( errno == EINTR) {
				nread = 0;
			}else {
				return -1;
			}
		} else if(nread == 0)
			break; // EOF

		nleft -= nread;
		p += nread;
	}
	
	return len-nleft;
}

int main(int argc, char **argv)
{
	int		i, j, fd, nchildren, nloops, nbytes;
	pid_t	pid;
	ssize_t	n;
	char	request[MAXLINE], reply[MAXN];

	if (argc != 6) {
		fprintf(stderr, "usage: client <hostname or IPaddr> <port> <#children> "
				 "<#loops/child> <#bytes/request>");
		exit(1);
	}

	nchildren = atoi(argv[3]);
	nloops = atoi(argv[4]);
	nbytes = atoi(argv[5]);
	snprintf(request, sizeof(request), "%d\n", nbytes); /* newline at end */

	for (i = 0; i < nchildren; i++) {
		if ( (pid = fork()) == 0) {		/* child */
			for (j = 0; j < nloops; j++) {
				fd = tcp_connect(argv[1], argv[2]);

				write(fd, request, strlen(request));

				if ( (n = readn(fd, reply, nbytes)) != nbytes) {
					fprintf(stderr, "server returned %d bytes", n);
					exit(1);
				}

				close(fd);		/* TIME_WAIT on client, not server */
			}
			printf("child %d done\n", i);
			exit(0);
		}
		/* parent loops around to fork() again */
	}

	while (wait(NULL) > 0)	/* now parent waits for all children */
		;
	if (errno != ECHILD) {
		fprintf(stderr, "wait error");
		exit(1);
	}

	exit(0);
}
