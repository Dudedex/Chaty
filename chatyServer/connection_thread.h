#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QtNetwork>
#include <QDateTime>
#include <QDebug>
#include <set>
#include "mysql_wrapper.h"
#include "../shared/transmissiontype.h"
#include "../shared/request_json_wrapper.h"
#include "../shared/response_json_wrapper.h"
#include "../shared/crypto_helper.h"
#include "../shared/tcp_client.h"
#include "../shared/my_rsa_keys_handler.h"

class ConnectionThread: public QThread
{
    Q_OBJECT
    
    public:
        ConnectionThread(int socketDescriptor, QObject *parent);
        void run();
        void setMyRsaKeysHandler(MyRsaKeysHandler *value);

signals:
        void error(QTcpSocket::SocketError socketError);
    
    private:
        MyRsaKeysHandler* myRsaKeysHandler;
        int socketDescriptor;
        MysqlWrapper* mysqlWrapper;
        ResponseJsonWrapper executeClientActionBasedOnMessageType(std::string serverAddress, RequestJsonWrapper jsonObject);
        ResponseJsonWrapper executeServerActionBasedOnMessageType(RequestJsonWrapper jsonObject);
        void handleReceivedMessage(QTcpSocket &tcpSocket, RequestJsonWrapper jsonObject, ResponseJsonWrapper &output);
        ResponseJsonWrapper generateRSAPubKeyResponseForServer();
        void executeActionIfRightsExists(RequestJsonWrapper &jsonObject, QTcpSocket &tcpSocket, ResponseJsonWrapper &output);
};



#endif // CONNECTIONTHREAD_H
