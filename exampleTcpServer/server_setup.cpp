#include "server_setup.h"

ServerSetup::ServerSetup()
{
    qDebug()<< "SERVER LOG";
    if (!server.listen(QHostAddress::Any, SERVER_PORT)) {
        qDebug()<< "Unable to start server " << server.errorString();
        return;
    } else {
        qDebug() << "Server is running on port: "<< server.serverPort();
    }
}
