#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QTcpServer>
#include "connection_thread.h"

class MainServer : public QTcpServer
{
    Q_OBJECT

public:
    MainServer(){}

protected:
    void incomingConnection(qintptr socketDescriptor);
};

#endif // MAINSERVER_H
