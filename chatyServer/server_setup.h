#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#include "../shared/application_ports.h"
#include "../shared/my_rsa_keys_handler.h"
#include "main_server.h"
#include <QtWidgets>
#include <QtNetwork>
#include <QDebug>
#include <stdlib.h>

class ServerSetup
{
    public:
        ServerSetup();
        ~ServerSetup();
        MyRsaKeysHandler *getMyRsaKeysHandler();
        bool isInitialized() const;

private:
        bool initialisationComplete = false;
        MyRsaKeysHandler* myRsaKeysHandler;
        MainServer server;
};

#endif // SERVER_SETUP_H
