#include "controller.h"

Controller::Controller(QWidget *parent) : QWidget(parent)
{
    connectDlg = new ConnectDialog;
    chatDlg    = new ChatDialog;
    connect(connectDlg, &ConnectDialog::ConnectSuccess,
            chatDlg, &ChatDialog::start);
}

void Controller::start()
{
    connectDlg->show();
}

Controller::~Controller()
{
    delete connectDlg;
    delete chatDlg;
}
