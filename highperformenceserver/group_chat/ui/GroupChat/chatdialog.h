#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "def.h"

namespace Ui {
class ChatDialog;
}

#define BUFFER_SIZE 1024

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = 0);
    ~ChatDialog();
    void PollThreadProc();

public slots:
    void start(int connfd);

private slots:
    void on_sendBtn_clicked();

private:
    Ui::ChatDialog *ui;
    int m_connfd;
    char sendbuff[BUFFER_SIZE];
    char recvbuff[BUFFER_SIZE];
    size_t recvpos = 0;
    size_t sendpos = 0;
    CThread *poll_thread;
};

#endif // CHATDIALOG_H
