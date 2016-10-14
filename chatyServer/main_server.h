#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QStringList>
#include <QTcpServer>
#include "connection_thread.h"
#include "../shared/my_rsa_keys_handler.h"

class MainServer : public QTcpServer
{
    Q_OBJECT

public:
    MainServer();
    void setMyRsaKeysHandler(MyRsaKeysHandler *value);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    MyRsaKeysHandler* myRsaKeysHandler;

};
#endif // MAINSERVER_H
