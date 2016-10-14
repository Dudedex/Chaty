#include "tcp_client.h"

TcpClient::TcpClient(std::string host, int port)
{
    this->host = host;
    this->port = port;
}

void TcpClient::transmitDataToServer(QByteArray data)
{
    //Begin 1.
    if(!tcpSocket.isOpen()){
        tcpSocket.connectToHost(QString::fromStdString(host), port);
        if(!tcpSocket.waitForConnected(5000)){
            qDebug() << "Cant connect to server";
            //Fehlerbehandlung wenn Host nicht erreichbar ist
            return;
        }
    }
    //Ende 1.

    //Begin 2.
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    //DataStream Version, muss gleich der im Client sein
    out.setVersion(QDataStream::Qt_5_6);
    out << (qint32)0;
    //Schreibe Daten in Bytes Block
    out << data;
    //Schreibe Gesamtblocklänge an den Anfang der Nachricht
    out.device()->seek(0);
    out << (qint32)(block.size() - sizeof(qint32));
    //Ende 2.

    //3.
    tcpSocket.write(block);

    //Begin 4.
    if(!tcpSocket.waitForReadyRead()){
        qDebug() << "No response";
        //Fehlerbehandlung wenn Host nicht antwortet, Timeout 30sec
        return;
    }
    qDebug() << "Erster Nachrichtenblock der Antwort verfuegbar";
    //Ende 4.

    //Begin 5.
    qint32 blockSize = 0;
    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    if(tcpSocket.bytesAvailable()){
        in >> blockSize;
    }
    qDebug() << "Nachrichtenlaenge: " << blockSize << " Bytes";
    //Ende 5.

    //Begin 6.
    qDebug() << "Lade vollstaendige Nachricht";
    while (tcpSocket.bytesAvailable() < (blockSize - sizeof(qint32))) {
        if (!tcpSocket.waitForReadyRead()) {
            break;
        }
    }
    qDebug() << "Gesamte Nachricht geladen! Verfuegbare Bytes: "
             << tcpSocket.bytesAvailable();
    //Ende 6.

    //Begin 7.
    QByteArray responseBuffer;
    in >> responseBuffer;
    qDebug() << "Schreibe Antwort in Response Objekt";
    this->response = responseBuffer;
    //Ende 7.

    //8.
    if(tcpSocket.waitForDisconnected()){
        qDebug() << "Verbindung beendet";
    }

    //9.
    tcpSocket.close();
    qDebug() << "Socket geschlossen";
}

QByteArray TcpClient::getResponse() const
{
    return response;
}
