/**
 * epoll Level-Trigger example
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/epoll.h>
#include <netdb.h>

void printf_address(struct sockaddr *in_addr, socklen_t in_len, const char *msg)
{

    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	int s = getnameinfo (in_addr, in_len,
			hbuf, sizeof hbuf,
			sbuf, sizeof sbuf,
			NI_NUMERICHOST | NI_NUMERICSERV);
	if (s == 0)
	{
		printf("%s (host=%s, port=%s)\n", msg, hbuf, sbuf);
	}
}


static int create_and_bind (char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, listenfd;

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */ 
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */ 
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */ 

	s = getaddrinfo (NULL, port, &hints, &result);
	if (s != 0)
	{
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		listenfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (listenfd == -1)
			continue;

		s = bind (listenfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)
		{
			/* We managed to bind successfully! */
			break;
		}

		close (listenfd);
	}

	if (rp == NULL)
	{
		fprintf (stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo (result);

	return listenfd;
}

#define EPOLLMAXEVENTS 64

int main(int argc, char *argv[])
{
	if(argc <= 1)
	{
		printf("Usage: %s port\n", basename(argv[0]));
		return -1;
	}

	int listenfd = create_and_bind(argv[1]);
	if(listenfd < 0) {printf("create_and_bind failed.\n"); return -2;}

	listen(listenfd, 5);

	int epfd = epoll_create1(0);
	if(epfd == -1)
	{
		perror("epoll_create1\n");
		return -1;
	}

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = listenfd;

	if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event) == -1)
	{
		perror("epoll_ctl");
		return -1;
	}

	while(1)
	{
		struct epoll_event events[EPOLLMAXEVENTS];
		int n = epoll_wait(epfd, events, EPOLLMAXEVENTS, -1);

		int i;
		for(i = 0; i < n; i++)
		{
			if((events[i].events & EPOLLERR)  ||
				(events[i].events & EPOLLHUP) ||
				(!(events[i].events & EPOLLIN)))
			{
				fprintf(stderr, "epoll error, close fd %d\n", events[i].data.fd);
				close(events[i].data.fd);
				epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				continue;
			}
			else if(events[i].data.fd == listenfd)
			{
				/* client connection */

				struct sockaddr_in clientaddr;
				socklen_t clientaddrlen = sizeof(clientaddr);
				int connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
				if(connfd == -1)
				{
					perror("accept");
					continue;
				}
				printf_address((struct sockaddr *)&clientaddr, clientaddrlen, "Accept:");

				event.data.fd = connfd;
				event.events = EPOLLIN;
				if(epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event) == -1)
				{
					perror("epoll_ctl");
					continue;
				}

			}
			else
			{
				/* client send data to me (server) */

				/* 只要该socket内核接收缓冲区中还有数据，此就会再次进入此代码块 */
				char buf[1024];
				ssize_t count;

				count = read(events[i].data.fd, buf, sizeof(buf));
				if(count == -1)
				{
					perror("read");
					continue;
				}
				else if(count == 0)
				{
					printf("client close the connection. close fd %d\n", events[i].data.fd);
					close(events[i].data.fd);
					epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					continue;
				}

				printf("Get %ld bytes data: %s\n", count, buf);

			}

		}

	}

	close(listenfd);
	return 0;
}
