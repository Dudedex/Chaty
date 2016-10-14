#include "connection_thread.h"

ConnectionThread::ConnectionThread(int socketDescriptor, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor)
{
    qDebug() << "Neue Verbindung eingetroffen!";
}

void ConnectionThread::run()
{
    QTcpSocket tcpSocket;
    //Begin 1.
    if (!tcpSocket.setSocketDescriptor(socketDescriptor)) {
        qDebug() << "could not bind socket";
        return;
    }
    qDebug() << "Kommunikation findet auf Port "
             << tcpSocket.socketDescriptor()
             << " statt.";
    //Ende 1.

    //Begin 2.
    if(!tcpSocket.waitForReadyRead()){
        qDebug() << "no Data Available";
        return;
    }
    qDebug() << "Erster Nachrichtenblock verfuegbar";
    //Ende 2.

    //Begin 3.
    qint32 blockSize = 0;
    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    if(tcpSocket.bytesAvailable()){
        in >> blockSize;
    }
    qDebug() << "Nachrichtenlaenge: " << blockSize << " Bytes";
    //Ende 3.

    //Begin 4.
    qDebug() << "Lade vollstaendige Nachricht";
    while (tcpSocket.bytesAvailable() < (blockSize - sizeof(qint32))) {
        if (!tcpSocket.waitForReadyRead()) {
            break;
        }
    }
    qDebug() << "Gesamte Nachricht geladen! Verfuegbare Bytes: "
             << tcpSocket.bytesAvailable();
    //Ende 4.

    //Begin 5.
    QByteArray data;
    in >> data;
    //Ende 5.

    //Begin 6. Datenverarbeitung
    QByteArray output;
    output = QByteArray::fromStdString("Echo: " + data.toStdString());
    qDebug() << "Antwort: " << output.toStdString().c_str();
    qDebug() << "Antwortlaenge: " << output.size() << " Bytes";
    //Ende 6.

    // Begin 7.
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);
    //Platzhalter für spätere Gesamtblocklänge der Nachricht
    out << (qint32)0;
    out << output;
    //Schreibe Gesamtblocklänge an den Anfang der Nachricht
    out.device()->seek(0);
    out << (qint32)(block.size() - sizeof(qint32));
    //Ende 7.

    //8.
    tcpSocket.write(block);
    qDebug() << "Sende Antwort!";

    //9.
    qDebug() << "Beende Verbindung.";
    tcpSocket.disconnectFromHost();
    qDebug() << "Verbindung beendet.";
}
