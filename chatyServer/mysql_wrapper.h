#ifndef MSQLWRAPPER_H
#define MSQLWRAPPER_H

#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QDateTime>
#include <QUuid>
#include <iostream>
#include "../shared/request_json_wrapper.h"
#include "../shared/transmissiontype.h"
#include "../shared/response_json_wrapper.h"
#include "../shared/crypto_helper.h"
#include "../shared/ip_refresh_json_wrapper.h"
#include "../shared/login_message_json_wrapper.h"
#include "../shared/file_io_helper.h"
#include "../shared/rsa_wrapper.h"

class MysqlWrapper
{
    public:
        MysqlWrapper();
        ~MysqlWrapper();
        ResponseJsonWrapper insertMessage(RequestJsonWrapper &jsonObject);
        ResponseJsonWrapper handleHeartbeat(RequestJsonWrapper &jsonObject, std::string ipAddress);
        ResponseJsonWrapper getAllMessagesForChat(RequestJsonWrapper &jsonObject);
        ResponseJsonWrapper getRsaPublicKeyForUser(RequestJsonWrapper &jsonObject);
        ResponseJsonWrapper setRsaPublicKeyForUser(RequestJsonWrapper &jsonObject);
        ResponseJsonWrapper getIpAddressesForUsers(RequestJsonWrapper &jsonObject);
        ResponseJsonWrapper getPwSaltForUser(RequestJsonWrapper &jsonObject);
        ResponseJsonWrapper checkLoginCredentials(RequestJsonWrapper &jsonObject, std::string ipAddress, std::string serverRsaPrivateKey);
        int isDeviceIdentifierValid(RequestJsonWrapper &jsonObject);
        bool isTransmitterTrustedServer(std::string address);
        int loadIntDBConfig(std::string key);

        static int maxAllowedPacket;

    private:
        QSqlDatabase db;
        bool areLoginCredentialsCorrect(std::string username, std::string password);
        bool isOldDeviceIdentifierValid(std::string oldDeviceIdentifier);
        QString getLastExecutedQuery(const QSqlQuery& query);
        bool insertDeviceInformation(RequestJsonWrapper &jsonObject, std::string deviceIdentifier, std::string ipAddress, QDateTime lastIpUpdate);
        bool updateDeviceInformation(RequestJsonWrapper &jsonObject, std::string oldDeviceIdentifier, std::string newDeviceIdentifier, std::string ipAddress, QDateTime lastIpUpdate);
        std::string getLastDeviceIdentifier(RequestJsonWrapper &jsonObject, std::string &ipAddress);
        std::vector<std::string> getIpAddressesForUser(std::string username);
        ResponseJsonWrapper getRsaPublicKeyByUsername(std::string username);
};

#endif // MSQLWRAPPER_H
