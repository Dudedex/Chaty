#ifndef HEARTBEAT_H
#define HEARTBEAT_H


#include "local_storage.h"
#include "current_configuration.h"
#include "../shared/tcp_client.h"
#include <QDebug>
#include <QThread>
#include <QObject>

class CurrentConfiguration;

class BackgroundScheduler : public QThread
{
        Q_OBJECT
    public:
        BackgroundScheduler(CurrentConfiguration *configuration);
        ~BackgroundScheduler();
        bool getLoggedIn() const;
        void stopScheduler();
        void setLoggedIn(bool value);
        void setHeartbeatIntervall(int value);
        void setRefreshIpIntervall(int value);
        void setPullMessagesIntervall(int value);

private:
        bool terminateScheduler;
        bool loggedIn;
        int heartbeatIntervall = 60;
        int refreshIpIntervall = 60;
        int pullMessagesIntervall = 10;
        LocalStorage* localStorage;
        CurrentConfiguration *configuration;
        void run();

    signals:
        void messagesAvailable(QString messagesAvailableFromUser);
        void authenticationRequired();
        void loggedOut();
        void refreshIpAdresses(QString ipAdressObjectsAsJson);
        void pullMessagesByUsername(QString username);
};

#endif // HEARTBEAT_H
