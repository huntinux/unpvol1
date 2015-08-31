/*
 *
 *
 * tcp 并发服务器,预先创建线程,每个线程各自accept
 * 使用互斥锁,让accept被单独调用,预防"惊群"现象
 *
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
#include <fcntl.h>

#define LISTENQ 10
#define MAXLINE 1024
#define	MAXN	16384		/* max # bytes client can request */

typedef void (Sigfunc)(int);
Sigfunc*  signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}


typedef struct {
  pthread_t		thread_tid;		/* thread ID */
  long			thread_count;	/* # connections handled */
} Thread;
Thread	*tptr;		/* array of Thread structures; calloc'ed */

int				listenfd, nthreads;
socklen_t		addrlen;

void
thread_make(int i)
{
	void	*thread_main(void *);

	pthread_create(&tptr[i].thread_tid, NULL, &thread_main, (void *) i);
	return;		/* main thread returns */
}

pthread_mutex_t	mlock = PTHREAD_MUTEX_INITIALIZER;
void *
thread_main(void *arg)
{
	int				connfd;
	void			web_child(int);
	socklen_t		clilen;
	struct sockaddr	*cliaddr;

	cliaddr = malloc(addrlen);

	printf("thread %d starting\n", (int) arg);
	for ( ; ; ) {
		clilen = addrlen;
    	pthread_mutex_lock(&mlock);
		connfd = accept(listenfd, cliaddr, &clilen);
		pthread_mutex_unlock(&mlock);
		tptr[(int) arg].thread_count++;

		web_child(connfd);		/* process request */
		close(connfd);
	}
}

#define LISTENQ 10
#define MAXLINE 1024
#define	MAXN	16384		/* max # bytes client can request */

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

int main(int argc, char **argv)
{
	int		i;
	void	sig_int(int), thread_make(int);

	if (argc == 3)
		listenfd = tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 4)
		listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	else {
		fprintf(stderr, "usage: serv07 [ <host> ] <port#> <#threads>");
		exit(1);
	}
	nthreads = atoi(argv[argc-1]);
	tptr = calloc(nthreads, sizeof(Thread));

	for (i = 0; i < nthreads; i++)
		thread_make(i);			/* only main thread returns */

	signal(SIGINT, sig_int);

	for ( ; ; )
		pause();	/* everything done by threads */
}
/* end serv07 */

void
sig_int(int signo)
{
	int		i;
	void	pr_cpu_time(void);

	pr_cpu_time();

	for (i = 0; i < nthreads; i++)
		printf("thread %d, %ld connections\n", i, tptr[i].thread_count);

	exit(0);
}
