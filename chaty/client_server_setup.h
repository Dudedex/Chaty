#ifndef CLIENTSERVERSETUP_H
#define CLIENTSERVERSETUP_H

#include "client_server.h"
#include "../shared/application_ports.h"
#include <QtWidgets>
#include <QtNetwork>
#include <QDebug>
#include <stdlib.h>
#include <QMessageBox>

class ClientServerSetup: public QObject
{
        Q_OBJECT
    signals:
        void messageReceived(QString message);
    public slots:
        void onMessageReceived(QString message);
    public:
        ClientServerSetup();
    public:
        ClientServer clientServer;
};

#endif // CLIENTSERVERSETUP_H
