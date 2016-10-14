#include "current_configuration.h"

CurrentConfiguration::CurrentConfiguration()
{

}

CurrentConfiguration::~CurrentConfiguration()
{
    if(myRsaKeysHandler != 0){
        delete myRsaKeysHandler;
    }
}

std::string CurrentConfiguration::getCurrentUsernameAtDomainPair() const
{
    return currentUsernameAtDomainPair;
}

std::string CurrentConfiguration::getCurrentAlias() const
{
    return currentAlias;
}

std::string CurrentConfiguration::getCurrentPassword() const
{
    return currentPassword;
}

std::string CurrentConfiguration::getCurrentDeviceIdentifier() const
{
    return currentDeviceIdentifier;
}

std::string CurrentConfiguration::getCurrentMediaPath() const
{
    return currentMediaPath;
}

bool CurrentConfiguration::getSaveMessagesOnServer() const
{
    return saveMessagesOnServer;
}

int CurrentConfiguration::getHeartbeatIntervall() const
{
    return heartbeatIntervall;
}

int CurrentConfiguration::getRefeshIpIntervall() const
{
    return refeshIpIntervall;
}

int CurrentConfiguration::getPullMessagesIntervall() const
{
    return pullMessagesIntervall;
}

int CurrentConfiguration::getCurrentReceiverIndex() const
{
    return currentReceiverIndex;
}

MyRsaKeysHandler *CurrentConfiguration::getMyRsaKeysHandler() const
{
    return myRsaKeysHandler;
}

std::string CurrentConfiguration::getCurrentServerRSAPublicKey() const
{
    return currentServerRSAPublicKey;
}

void CurrentConfiguration::initConfigurationValues()
{
    currentUsernameAtDomainPair = controller->getConfigurationValueFromDB(USERNAME_DB_IDENTIFIER);
    currentAlias = controller->getConfigurationValueFromDB(ALIAS_DB_IDENTIFIER);
    currentPassword = controller->getConfigurationValueFromDB(PASSWORD_DB_IDENTIFIER);
    currentDeviceIdentifier = controller->getConfigurationValueFromDB(DEVICEIDENTIFIER_DB_IDENTIFIER);
    currentServerRSAPublicKey = controller->getConfigurationValueFromDB(SERVER_RSA_PUB_KEY);

    std::string pullMessagesString = controller->getConfigurationValueFromDB(MESSAGES_REFRESH_INTERVALL);
    if(pullMessagesString.compare("") == 0) {
        pullMessagesIntervall = PULL_MESSAGES_INTERVALL_DEFAULT_VALUE;
    } else {
        pullMessagesIntervall = atoi( pullMessagesString.c_str() );
    }

    std::string heartbeatString = controller->getConfigurationValueFromDB(HEARTBEAT_REFRESH_INTERVALL);
    if(heartbeatString.compare("") == 0) {
        heartbeatIntervall = HEARTBEAT_INTERVALL_DEFAULT_VALUE;
    } else {
        heartbeatIntervall = atoi( heartbeatString.c_str() );
    }

    std::string ipRefeshString = controller->getConfigurationValueFromDB(IP_REFRESH_INTERVALL);
    if(ipRefeshString.compare("") == 0) {
        refeshIpIntervall = IP_REFRESH_INTERVALL_DEFAULT_VALUE;
    } else {
        refeshIpIntervall = atoi( ipRefeshString.c_str() );
    }
}

void CurrentConfiguration::setCurrentUsernameAtDomainPair(const std::string &value)
{
    currentUsernameAtDomainPair = value;
    if(controller != 0){
        controller->saveConfigurationToDB(USERNAME_DB_IDENTIFIER, currentUsernameAtDomainPair);
    } else {
        qDebug()<< "ERROR: controller not defined!";
    }
}

void CurrentConfiguration::setMyRsaKeysHandler(MyRsaKeysHandler *value)
{
    myRsaKeysHandler = value;
}

void CurrentConfiguration::setSaveMessagesOnServer(bool value)
{
    saveMessagesOnServer = value;
}

void CurrentConfiguration::setCurrentAlias(const std::string &value)
{
    currentAlias = value;
    if(controller != 0){
        controller->saveConfigurationToDB(ALIAS_DB_IDENTIFIER, currentAlias);
    } else {
        qDebug()<< "ERROR: controller not defined!";
    }
}

void CurrentConfiguration::setCurrentPassword(const std::string &value)
{
    currentPassword = value;
    if(controller != 0){
        controller->saveConfigurationToDB(PASSWORD_DB_IDENTIFIER, currentPassword);
    } else {
        qDebug()<< "ERROR: controller not defined!";
    }
}

void CurrentConfiguration::setCurrentDeviceIdentifier(const std::string &value)
{
    currentDeviceIdentifier = value;
    if(controller != 0){
        controller->saveConfigurationToDB(DEVICEIDENTIFIER_DB_IDENTIFIER, currentDeviceIdentifier);
    } else {
        qDebug()<< "ERROR: controller not defined!";
    }
}

void CurrentConfiguration::setController(MainwindowController *value)
{
    controller = value;
}

void CurrentConfiguration::setCurrentMediaPath(const std::string &value)
{
    currentMediaPath = value;
}

void CurrentConfiguration::setCurrentReceiverIndex(int value)
{
    currentReceiverIndex = value;
}

void CurrentConfiguration::setServerRSAPublicKey(const std::string &value)
{
    currentServerRSAPublicKey = value;
    if(controller != 0){
        controller->saveConfigurationToDB(SERVER_RSA_PUB_KEY, currentServerRSAPublicKey);
    } else {
        qDebug()<< "ERROR: controller not defined!";
    }
}

void CurrentConfiguration::setRefeshIpIntervall(int value)
{
    refeshIpIntervall = value;
    if(controller != 0){
        std::string value = CryptoHelper::saveStringConversion(QString::number(refeshIpIntervall));
        controller->saveConfigurationToDB(IP_REFRESH_INTERVALL, value);
    } else {
        qDebug()<< "ERROR: controller not defined!";
    }
}

void CurrentConfiguration::setHeartbeatIntervall(int value)
{
    heartbeatIntervall = value;
    if(controller != 0){
         std::string value = CryptoHelper::saveStringConversion(QString::number(heartbeatIntervall));
        controller->saveConfigurationToDB(HEARTBEAT_REFRESH_INTERVALL, value);
    } else {
        qDebug()<< "ERROR: controller not defined!";
    }
}

void CurrentConfiguration::setPullMessagesIntervall(int value)
{
    pullMessagesIntervall = value;
    if(controller != 0){
        std::string value = CryptoHelper::saveStringConversion(QString::number(pullMessagesIntervall));
        controller->saveConfigurationToDB(MESSAGES_REFRESH_INTERVALL, value);
    } else {
        qDebug()<< "ERROR: controller not defined!";
    }
}

