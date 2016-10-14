#ifndef CLIENTSERVER_H
#define CLIENTSERVER_H

#include <QStringList>
#include <QTcpServer>
#include "client_connection_thread.h"

class ClientServer : public QTcpServer
{
    Q_OBJECT

    signals:
        void messageReceived(QString message);
    public slots:
        void onMessageReceived(QString message);
    public:
        ClientServer();
    protected:
        void incomingConnection(qintptr socketDescriptor);

};

#endif // CLIENTSERVER_H
