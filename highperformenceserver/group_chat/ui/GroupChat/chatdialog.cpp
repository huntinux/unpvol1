#include "chatdialog.h"
#include "ui_chatdialog.h"
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
extern char sendbuff[BUFFER_SIZE];
extern char recvbuff[BUFFER_SIZE];
extern size_t recvpos;
extern size_t sendpos;

ChatDialog::ChatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatDialog)
{
    ui->setupUi(this);
}

ChatDialog::~ChatDialog()
{
    delete ui;
    delete poll_thread;
}

void ChatDialog::start(int connfd)
{
    m_connfd = connfd;
    CreateThreadAndStart(this, &ChatDialog::PollThreadProc);
    show();
}

void ChatDialog::on_sendBtn_clicked()
{
    sendbuff[0] = 'H';
    sendbuff[1] = 'i';
    sendbuff[2] = '\0';
    sendpos = 3;
    ui->historyTextEdit->append(sendbuff);
}

void ChatDialog::PollThreadProc()
{
    fprintf(stderr, "Poll thread start...\n");
    struct pollfd pfd[1];
    pfd[0].fd = m_connfd;
    pfd[0].events = POLLOUT | POLLIN | POLLRDHUP | POLLERR;
    while(1)
    {
        int n = poll(pfd, 1, -1);
        if(pfd[0].revents & POLLERR)
        {
            fprintf(stderr, "get an error from %d\n", pfd[0].fd );
            char errors[100];
            memset(errors, '\0', 100);
            socklen_t length = sizeof(errors);
            if(getsockopt(pfd[0].fd, SOL_SOCKET, SO_ERROR, &errors, &length ) < 0)
            {
                printf("get socket option failed\n");
            }
            continue;
        }
        else if(pfd[0].revents & POLLRDHUP)
        {
            fprintf(stderr, "POLLRDHUP\n");
            /* server close the connection */
            ::closesocket(pfd[0].fd);
            fprintf(stderr, "Server close the connection\n");
            exit(-1);
        }
        else if( pfd[0].revents & POLLIN )
        {
            fprintf(stderr, "POLLIN\n");
            /* server send data to client */
            int connfd = pfd[0].fd;
            memset(recvbuff, '\0', BUFFER_SIZE);
            int ret = recv(connfd, recvbuff, BUFFER_SIZE-1, 0 );
            if( ret < 0 )
            {
                ::closesocket( connfd );
                fprintf(stderr, "recv failed.\n");
            }
            else if( ret == 0 )
            {
                fprintf(stderr, "code should not come to here\n" );
            }
            recvpos = ret;
            fprintf(stderr, "%s", recvbuff);
        }
        else if(pfd[0].revents & POLLOUT)
        {
            fprintf(stderr, "POLLOUT\n");
            if(sendpos == 0)
            {
                continue;
            }
            int connfd = pfd[0].fd;
            int ret = send( connfd, sendbuff, strlen(sendbuff), 0 );
            sendpos -= ret;
        }

    }

    ::closesocket(m_connfd);

}
