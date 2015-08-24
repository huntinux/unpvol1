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

void	*copyto(void *);

static int	sockfd;		/* global for both threads to access */
static FILE	*fp;

static int	read_cnt;
static char	*read_ptr;
static char	read_buf[MAXLINE];


void str_cli(FILE *fp_arg, int sockfd_arg);
int tcp_connect(const char *host, const char *serv);
ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t writen(int fd, const void *buff, size_t n);

int main(int argc, char **argv)
{
	int		sockfd;

	if (argc != 3) {
		fprintf(stderr, "usage: tcpcli <hostname> <service>");
		exit(1);
	}

	sockfd = tcp_connect(argv[1], argv[2]);

	str_cli(stdin, sockfd);		/* do it all */

	exit(0);
}


void str_cli(FILE *fp_arg, int sockfd_arg)
{
	char		recvline[MAXLINE];
	pthread_t	tid;

	sockfd = sockfd_arg;	/* copy arguments to externals */
	fp = fp_arg;

	pthread_create(&tid, NULL, copyto, NULL);

	while (readline(sockfd, recvline, MAXLINE) > 0)
		fputs(recvline, stdout);
}

static ssize_t my_read(int fd, char *ptr)
{

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

ssize_t readlinebuf(void **vptrptr)
{
	if (read_cnt)
		*vptrptr = read_ptr;
	return(read_cnt);
}
/* end readline */



void * copyto(void *arg)
{
	char	sendline[MAXLINE];

	while (fgets(sendline, MAXLINE, fp) != NULL)
		writen(sockfd, sendline, strlen(sendline));

	shutdown(sockfd, SHUT_WR);	/* EOF on stdin, send FIN */

	return(NULL);
		/* 4return (i.e., thread terminates) when EOF on stdin */

}




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
		fprintf(stderr, "tcp_connect error for %s, %s", host, serv);
		exit(1);
	}

	freeaddrinfo(ressave);

	return(sockfd);
}
/* end tcp_connect */

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_connect() function.
 */

int Tcp_connect(const char *host, const char *serv)
{
	return(tcp_connect(host, serv));
}



ssize_t writen(int fd, const void *buff, size_t n)
{
	size_t nleft, nwrite;
	const char *p;

	p = (const char *)buff;
	nleft = n;

	while(nleft){

		if ( (nwrite = write(fd, p, nleft)) <= 0){
			if(nwrite < 0 && errno == EINTR){
				nwrite = 0;
			}else{
				return -1;
			}
		}

		nleft  -=  nwrite;
		p  += nwrite;
	}

	return n;
}

