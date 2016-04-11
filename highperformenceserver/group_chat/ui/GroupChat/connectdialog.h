#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectDialog;
}

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = 0);
    ~ConnectDialog();
    void PollThreadProc();

private:
    bool ConnectToServer(const char *ip, const char *port);

private slots:
    void on_connectBtn_clicked();

signals:
    void ConnectSuccess(int fd);

private:
    Ui::ConnectDialog *ui;
    int m_connfd;
};

#endif // CONNECTDIALOG_H
