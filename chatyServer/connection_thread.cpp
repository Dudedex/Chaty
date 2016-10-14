#include "connection_thread.h"

ConnectionThread::ConnectionThread(int socketDescriptor, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor)
{
    qDebug() << "START NEW THREAD";
}

ResponseJsonWrapper ConnectionThread::executeClientActionBasedOnMessageType(std::string serverAddress, RequestJsonWrapper jsonObject)
{
    switch(jsonObject.getMessageType()){
        case SEND_MESSAGE:
            qDebug() << "Client SEND_MESSAGE";
            jsonObject.setDeviceIdentifier("");
            return mysqlWrapper->insertMessage(jsonObject);
            break;
        case HEARTBEAT:
            qDebug() << "Client HEARTBEAT";
            return mysqlWrapper->handleHeartbeat(jsonObject, serverAddress);
            break;
        case GET_ALL_MESSAGES_FOR_CHAT:
            qDebug() << "Client GET_ALL_MESSAGES_FOR_CHAT";
            return mysqlWrapper->getAllMessagesForChat(jsonObject);
            break;
        case GET_RSA_KEY_FOR_USER:
            qDebug() << "Client GET_RSA_KEY_FOR_USER";
            return mysqlWrapper->getRsaPublicKeyForUser(jsonObject);
            break;
        case SET_RSA_KEY_FOR_USER:
            qDebug() << "Client SET_RSA_KEY_FOR_USER";
            return mysqlWrapper->setRsaPublicKeyForUser(jsonObject);
            break;
        case GET_IP_ADDRESS_FOR_USERS:
            qDebug() << "Client GET_IP_ADRESS_FOR_USERS";
            return mysqlWrapper->getIpAddressesForUsers(jsonObject);
            break;
        case GET_PW_SALT_FOR_USER:
            qDebug() << "Client GET_PW_SALT_FOR_USER";
            return mysqlWrapper->getPwSaltForUser(jsonObject);
            break;
        case CHECK_LOGIN_CREDENTIALS:
            qDebug() << "Client CHECK_LOGIN_CREDENTIALS";
            return mysqlWrapper->checkLoginCredentials(jsonObject, serverAddress, myRsaKeysHandler->getPrivateKey());
            break;
        case GET_RSA_KEY_FOR_SERVER:
            qDebug() << "Client CHECK_LOGIN_CREDENTIALS";
            return generateRSAPubKeyResponseForServer();
            break;
        default:
            qDebug() << "Client DEFAULT";
            ResponseJsonWrapper responseJson;
            responseJson.setStatusCode(NOT_IMPLEMENTED);
            return responseJson;
            break;
    }
}

ResponseJsonWrapper ConnectionThread::executeServerActionBasedOnMessageType(RequestJsonWrapper jsonObject)
{
    switch(jsonObject.getMessageType()){
        case SEND_MESSAGE:
            jsonObject.setTimestamp(QDateTime::currentDateTime().addSecs(1));
            return mysqlWrapper->insertMessage(jsonObject);
            break;
        case GET_IP_ADDRESS_FOR_USERS:
            return mysqlWrapper->getIpAddressesForUsers(jsonObject);
            break;
        case GET_RSA_KEY_FOR_USER:
            return mysqlWrapper->getRsaPublicKeyForUser(jsonObject);
            break;
        default:
            qDebug() << "Method NOT IMPLEMENTED";
            ResponseJsonWrapper responseJson;
            responseJson.setStatusCode(NOT_IMPLEMENTED);
            return responseJson;
            break;
    }
}

void ConnectionThread::handleReceivedMessage(QTcpSocket &tcpSocket, RequestJsonWrapper jsonObject, ResponseJsonWrapper &output)
{
    if(jsonObject.getReceivers().size() > 0 &&
            (jsonObject.getMessageType() == SEND_MESSAGE
            || jsonObject.getMessageType() == GET_IP_ADDRESS_FOR_USERS
            || jsonObject.getMessageType() == GET_RSA_KEY_FOR_USER)){
        std::set<std::string> serverAddresses;
        for(unsigned int i = 0; i < jsonObject.getReceivers().size(); i++){
            std::string serverAddress = CryptoHelper::parseServerAdressOutOfUsername(jsonObject.getReceivers()[i]);
            qDebug() << "ServerAddress: " << serverAddress.c_str();
            qDebug() << "To contacting server " << serverAddress.c_str() << " is trusted " << mysqlWrapper->isTransmitterTrustedServer(serverAddress);
            if(!mysqlWrapper->isTransmitterTrustedServer(serverAddress) && serverAddress.compare(CryptoHelper::parseServerAdressOutOfUsername(jsonObject.getTransmitter())) != 0){
                 output.setStatusCode(FORBIDDEN);
                 return;
            }
            serverAddresses.insert(serverAddress);
        }
        qDebug() << "Size of ServerAddresses " << serverAddresses.size();
        bool rsaKeyReceived = false;
        for(auto serverAddress : serverAddresses){
            qDebug() << "Working on Address: " << serverAddress.c_str();
            if(serverAddress.compare("") != 0 &&
                    (serverAddress.compare(CryptoHelper::parseServerAdressOutOfUsername(jsonObject.getTransmitter())) != 0)){
                TcpClient communication(serverAddress, SERVER_PORT);
                communication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(jsonObject.getJsonString()));
                ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(communication.getResponse()));
                output.mergeResponseObjectIntoThis(response);
                if(jsonObject.getMessageType() == GET_RSA_KEY_FOR_USER){
                    rsaKeyReceived = true;
                    break;
                }
                if(response.getStatusCode() != OK){
                    return;
                }
            }
        }
        if(!rsaKeyReceived){
            output.mergeResponseObjectIntoThis(executeClientActionBasedOnMessageType(CryptoHelper::saveStringConversion(tcpSocket.peerAddress().toString()), jsonObject));
        }
    } else {
        output = executeClientActionBasedOnMessageType(CryptoHelper::saveStringConversion(tcpSocket.peerAddress().toString()), jsonObject);
    }
}

