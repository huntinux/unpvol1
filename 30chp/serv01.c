
/*
 *
 *  并发服务器， 每个连接创建一个子进程 fork()
 *
 */

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
#include <signal.h>
#include <sys/resource.h>

#define LISTENQ 10
#define MAXLINE 1024
#define	MAXN	16384		/* max # bytes client can request */

void pr_cpu_time(void)
{
	double			user, sys;
	struct rusage	myusage, childusage;

	if (getrusage(RUSAGE_SELF, &myusage) < 0) {
		fprintf(stderr, "getrusage error");
		exit(1);
	}
	if (getrusage(RUSAGE_CHILDREN, &childusage) < 0) {
		fprintf(stderr, "getrusage error");
		exit(1);
	}

	user = (double) myusage.ru_utime.tv_sec +
					myusage.ru_utime.tv_usec/1000000.0;
	user += (double) childusage.ru_utime.tv_sec +
					 childusage.ru_utime.tv_usec/1000000.0;
	sys = (double) myusage.ru_stime.tv_sec +
				   myusage.ru_stime.tv_usec/1000000.0;
	sys += (double) childusage.ru_stime.tv_sec +
					childusage.ru_stime.tv_usec/1000000.0;

	printf("\nuser time = %g, sys time = %g\n", user, sys);
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

void web_child(int sockfd)
{
	int			ntowrite;
	ssize_t		nread;
	char		line[MAXLINE], result[MAXN];

	for ( ; ; ) {
		if ( (nread = read(sockfd, line, MAXLINE)) == 0)
			return;		/* connection closed by other end */

			/* 4line from client specifies #bytes to write back */
		ntowrite = atol(line);
		if ((ntowrite <= 0) || (ntowrite > MAXN)) {
			fprintf(stderr, "client request for %d bytes", ntowrite);
			exit(1);
		}

		writen(sockfd, result, ntowrite);
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

int main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	void				sig_chld(int), sig_int(int), web_child(int);
	socklen_t			clilen, addrlen;
	struct sockaddr		*cliaddr;

	if (argc == 2)
		listenfd = tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	else {
		fprintf(stderr, "usage: serv01 [ <host> ] <port#>");
		exit(1);
	}
	cliaddr = malloc(addrlen);

	//signal(SIGCHLD, sig_chld);
	signal(SIGINT, sig_int);

	for ( ; ; ) {
		clilen = addrlen;
		if ( (connfd = accept(listenfd, cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else {
				fprintf(stderr, "accept error");
				exit(1);
			}
		}

		if ( (childpid = fork()) == 0) {	/* child process */
			close(listenfd);	/* close listening socket */
			web_child(connfd);	/* process request */
			exit(0);
		}
		close(connfd);			/* parent closes connected socket */
	}
}
/* end serv01 */

void sig_int(int signo)
{
	void	pr_cpu_time(void);

	pr_cpu_time();
	exit(0);
}

