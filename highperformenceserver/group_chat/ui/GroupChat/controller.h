#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QWidget>
#include "connectdialog.h"
#include "chatdialog.h"

class Controller : public QWidget
{
    Q_OBJECT
public:
    explicit Controller(QWidget *parent = 0);
    ~Controller();
    void start();

signals:

public slots:

public:
    ConnectDialog *connectDlg;
    ChatDialog *chatDlg;
};

#endif // CONTROLLER_H
