#include <QCoreApplication>
#include <QDebug>
#include "tcp_client.h"
#include "application_ports.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "Client LOG";
    QByteArray data("Dies ist eine Zeichenkette! 10 MB lang");
    //10.485.760 Bytes
    data.resize(10*1024*1024);
    qDebug() << "Sende Nachricht: \"" << data.toStdString().c_str()
             << "\" mit Laenge: " << data.size() << " Bytes";
    TcpClient tcpClient("127.0.0.1", SERVER_PORT);
    tcpClient.transmitDataToServer(data);
    qDebug() << "Nachricht erhalten: \""
             << tcpClient.getResponse().toStdString().c_str()
             << "\" mit Laenge: " << tcpClient.getResponse().size();
    return a.exec();
}
