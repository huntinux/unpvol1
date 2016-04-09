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
#include <netdb.h>
#include <poll.h>



static int create_and_bind (char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */ // 返回ipV4 或 ipv6
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */ // 告诉该函数，我们需要TCP服务信息
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */ // 套接字用于被动打开 (服务器通常是被动的)

	s = getaddrinfo (NULL, port, &hints, &result);
	if (s != 0)
	{
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)
		{
			/* We managed to bind successfully! */
			break;
		}

		close (sfd);
	}

	if (rp == NULL)
	{
		fprintf (stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo (result);

	return sfd;
}

#define FDNUM 2

int main(int argc, char *argv[])
{
	if(argc <= 1)
	{
		printf("Usage: %s port\n", basename(argv[0]));
		return -1;
	}

	int ret = 0;
	int sfd = create_and_bind(argv[1]);
	if(sfd < 0) {printf("create_and_bind failed.\n"); return -2;}

	listen(sfd, 5);

	struct sockaddr_in client_address;
	socklen_t client_addrlength = sizeof( client_address );
	int connfd = accept( sfd, ( struct sockaddr* )&client_address, &client_addrlength );
	if ( connfd < 0 )
	{
		printf( "errno is: %d\n", errno );
		close( sfd);
		return -3;
	}

	char buff[1024];
	struct pollfd pfd[FDNUM];
	pfd[0].fd = connfd;
	pfd[0].events = POLLIN | POLLPRI;

	while(1)
	{
		memset( buff, '\0', sizeof( buff ) );
		int n = poll(pfd, 1, -1);
		printf("poll get %d fd ready.\n");

		if(pfd[0].revents & POLLIN)
		{
			int nr = read(connfd, buff, sizeof(buff));
			if(nr > 0)
			{
				printf("get %d bytes data: %s\n", nr, buff);
			}
			else if(nr == 0)
			{
				// client close the connection
				printf("client close the connection.\n");
				break;
			}
			else
			{
				// error occures
				perror("read");
				ret = -4;
				break;
			}
		}

		if(pfd[0].revents & POLLPRI)
		{
			int nr = recv(connfd, buff, sizeof(buff) - 1, MSG_OOB);
			if(nr < 0)
			{
				perror("recv");
				ret = -5;
				break;
			}
			printf("recv %d bytes OOB data: %s\n", nr, buff);
		}

	}


	close(sfd);

	return ret;
}
