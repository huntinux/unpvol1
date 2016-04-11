#include "connectdialog.h"
#include "ui_connectdialog.h"
#include <QDebug>

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
#include "def.h"

ConnectDialog::ConnectDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    ui->serverIpLineEdit->setText("127.0.0.1");
    ui->serverPortLineEdit->setText("9090");
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}
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

bool ConnectDialog::ConnectToServer(const char *ip, const char *port)
{
    return (m_connfd = create_and_connect(ip, port) > 0);
}


void ConnectDialog::on_connectBtn_clicked()
{
    qDebug()<<ui->serverIpLineEdit->text();
    qDebug()<<ui->serverPortLineEdit->text();

    if(ConnectToServer(ui->serverIpLineEdit->text().toStdString().c_str(),
                       ui->serverPortLineEdit->text().toStdString().c_str()))
    {
        hide();
        emit ConnectSuccess(m_connfd);
    }
    else
    {

    }
}