ResponseJsonWrapper ConnectionThread::generateRSAPubKeyResponseForServer()
{
    ResponseJsonWrapper response;
    response.setMessage(myRsaKeysHandler->getPublicKey());
    response.setStatusCode(OK);
    return response;
}

void ConnectionThread::executeActionIfRightsExists(RequestJsonWrapper &jsonObject, QTcpSocket &tcpSocket, ResponseJsonWrapper &output)
{
    if(mysqlWrapper->isTransmitterTrustedServer(CryptoHelper::saveStringConversion(tcpSocket.peerAddress().toString()))){
        output = executeServerActionBasedOnMessageType(jsonObject);
    }else {
        qDebug() << "Name: " << jsonObject.getTransmitter().c_str() << tcpSocket.peerAddress().toString() << ":" << tcpSocket.peerPort();
        if(CryptoHelper::parseServerAdressOutOfUsername(jsonObject.getTransmitter()).compare("") == 0){
            qDebug() << "MessageType: " << jsonObject.getMessageType() << " is missing serveraddress for username";
            output.setStatusCode(BAD_REQUEST);
        } else {
            if(jsonObject.getMessageType() != CHECK_LOGIN_CREDENTIALS &&
                    jsonObject.getMessageType() != GET_PW_SALT_FOR_USER &&
                    jsonObject.getMessageType() != GET_RSA_KEY_FOR_USER &&
                    jsonObject.getMessageType() != GET_RSA_KEY_FOR_SERVER &&
                    jsonObject.getMessageType() != HEARTBEAT){
                int authCode = mysqlWrapper->isDeviceIdentifierValid(jsonObject);
                if (authCode == OK){
                    handleReceivedMessage(tcpSocket, jsonObject, output);
                } else {
                    qDebug() << ">>>>>>>>>>>>>> AUTH TOKE INVALID <<<<<<<<<<<<<<<";
                    qDebug() << "MessageType: " << jsonObject.getMessageType();
                    output.setStatusCode(UNAUTHORIZED);
                }
            } else {
                handleReceivedMessage(tcpSocket, jsonObject, output);
            }
        }
    }
}

void ConnectionThread::run()
{
    QTcpSocket tcpSocket;
    if (!tcpSocket.setSocketDescriptor(socketDescriptor)) {
        emit error(tcpSocket.error());
        return;
    }

    tcpSocket.isOpen();
    if(!tcpSocket.waitForReadyRead()){
        qDebug() << "no Data Available";
        return;
    }
    qint32 blockSize = 0;
    QDataStream in(&tcpSocket);
    //QT TCP Version Number, must be equal to the one used in the client
    in.setVersion(QDataStream::Qt_4_0);
    if(tcpSocket.bytesAvailable()){
        in >> blockSize;
    }
    qDebug() << "Blocksize: " << blockSize;
    while (tcpSocket.bytesAvailable() < (blockSize - sizeof(qint32))) {
        if (!tcpSocket.waitForReadyRead()) {
            break;
        }
    }

    QByteArray data;
    in >> data;

    std::string jsonString = CryptoHelper::saveStringQByteConversion(data);
    RequestJsonWrapper jsonObject(jsonString);

    if(jsonObject.getJsonString().length() < 500){
        qDebug() << "Request: " << jsonObject.getJsonString().c_str();
    } else {
        qDebug() << "Request transmitter: " << jsonObject.getTransmitter().c_str();
        qDebug() << "Request message size: " << jsonObject.getJsonString().length();
        qDebug() << "Request Messagetype: " << jsonObject.getMessageType();
    }

    ResponseJsonWrapper output;
    output.setStatusCode(0);

    mysqlWrapper = new MysqlWrapper();
    executeActionIfRightsExists(jsonObject, tcpSocket, output);
    delete mysqlWrapper;

    if(output.getJsonString().length() < 500){
        qDebug() << "Response: " << output.getJsonString().c_str();
    } else {
       qDebug() << "Response size: " << output.getJsonString().length();
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (qint32)0;
    out << CryptoHelper::saveStringQByteConversion(output.getJsonString());
    out.device()->seek(0);
    out << (qint32)(block.size() - sizeof(qint32));
    qDebug() << "BlockSize: " << (block.size() - sizeof(qint32));

    tcpSocket.write(block);
    tcpSocket.disconnectFromHost();
    tcpSocket.waitForDisconnected();


}

void ConnectionThread::setMyRsaKeysHandler(MyRsaKeysHandler *value)
{
    myRsaKeysHandler = value;
}
