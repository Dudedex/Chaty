#include "client_connection_thread.h"


ClientConnectionThread::ClientConnectionThread(int socketDescriptor, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor)
{
}

void ClientConnectionThread::run()
{
    QTcpSocket tcpSocket;
    if (!tcpSocket.setSocketDescriptor(socketDescriptor)) {
        emit error(tcpSocket.error());
        return;
    }

    qDebug() << "Open: "  << tcpSocket.isOpen();
    if(!tcpSocket.waitForReadyRead()){
        qDebug() << "no Data Available";
        return;
    }
    qint32 blockSize = 0;
    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    if(tcpSocket.bytesAvailable()){
        in >> blockSize;
    }
    while (tcpSocket.bytesAvailable() < (blockSize - sizeof(qint32))) {
        if (!tcpSocket.waitForReadyRead()) {
            break;
        }
    }

    QByteArray data;
    in >> data;

    std::string jsonString = CryptoHelper::saveStringQByteConversion(data);

    qDebug() << "Received: " << jsonString.c_str() << "\n";
    qDebug() << "--------------------------------";

    RequestJsonWrapper jsonObject(jsonString);
    QByteArray output;
    ResponseJsonWrapper response;
    qDebug() << "Name: " << jsonObject.getTransmitter().c_str() << tcpSocket.peerAddress().toString() << ":" << tcpSocket.peerPort();
    switch(jsonObject.getMessageType()){
        case SEND_MESSAGE:
            emit messageReceived(CryptoHelper::saveStringConversion(jsonObject.getJsonString()));
            response.setStatusCode(OK);
            output = CryptoHelper::saveStringQByteConversion(response.getJsonString());
            break;
        default:
            ResponseJsonWrapper responseJson;
            responseJson.setStatusCode(NOT_IMPLEMENTED);
            output = CryptoHelper::saveStringQByteConversion(responseJson.getJsonString());
            break;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (qint32)0;
    out << output;
    out.device()->seek(0);
    out << (qint32)(block.size() - sizeof(qint32));
    qDebug() << "BlockSize: " << (block.size() - sizeof(qint32));

    tcpSocket.write(block);
    tcpSocket.disconnectFromHost();
    tcpSocket.waitForDisconnected();
}
