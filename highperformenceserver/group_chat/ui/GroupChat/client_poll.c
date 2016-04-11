#if 0
/**
 * Group Chat Client 
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

static int create_and_connect(const char *ip, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, connfd;

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
        connfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (connfd == -1)
            continue;

        s = connect(connfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0)
        {
            /* We managed to bind successfully! */
            break;
        }

        close (connfd);
    }

    if (rp == NULL)
    {
        fprintf (stderr, "Could not connect\n");
        return -1;
    }

    freeaddrinfo (result);

    return connfd;
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Usage: %s [server_ip] [server_port]\n", basename(argv[0]));
        return -1;
    }

    int connfd = create_and_connect(argv[1], argv[2]);
    if(connfd < 0) return -2;

    /* prepare client connection socket descriptors,
       plus one for listen socket descriptor */
    struct pollfd pfd[2];
    pfd[0].fd = STDIN_FILENO;
    pfd[0].events = POLLIN;
    pfd[1].fd = connfd;
    pfd[1].events = POLLIN | POLLRDHUP | POLLERR;
    
    char sendbuff[BUFFER_SIZE];
    char recvbuff[BUFFER_SIZE];
    size_t recvpos = 0;
    size_t sendpos = 0;
    while(1)
    {
        int n = poll(pfd, 2, -1);
        for(int i = 0; i < 2; i++)
        {
            if((pfd[i].fd == STDIN_FILENO) && (pfd[i].revents & POLLIN))
            {
                /* user input */
                memset(sendbuff, '\0', BUFFER_SIZE);
                sendpos = read(STDIN_FILENO, sendbuff, BUFFER_SIZE - 1);
                pfd[1].events &= ~POLLIN;
                pfd[1].events |= POLLOUT;
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
                /* server close the connection */
                close(pfd[i].fd);
                printf("Server close the connection\n");
                exit(-1);
            }
            else if( pfd[i].revents & POLLIN )
            {
                /* server send data to client */
                int connfd = pfd[i].fd;
                memset(recvbuff, '\0', BUFFER_SIZE);
                int ret = recv(connfd, recvbuff, BUFFER_SIZE-1, 0 );
                if( ret < 0 )
                {
                    close( connfd );
                    printf("recv failed.\n");
                }
                else if( ret == 0 )
                {
                    printf( "code should not come to here\n" );
                }
                recvpos = ret;
                fprintf(stdout, "%s", recvbuff);
            }
            else if(pfd[i].revents & POLLOUT)
            {
                if(sendpos == 0)
                {
                    continue;
                }
                int connfd = pfd[i].fd;
                int ret = send( connfd, sendbuff, strlen(sendbuff), 0 );
                sendpos -= ret;
                pfd[i].events &= ~POLLOUT;
                pfd[i].events |= POLLIN;
            }
        }

    }

    close(connfd);
    return 0;
}
#endif
