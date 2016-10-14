#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QDataStream>
#include "application_ports.h"

class TcpClient
{
    public:
        TcpClient(std::string host = "localhost", int port = SERVER_PORT);
        void transmitDataToServer(QByteArray data);
        QByteArray getResponse() const;

    private:
        QTcpSocket tcpSocket;
        std::string host;
        int port;
        QByteArray response;
};

#endif // TCPCLIENT_H
