#include "background_scheduler.h"

BackgroundScheduler::BackgroundScheduler(CurrentConfiguration *currentConfiguration)
{
    this->configuration = currentConfiguration;
    this->terminateScheduler = false;
    this->localStorage = new LocalStorage("backgroundThreadConnection");

}

BackgroundScheduler::~BackgroundScheduler()
{
    delete localStorage;
}

bool BackgroundScheduler::getLoggedIn() const
{
    return loggedIn;
}

void BackgroundScheduler::stopScheduler()
{
    qDebug() << "Thread stoped";
    this->terminateScheduler = true;
}

void BackgroundScheduler::setLoggedIn(bool value)
{
    loggedIn = value;
}

void BackgroundScheduler::setHeartbeatIntervall(int value)
{
    heartbeatIntervall = value;
}

void BackgroundScheduler::setRefreshIpIntervall(int value)
{
    refreshIpIntervall = value;
}

void BackgroundScheduler::setPullMessagesIntervall(int value)
{
    pullMessagesIntervall = value;
}

void BackgroundScheduler::run()
{
    int heartbeatCounter = 0;
    int refreshIpCounter = 0;
    int pullMessagesCounter = 0;
    while(!terminateScheduler){
        if(heartbeatCounter >= heartbeatIntervall && loggedIn){
            qDebug() << "HEARTBEAT";
            std::string deviceIdentifier = configuration->getCurrentDeviceIdentifier();
            TcpClient tcpCommunication(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()));
            RequestJsonWrapper requestJsonObj(configuration->getCurrentUsernameAtDomainPair(), "", QDateTime::currentDateTime(), deviceIdentifier, HEARTBEAT);
            tcpCommunication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(requestJsonObj.getJsonString()));
            ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(tcpCommunication.getResponse()));
            qDebug() << "RESPONSE " << response.getJsonString().c_str();
            if(response.getStatusCode() == OK){
                MultiLinesResponseJsonWrapper lines(response.getMessage());
                if(!lines.getLines().empty()){
                    qDebug() << "Messages FOUND";
                    emit messagesAvailable(CryptoHelper::saveStringConversion(CryptoHelper::saveStringQByteConversion(tcpCommunication.getResponse())));
                }
                heartbeatCounter = 0;
            } else if(response.getStatusCode() == UNAUTHORIZED){
                loggedIn = false;
                emit authenticationRequired();
            } else if(response.getStatusCode() == FORBIDDEN){
                loggedIn = false;
                emit loggedOut();
            } else {
                heartbeatCounter = 0;
            }

        }

        if(refreshIpCounter >= refreshIpIntervall && loggedIn){
            qDebug() << "REFRESHING IP";
            std::vector<Contact> knownUsers = localStorage->getKnownContacts();
            std::vector<std::string> receivers;
            for(unsigned int i = 0; i < knownUsers.size(); i++){
                receivers.push_back(knownUsers[i].getUsername());
            }
            if(receivers.size() > 0){
                TcpClient tcpCommunication(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()));
                RequestJsonWrapper requestJsonObj(configuration->getCurrentUsernameAtDomainPair(), receivers, QDateTime::currentDateTime(), configuration->getCurrentDeviceIdentifier(), GET_IP_ADDRESS_FOR_USERS);
                tcpCommunication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(requestJsonObj.getJsonString()));
                qDebug() << "IP- REFRESH: " << requestJsonObj.getJsonString().c_str();
                ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(tcpCommunication.getResponse()));
                qDebug() << "IP UPDATE" << CryptoHelper::saveStringQByteConversion(tcpCommunication.getResponse()).c_str();
                if(response.getStatusCode() == OK){
                    emit refreshIpAdresses(CryptoHelper::saveStringConversion(response.getMessage()));
                }
            }
            refreshIpCounter = 0;
        }

        if(pullMessagesCounter >= pullMessagesIntervall && loggedIn) {
            qDebug() << "PULLING MESSAGES";
            std::vector<Contact> knownUsers = localStorage->getKnownContacts();
            if(configuration->getCurrentReceiverIndex() > -1){
                emit pullMessagesByUsername(CryptoHelper::saveStringConversion(knownUsers[configuration->getCurrentReceiverIndex()].getUsername()));
            }
            pullMessagesCounter = 0;
        }

        heartbeatCounter++;
        refreshIpCounter++;
        pullMessagesCounter++;
        msleep(1000);
    }
}


