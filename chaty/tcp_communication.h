#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

#include <QTcpSocket>
#include "../shared/crypto_helper.h"
#include "../shared/application_ports.h"

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

#endif // COMMUNICATIONS_H
