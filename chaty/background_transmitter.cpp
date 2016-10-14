#include "background_transmitter.h"

BackgroundTransmitter::BackgroundTransmitter(std::string serverAdress, int port, RequestJsonWrapper messageJsonObject)
{
    this->serverAdress = serverAdress;
    this->port = port;
    this->messageJsonObject = messageJsonObject;
}

void BackgroundTransmitter::run()
{
    TcpClient communication(serverAdress, port);
    communication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(messageJsonObject.getJsonString()));
    ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(communication.getResponse()));
    if(response.getStatusCode() == OK || response.getStatusCode() == UNAUTHORIZED){
        if(response.getMessage().compare("") != 0){
            qDebug() << "setArrived time";
            QDateTime returnedTime;
            returnedTime.setTime_t(atoi(response.getMessage().c_str()));
            messageJsonObject.setTimestamp(returnedTime);
        }       
    }
    emit backgroundTransmissionFinished(CryptoHelper::saveStringQByteConversion(messageJsonObject.getJsonString()), response.getStatusCode(), this);
}

RequestJsonWrapper BackgroundTransmitter::getMessageJsonObject() const
{
    return messageJsonObject;
}

int BackgroundTransmitter::getPort() const
{
    return port;
}

std::string BackgroundTransmitter::getServerAdress() const
{
    return serverAdress;
}
