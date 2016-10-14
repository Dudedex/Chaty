#ifndef SERVERSETUP_H
#define SERVERSETUP_H

#include "application_ports.h"
#include "main_server.h"
#include <QtNetwork>
#include <QDebug>

class ServerSetup
{
public:
    ServerSetup();
private:
    MainServer server;
};

#endif // SERVERSETUP_H
