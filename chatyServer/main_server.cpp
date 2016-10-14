#include "main_server.h"


MainServer::MainServer()
{

}

void MainServer::incomingConnection(qintptr socketDescriptor)
{
    ConnectionThread *thread = new ConnectionThread(socketDescriptor, this);
    thread->setMyRsaKeysHandler(this->myRsaKeysHandler);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

void MainServer::setMyRsaKeysHandler(MyRsaKeysHandler *value)
{
    myRsaKeysHandler = value;
}
