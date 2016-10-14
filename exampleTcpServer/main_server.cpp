#include "main_server.h"

void MainServer::incomingConnection(qintptr socketDescriptor)
{
    ConnectionThread *thread = new ConnectionThread(socketDescriptor, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}
