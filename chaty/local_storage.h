#ifndef LOCALSTORAGE_H
#define LOCALSTORAGE_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <openssl/sha.h>
#include "../shared/response_json_wrapper.h"
#include "../shared/ip_refresh_json_wrapper.h"
#include "../shared/request_json_wrapper.h"
#include "../shared/crypto_helper.h"
#include "contact.h"

const std::string USERNAME_DB_IDENTIFIER = "usernameAtDomain";
const std::string ALIAS_DB_IDENTIFIER = "alias";
const std::string PASSWORD_DB_IDENTIFIER = "password";
const std::string DEVICEIDENTIFIER_DB_IDENTIFIER = "devicename";
const std::string SERVER_RSA_PUB_KEY = "serverRsaPub";
const std::string HEARTBEAT_REFRESH_INTERVALL = "heartbeatIntervall";
const std::string IP_REFRESH_INTERVALL = "ipRefreshIntervall";
const std::string MESSAGES_REFRESH_INTERVALL = "pullMessagesIntervall";

class LocalStorage
{
    public:
        static QString DATABASE_NAME;
        LocalStorage(std::string connectionName);
        bool getStatus() const;
        bool insertMessagesInDB(ResponseJsonWrapper response);
        bool insertMessagesInDB(RequestJsonWrapper message);
        std::string getConfigurationKey(std::string key);
        bool setConfigurationKey(std::string key, std::string value);
        std::vector<std::string> getMessagesForChat(std::string user, std::string chatReceiver);
        QDateTime getLatestsTimeStampForChat(std::string chatReceiver);
        bool setLatestsTimeStampForChat(QDateTime dateTime, std::string chatReceiver);
        std::vector<Contact> getKnownContacts();
        Contact getContactByUsername(std::string username);
        bool addNewContact(std::string username, std::string alias, std::string rsaKey);
        bool refreshIpAddressesByUsername(IpRefreshJsonWrapper ipObject);
        std::vector<std::string> getIpAddressesByUsername(std::string username);
        bool deleteContactByUsername(std::string username);
        bool updateContactAliasByUsername(std::string username, std::string alias);
        bool updateContactRSAKeyByUsername(std::string username, std::string rsaKey);

    private:
        QString getLastExecutedQuery(const QSqlQuery &query);
        QSqlDatabase db;
        bool status;

};

#endif // LOCALSTORAGE_H
