#ifndef CURRENTCONFIGURATION_H
#define CURRENTCONFIGURATION_H 

#include <QString>
#include <QDebug>
#include "../shared/my_rsa_keys_handler.h"
#include "mainwindow_controller.h"

class MainwindowController;

const int HEARTBEAT_INTERVALL_DEFAULT_VALUE = 60;
const int IP_REFRESH_INTERVALL_DEFAULT_VALUE = 60;
const int PULL_MESSAGES_INTERVALL_DEFAULT_VALUE = 10;

class CurrentConfiguration
{
    public:
        CurrentConfiguration();
        ~CurrentConfiguration();
        std::string getCurrentUsernameAtDomainPair() const;
        std::string getCurrentAlias() const;
        std::string getCurrentPassword() const;
        std::string getCurrentDeviceIdentifier() const;
        std::string getCurrentMediaPath() const;
        bool getSaveMessagesOnServer() const;
        int getRefeshIpIntervall() const;
        int getHeartbeatIntervall() const;
        int getPullMessagesIntervall() const;
        int getCurrentReceiverIndex() const;
        MyRsaKeysHandler *getMyRsaKeysHandler() const;
        std::string getCurrentServerRSAPublicKey() const;
        void initConfigurationValues();
        void setCurrentUsernameAtDomainPair(const std::string &value);
        void setMyRsaKeysHandler(MyRsaKeysHandler *value);
        void setSaveMessagesOnServer(bool value);
        void setCurrentAlias(const std::string &value);
        void setCurrentPassword(const std::string &value);
        void setCurrentDeviceIdentifier(const std::string &value);
        void setController(MainwindowController *value);
        void setCurrentMediaPath(const std::string &value);
        void setCurrentReceiverIndex(int value);
        void setServerRSAPublicKey(const std::string &value);
        void setRefeshIpIntervall(int value);
        void setHeartbeatIntervall(int value);
        void setPullMessagesIntervall(int value);

private:
        std::string currentUsernameAtDomainPair;
        std::string currentAlias;
        std::string currentPassword;
        std::string currentDeviceIdentifier;
        std::string currentMediaPath;
        std::string currentServerRSAPublicKey;
        int refeshIpIntervall;
        int heartbeatIntervall;
        int pullMessagesIntervall;
        int currentReceiverIndex;
        bool saveMessagesOnServer;
        MyRsaKeysHandler* myRsaKeysHandler = 0;
        MainwindowController *controller = 0;
};

#endif // CURRENTCONFIGURATION_H
