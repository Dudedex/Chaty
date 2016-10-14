#include "tcp_communication.h"

static const int STANDARD_PORT = 1992;

TcpCommunication::TcpCommunication(std::string host, int port)
{
    this->host = host;
    this->port = port;
}

void TcpCommunication::transmitDataToServer(QByteArray data)
{
    tcpSocket.connectToHost(CryptoHelper::saveStringConversion(host), port);
    if(!tcpSocket.waitForConnected(5000)){
        qDebug() << "Cant connect to server";
        ResponseJsonWrapper couldNotConnectObj;
        couldNotConnectObj.setStatusCode(NOT_FOUND);
        this->response = CryptoHelper::saveStringQByteConversion(couldNotConnectObj.getJsonString());
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (qint32)0;
    out << data;
    out.device()->seek(0);
    out << (qint32)(block.size() - sizeof(qint32));
    tcpSocket.write(block);
    qDebug() << "WRITTEN DATA SIZE: " << block.size();

    if(!tcpSocket.waitForReadyRead(5000)){
        qDebug() << "No response";
        ResponseJsonWrapper couldNotConnectObj;
        couldNotConnectObj.setStatusCode(NOT_FOUND);
        this->response = CryptoHelper::saveStringQByteConversion(couldNotConnectObj.getJsonString());
        return;
    }

    qint32 blockSize = 0;
    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    if(tcpSocket.bytesAvailable()){
        in >> blockSize;
    }
    while (tcpSocket.bytesAvailable() < (blockSize - sizeof(qint32))) {
        if (!tcpSocket.waitForReadyRead(100)) {
            tcpSocket.disconnectFromHost();
            break;
        }
    }
    QByteArray responseBuffer;
    in >> responseBuffer;
    this->response = responseBuffer;
}

QByteArray TcpCommunication::getResponse() const
{
    return response;
}
