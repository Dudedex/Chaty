#include "client_server.h"

void ClientServer::onMessageReceived(QString message)
{
    qDebug() << "ClientServer emit message";
    emit messageReceived(message);
}

ClientServer::ClientServer()
{

}

void ClientServer::incomingConnection(qintptr socketDescriptor)
{
    ClientConnectionThread *thread = new ClientConnectionThread(socketDescriptor, this);
    QObject::connect(thread, SIGNAL(messageReceived(QString)), this,
                              SLOT(onMessageReceived(QString)));
    QObject::connect(thread, SIGNAL(finished()), thread,
                              SLOT(deleteLater()));
    thread->start();
}
