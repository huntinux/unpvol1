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


#define LISTENQ 10
#define MAXLINE 1024

static void	*doit(void *);		/* each thread executes this function */
int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp);
void str_echo(int sockfd);


int main(int argc, char **argv)
{
	int				listenfd, connfd;
	int *pconnfd;
	pthread_t		tid;
	socklen_t		addrlen, len;
	struct sockaddr	*cliaddr;

	if (argc == 2)
		listenfd = tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	else {
		fprintf(stderr, "usage: tcpserv01 [ <host> ] <service or port>");
		exit(1);
	}

	cliaddr = malloc(addrlen);

	for ( ; ; ) {
		len = addrlen;
		
		pconnfd = malloc(sizeof(int));
		*pconnfd = accept(listenfd, cliaddr, &len);

		pthread_create(&tid, NULL, &doit, (void *) pconnfd);
	}
}

static void * doit(void *arg)
{
	int fd;

	fd = *((int *) arg);
	free(arg);
	pthread_detach(pthread_self());
	str_echo(fd);	/* same function as before */
	close(fd);		/* done with connected socket */
	return(NULL);
}

ssize_t writen(int fd, const void *buff, size_t n)
{
	size_t nleft;
	size_t nwrite;
	const char *p;

	p = (const char *)buff;
	nleft = n;

	while(nleft){
		if((nwrite = write(fd, p, nleft)) <= 0) {
			if(nwrite < 0 && errno == EINTR){
				nwrite = 0;
			}else{
				return -1;
			}
		}

		p += nwrite;
		nleft -= nwrite;
	}

	return n;
}



void str_echo(int sockfd)
{
	ssize_t		n;
	char		buf[MAXLINE];

again:
	while ( (n = read(sockfd, buf, MAXLINE)) > 0)
		writen(sockfd, buf, n);

	if (n < 0 && errno == EINTR)
		goto again;
	else if (n < 0) {
		fprintf(stderr, "str_echo: read error");
		exit(1);
	}
}


int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	int				listenfd, n;
	const int		on = 1;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		fprintf(stderr, "tcp_listen error for %s, %s: %s",
				host, serv, gai_strerror(n));
		exit(1);
	}
	ressave = res;

	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
			continue;		/* error, try next one */

		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;			/* success */

		close(listenfd);	/* bind error, close and try next one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL){	/* errno from final socket() or bind() */
		fprintf(stderr, "tcp_listen error for %s, %s", host, serv);
		exit(1);
	}

	listen(listenfd, LISTENQ);

	if (addrlenp)
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */

	freeaddrinfo(ressave);

	return(listenfd);
}
