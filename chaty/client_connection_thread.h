#ifndef CLIENTCONNECTIONTHREAD_H
#define CLIENTCONNECTIONTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QtNetwork>
#include <QDateTime>
#include <QDebug>
#include <QUuid>
#include "local_storage.h"
#include "../shared/transmissiontype.h"
#include "../shared/request_json_wrapper.h"
#include "../shared/response_json_wrapper.h"
#include "../shared/crypto_helper.h"

class ClientConnectionThread: public QThread
{
    Q_OBJECT

    signals:
        void messageReceived(QString message);

    public:
        ClientConnectionThread(int socketDescriptor, QObject *parent);
        void run();

    signals:
        void error(QTcpSocket::SocketError socketError);

    private:
        int socketDescriptor;
};

#endif // CLIENTCONNECTIONTHREAD_H
