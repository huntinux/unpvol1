/**
 * Group Chat Server
 * Use 'poll'
 * author: hongjin.cao
 */

#define _GNU_SOURCE /* for POLLRDHUP */

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

#define BUFFER_SIZE 1024 
#define FD_LIMIT    65535 /* max number file descriptor */

struct client_info
{
    struct sockaddr_in address; /* connection socket file descriptor */
    const char *wb;             /* point to the message prepared send to this client */
    char rb[BUFFER_SIZE];       /* receive buff */
};

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

        int enable = 1;
		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&enable, sizeof(int)) < 0)
		{
			perror("setsockopt SO_REUSEADDR");
		}

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


int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Usage: %s [port] [max_client_num]\n", basename(argv[0]));
        return -1;
    }

    int listenfd = create_and_bind(argv[1]);
    if(listenfd < 0) return -2;

    if(listen(listenfd, 5) == -1)
    {
        perror("listen");
        return -3;
    }

    struct client_info *clients = (struct client_info *)calloc(FD_LIMIT, sizeof(struct client_info));
    if(clients == NULL)
    {
        fprintf(stderr, "calloc failed.\n");
        return -4;
    }

    /* prepare client connection socket descriptors,
       plus one for listen socket descriptor */
    int max_client_num = atoi(argv[2]);
    if(max_client_num > FD_LIMIT - 1 - 3) /* one for listen socket fd, three for stdin stdout stderr */
    {
        fprintf(stderr, "max_client_num should not larger than %d.\n", FD_LIMIT - 1 - 3);
        return -5;
    }
    int current_client_num = 0;
    struct pollfd *pfd = (struct pollfd *)calloc(max_client_num + 1, sizeof(struct pollfd));
    if(pfd == NULL)
    {
        fprintf(stderr, "calloc failed.\n");
        return -4;
    }
    pfd[0].fd = listenfd;
    pfd[0].events = POLLIN | POLLERR;

    while(1)
    {
        int n = poll(pfd, current_client_num + 1, -1);
        for(int i = 0; i < current_client_num + 1; i++)
        {
            if((pfd[i].fd == listenfd) && (pfd[i].revents & POLLIN))
            {
                /* a new connection */
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                if ( connfd < 0 )
                {
                    perror("accept");
                    continue;
                }

                if(current_client_num >= max_client_num)
                {
                    fprintf(stderr, "too many users.\n");
                    close(connfd);
                    continue;
                }

                /* record client infomation, and add connection socket descriptor to pollfd */
                clients[connfd].address = client_address;
                current_client_num++;
                pfd[current_client_num].fd = connfd;
                pfd[current_client_num].events = POLLIN | POLLRDHUP | POLLERR;
                pfd[current_client_num].revents = 0;
                printf_address((struct sockaddr *)&client_address, client_addrlength, "Accept: ");
            }
            else if(pfd[i].revents & POLLERR)
            {
                fprintf(stderr, "get an error from %d\n", pfd[i].fd );
                char errors[100];
                memset(errors, '\0', 100);
                socklen_t length = sizeof(errors);
                if(getsockopt(pfd[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length ) < 0)
                {
                    printf("get socket option failed\n");
                }
                continue;
            }
            else if(pfd[i].revents & POLLRDHUP)
            {
                //for(int j = i + 1; j < current_client_num + 1; j++)
                //{
                //    pfd[j - 1] = pfd[j];
                //}

                /* client close the connection */
                clients[pfd[i].fd] = clients[pfd[current_client_num].fd];
                close(pfd[i].fd);
                pfd[i] = pfd[current_client_num]; /* move the last to i, this method will not move element in a loop */
                current_client_num--;
                i--;
                printf( "a client left\n" );
            }
            else if( pfd[i].revents & POLLIN )
            {
                int connfd = pfd[i].fd;
                memset(clients[connfd].rb, '\0', BUFFER_SIZE);
                int ret = recv(connfd, clients[connfd].rb, BUFFER_SIZE-1, 0 );
                printf( "get %d bytes of client data %s from %d\n", ret, clients[connfd].rb, connfd );
                if( ret < 0 )
                {
                    if( errno != EAGAIN )
                    {
                        close( connfd );
                        clients[pfd[i].fd] = clients[pfd[current_client_num].fd];
                        pfd[i] = pfd[current_client_num];
                        i--;
                        current_client_num--;
                    }
                }
                else if( ret == 0 )
                {
                    printf( "code should not come to here\n" );
                }
                else
                {
                    /* prepare send message to other clients */
                    for( int j = 1; j <= current_client_num; ++j )
                    {
                        if( pfd[j].fd == connfd )
                        {
                            continue;
                        }
                        
                        pfd[j].events |= ~POLLIN;
                        pfd[j].events |= POLLOUT;
                        clients[pfd[j].fd].wb= clients[connfd].rb;
                    }
                }
            }
            else if(pfd[i].revents & POLLOUT)
            {
                int connfd = pfd[i].fd;
                if(!clients[connfd].wb)
                {
                    continue;
                }
                int ret = send( connfd, clients[connfd].wb, strlen( clients[connfd].wb), 0 );
                clients[connfd].wb = NULL;
                pfd[i].events |= ~POLLOUT;
                pfd[i].events |= POLLIN;
            }
        }

    }

    free(pfd);
    free(clients);
    close(listenfd);
    return 0;
}
