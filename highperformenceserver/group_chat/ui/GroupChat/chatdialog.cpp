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

ChatDialog::ChatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatDialog),
    recvpos(0),
    sendpos(0)
{
    ui->setupUi(this);
    ui->historyTextEdit->setReadOnly(true);
}

ChatDialog::~ChatDialog()
{
    delete ui;
    delete poll_thread;
}

void ChatDialog::start(int connfd)
{
    m_connfd = connfd;
    poll_thread = CreateThreadAndStart(this, &ChatDialog::PollThreadProc);
    show();
}

void ChatDialog::on_sendBtn_clicked()
{
    sendpos = ui->sendTextEdit->toPlainText().length();
    strncpy(sendbuff, ui->sendTextEdit->toPlainText().toStdString().c_str(), BUFFER_SIZE);
    ui->historyTextEdit->append(sendbuff);
    ui->sendTextEdit->clear();
}

void ChatDialog::PollThreadProc()
{
    struct pollfd pfd;
    pfd.fd 		= m_connfd;
    pfd.events 	= POLLOUT | POLLIN | POLLRDHUP;
    pfd.revents	= 0;
    while(1)
    {
        int n = poll(&pfd, 1, -1);
        if( pfd.revents & POLLIN )
        {
            /* server send data to client */
            int connfd = pfd.fd;
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
            recvpos += ret;
            fprintf(stderr, "%s", recvbuff);
            ui->historyTextEdit->append(recvbuff);
        }
        else if(pfd.revents & POLLOUT)
        {
            if(sendpos == 0)
            {
                continue;
            }
            int connfd = pfd.fd;
            int ret = send( connfd, sendbuff, sendpos, 0 );
            sendpos -= ret;
        }
        else if(pfd.revents & POLLRDHUP)
        {
            /* server close the connection */
            fprintf(stderr, "Server close the connection\n");
            ::closesocket(pfd.fd);
            qApp->exit(-1);
        }
        else if(pfd.revents & POLLERR)
        {
            fprintf(stderr, "get an error from %d\n", pfd.fd );
            char errors[100];
            memset(errors, '\0', 100);
            socklen_t length = sizeof(errors);
            if(getsockopt(pfd.fd, SOL_SOCKET, SO_ERROR, &errors, &length ) < 0)
            {
                printf("get socket option failed\n");
            }
            continue;
        }
        else
        {
            fprintf(stderr, "unknown event");
        }

    }

    ::closesocket(m_connfd);
}
