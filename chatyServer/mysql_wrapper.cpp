#include "mysql_wrapper.h"

MysqlWrapper::MysqlWrapper()
{
    qDebug() << "DB Connection opened ----------------------------------";
    db = QSqlDatabase::addDatabase("QMYSQL", QUuid::createUuid().toString());
    db.setHostName("localhost");
    db.setUserName("root");
    db.setPassword("");
    db.setDatabaseName("chatyserverdb");
}

MysqlWrapper::~MysqlWrapper()
{
    db.close();
    qDebug() << "DB Connection closed ----------------------------------";
}

ResponseJsonWrapper MysqlWrapper::insertMessage(RequestJsonWrapper &jsonObject)
{
    QDateTime arriveTime = QDateTime::currentDateTime();
    jsonObject.setTimestamp(arriveTime);
    ResponseJsonWrapper responseJson;
    if(!db.open()){
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }
    bool execReturn = true;
    bool isSavedAlready = false;
    for(unsigned int i = 0; i < jsonObject.getReceivers().size(); i++){
        QSqlQuery query(db);
        query.prepare("INSERT INTO messages (transmitter, receiver, message, timestamp) "
                     "VALUES (:transmitter, :receiver, :message, :timestamp)");
        query.bindValue(":transmitter", CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter()));
        query.bindValue(":receiver", CryptoHelper::saveStringQByteConversion(jsonObject.getReceivers()[i]));
        query.bindValue(":timestamp", arriveTime);
        QByteArray messageAsBytes = CryptoHelper::saveStringQByteConversion(jsonObject.getJsonString());
        QByteArray base64Data = CryptoHelper::encodeBase64(jsonObject.getJsonString());
        if(base64Data.size() <= maxAllowedPacket){
            query.bindValue(":message", base64Data);
        } else {
            std::string filename = MESSAGE_MEDIA_FOLDER + "/" + CryptoHelper::generateSha256(jsonObject.getJsonString()) + ".chaty";
            query.bindValue(":message", CryptoHelper::saveStringQByteConversion(filename));
            if(!isSavedAlready){
                bool check = FileIOHelper::writeDataToFile(filename, base64Data);
                if(!check){
                    execReturn = false;
                    qDebug() << "Could not save File";
                }
                isSavedAlready = true;
            }
        }
        execReturn = query.exec() && execReturn;
        if(!execReturn){
            qDebug() << "Insert Message ERROR BEGIN ==============================================";
            qDebug() << query.lastError();
            qDebug() << "MYSQL ERROR END ==============================================";
        }
    }
    if(execReturn){
        responseJson.setStatusCode(OK);
    } else {
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
    }
    qDebug() << "Timestamp: " << arriveTime.toTime_t();
    responseJson.setMessage(CryptoHelper::saveStringConversion(QString::number(arriveTime.toTime_t())));
    return responseJson;

}

ResponseJsonWrapper MysqlWrapper::handleHeartbeat(RequestJsonWrapper &jsonObject, std::string ipAddress)
{
    ResponseJsonWrapper response;
    if(!db.open()){
        response.setStatusCode(INTERNAL_SERVER_ERROR);
        return response;
    }

    QSqlQuery query(db);

    query.prepare("SELECT deviceIdentifier, ipAddress, ipAddressChangedDate, tokenExpirationDate FROM `registered_devices` "
                  "WHERE  username = :username;");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter()));
    bool getDeviceDataQuery = query.exec();
    std::string deviceIdentifier;
    std::string lastipAddress;
    QDateTime tokenExpirationDate;
    QDateTime lastIpUpdate = QDateTime::currentDateTime();
    if(getDeviceDataQuery){
        bool tokenNotFound = true;
        while(query.next()){
            std::string hashedDeviceIdentifier = CryptoHelper::calculateAbstractDeviceIdentifier(jsonObject.getTransmitter(), jsonObject.getTimestamp(), jsonObject.getMessage(), CryptoHelper::saveStringConversion(query.value("deviceIdentifier").toString()));
            if(hashedDeviceIdentifier.compare(jsonObject.getDeviceIdentifier()) == 0){
                deviceIdentifier = CryptoHelper::saveStringConversion(query.value("deviceIdentifier").toString());
                lastipAddress = CryptoHelper::saveStringQByteConversion(query.value("ipAddress").toByteArray());
                lastIpUpdate = query.value("ipAddressChangedDate").toDateTime();
                tokenExpirationDate = query.value("tokenExpirationDate").toDateTime();
                tokenNotFound = false;
                break;
            }
        }
        if(tokenNotFound){
            qDebug() << "TOKEN NOT FOUND";
            response.setStatusCode(FORBIDDEN);
            return response;
        }
    } else {
        qDebug() << "Error: " << query.lastError();
    }
    query.clear();


    QDateTime arriveTime = QDateTime::currentDateTime();
    jsonObject.setTimestamp(arriveTime);

    qDebug() << "tokenCheck: " << (tokenExpirationDate < QDateTime::currentDateTime());
    qDebug() << "tokenDate: " << tokenExpirationDate.time();
    qDebug() << "currentDate" << QDateTime::currentDateTime().time();
    if(tokenExpirationDate <= QDateTime::currentDateTime()){
        qDebug() << "Token expired";
        response.setStatusCode(UNAUTHORIZED);
        return response;
    }

    if(lastipAddress.compare(ipAddress) != 0){
        qDebug() << "old: " << CryptoHelper::saveStringConversion(lastipAddress) << " new: " << CryptoHelper::saveStringConversion(ipAddress);
        lastIpUpdate = QDateTime::currentDateTime();
    }

    bool insertNewDeviceInformationQuery = insertDeviceInformation(jsonObject, deviceIdentifier, ipAddress, lastIpUpdate);
    if(!insertNewDeviceInformationQuery){
        response.setStatusCode(INTERNAL_SERVER_ERROR);
        return response;
    }

    query.prepare("SELECT transmitter FROM `messages` "
                  "WHERE receiver = :username AND timestamp > IFNULL((SELECT lastHeartbeatDate FROM registered_devices WHERE deviceIdentifier = :deviceIdentifier),0)  "
                  "GROUP BY transmitter;");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter()));
    query.bindValue(":deviceIdentifier", CryptoHelper::saveStringQByteConversion(deviceIdentifier));
    bool getOpenMessagesQuery = query.exec();

    std::vector<std::string> messagesAvailableFromUser;
    if(getOpenMessagesQuery){
        while( query.next() ){
            messagesAvailableFromUser.push_back(CryptoHelper::saveStringQByteConversion(query.value(0).toByteArray()));
        }
        qDebug() << "Users found: " << messagesAvailableFromUser.size();
    } else {
        qDebug() << "Error: " << query.lastError();
    }
    MultiLinesResponseJsonWrapper heartbeatResponse(messagesAvailableFromUser);
    query.clear();

    if(getOpenMessagesQuery && getDeviceDataQuery){
        response.setStatusCode(OK);
    } else {
        qDebug() << query.lastError();
        response.setStatusCode(INTERNAL_SERVER_ERROR);
    }
    response.setMessage(heartbeatResponse.getJsonString());
    return response;
}

ResponseJsonWrapper MysqlWrapper::getAllMessagesForChat(RequestJsonWrapper &jsonObject)
{
    ResponseJsonWrapper responseJson;
    if(!db.open()){
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }

    QSqlQuery query(db);
    query.prepare("SELECT message FROM `messages` WHERE timestamp > :timestamp AND (receiver=:receiver OR receiver=:transmitter) AND (transmitter=:transmitter OR transmitter=:receiver) ORDER BY timestamp ASC");
    query.bindValue(":transmitter", CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter()));
    query.bindValue(":receiver", CryptoHelper::saveStringQByteConversion(jsonObject.getReceivers()[0]));
    query.bindValue(":timestamp", jsonObject.getTimestamp());

    std::vector<std::string> responseLines;
    if(query.exec()){
        qDebug() << getLastExecutedQuery(query);
        while( query.next() ){
            if(CryptoHelper::isChatyDataFile(CryptoHelper::saveStringConversion(query.value(0).toString()))){
                QByteArray data;
                bool check = FileIOHelper::readDataFromFile(CryptoHelper::saveStringConversion(query.value(0).toString()), data);
                if(check){
                   responseLines.push_back(CryptoHelper::decodeBase64(data));
                }
            } else {
                responseLines.push_back(CryptoHelper::decodeBase64(query.value(0).toByteArray()));
            }
        }
        if(responseLines.empty()){
            responseJson.setStatusCode(NO_CONTENT);
        } else {
            responseJson.setStatusCode(OK);
        }
        MultiLinesResponseJsonWrapper lines(responseLines);
        responseJson.setMessage(lines.getJsonString());

        return responseJson;
    } else {
        qDebug() << query.lastError();
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }
}

ResponseJsonWrapper MysqlWrapper::getRsaPublicKeyByUsername(std::string username)
{
    ResponseJsonWrapper responseJson;
    QSqlQuery query(db);
    query.prepare("SELECT rsaPublicKey FROM `users` WHERE username = :username LIMIT 1");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(CryptoHelper::parseUsernameOutOfUsername(username)));

    if(query.exec()){
        if(query.next()){
            std::string rsaPublicKey = CryptoHelper::saveStringConversion(query.value(0).toString().trimmed());
            if(rsaPublicKey.compare("") != 0){
                responseJson.setStatusCode(OK);
                responseJson.setMessage(rsaPublicKey);
            } else {
                responseJson.setStatusCode(NO_CONTENT);
            }
        } else {
            responseJson.setStatusCode(NO_CONTENT);
        }
    } else {
        qDebug() << query.lastError();
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
    }
    return responseJson;
}

ResponseJsonWrapper MysqlWrapper::getRsaPublicKeyForUser(RequestJsonWrapper &jsonObject)
{
    qDebug() << "User: "
             << CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter())
             << " requires RSA PUBLIC KEY from user: "
             << CryptoHelper::saveStringQByteConversion(jsonObject.getReceivers()[0]);
    ResponseJsonWrapper responseJson;
    if(!db.open()){
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }

    if(jsonObject.getReceivers().size() != 1){
        qDebug() << "No Receivers!";
        responseJson.setStatusCode(BAD_REQUEST);
        return responseJson;
    }

    std::string receiver = jsonObject.getReceivers()[0];

    return getRsaPublicKeyByUsername(receiver);;
}

ResponseJsonWrapper MysqlWrapper::setRsaPublicKeyForUser(RequestJsonWrapper &jsonObject)
{
    qDebug() << "SET RSA KEY for user: "
             << CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter());
    ResponseJsonWrapper responseJson;

    ResponseJsonWrapper oldRsaKey = getRsaPublicKeyByUsername(jsonObject.getTransmitter());
    if(oldRsaKey.getStatusCode() != OK && oldRsaKey.getStatusCode() != NO_CONTENT){
        qDebug() << "INTERNAL SERVER ERROR";
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }

    std::string savedRsaPubKey = oldRsaKey.getMessage();
    if(savedRsaPubKey.compare("") != 0){
        responseJson.setStatusCode(METHOD_NOT_ALLOWED);
        return responseJson;
    }

    if(!db.open()){
        qDebug() << "INTERNAL SERVER ERROR";
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, rsaPublicKey) VALUES (:username, :rsaPublicKey) "
                  "ON DUPLICATE KEY UPDATE rsaPublicKey = :rsaPublicKey;");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(CryptoHelper::parseUsernameOutOfUsername(jsonObject.getTransmitter())));
    query.bindValue(":rsaPublicKey", CryptoHelper::saveStringQByteConversion(jsonObject.getMessage()));

    qDebug() << getLastExecutedQuery(query);
    if(query.exec()){
        responseJson.setStatusCode(OK);
    } else {
        qDebug() << query.lastError();
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
    }
    return responseJson;
}

ResponseJsonWrapper MysqlWrapper::getIpAddressesForUsers(RequestJsonWrapper &jsonObject)
{
    qDebug() << "User: "
             << CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter())
             << " requires IP Adress for user: ";
    for(unsigned int i = 0; i < jsonObject.getReceivers().size(); i++){
        qDebug() << CryptoHelper::saveStringQByteConversion(jsonObject.getReceivers()[i]);
    }
    ResponseJsonWrapper responseJson;
    if(!db.open()){
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }

    unsigned int numberOfReceivers = jsonObject.getReceivers().size();

    if(numberOfReceivers == 0){
        qDebug() << "No Receivers!";
        responseJson.setStatusCode(BAD_REQUEST);
        return responseJson;
    }

    std::vector<std::string> responseLines;
    for(unsigned int i = 0; i < numberOfReceivers; i++){
        std::string currentReceiver = jsonObject.getReceivers()[i];
        std::vector<std::string> resultForReceiver = getIpAddressesForUser(currentReceiver);
        IpRefreshJsonWrapper currentReceiverIpObject(currentReceiver, resultForReceiver);
        responseLines.push_back(currentReceiverIpObject.getJsonString());
    }

    if(responseLines.empty()){
        responseJson.setStatusCode(NO_CONTENT);
    } else {
        MultiLinesResponseJsonWrapper lines(responseLines);
        responseJson.setMessage(lines.getJsonString());
        responseJson.setStatusCode(OK);
    }

    return responseJson;
}

ResponseJsonWrapper MysqlWrapper::getPwSaltForUser(RequestJsonWrapper &jsonObject)
{
    qDebug() << "User: "
             << CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter())
             << " requires password salt";
    ResponseJsonWrapper responseJson;
    if(!db.open()){
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }

    QSqlQuery query(db);
    query.prepare("SELECT password FROM `users` WHERE username = :username LIMIT 1");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(CryptoHelper::parseUsernameOutOfUsername(jsonObject.getTransmitter())));

    if(query.exec()){
        if(query.next()){
            responseJson.setMessage(CryptoHelper::saveStringQByteConversion(query.value(0).toByteArray()).substr(0, 25));
        }
        if(responseJson.getMessage().compare("") == 0){
            responseJson.setStatusCode(NO_CONTENT);
        } else {
            responseJson.setStatusCode(OK);
        }
        return responseJson;
    } else {
        qDebug() << query.lastError();
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }

}

ResponseJsonWrapper MysqlWrapper::checkLoginCredentials(RequestJsonWrapper &jsonObject, std::string ipAddress, std::string serverRsaPrivateKey)
{
    qDebug() << "User: "
             << CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter())
             << " tries to login";
    ResponseJsonWrapper responseJson;
    std::string username = jsonObject.getTransmitter();
    LoginMessageJsonWrapper loginObject(jsonObject.getMessage());
    std::string password;
    int result = RSAWrapper::privateDecrypt(loginObject.getHashedPassword(), password, serverRsaPrivateKey);
    if(result == -1){
        responseJson.setStatusCode(FORBIDDEN);
        return responseJson;
    }
    std::string oldDeviceIdentifier = loginObject.getOldDeviceInformation();
    std::string rsaPubKeyForClient = loginObject.getMyRsaPubKey();
    ResponseJsonWrapper oldKey = getRsaPublicKeyByUsername(jsonObject.getTransmitter());
    std::string savedRsaPubKey;
    if(oldKey.getStatusCode() == OK){
        savedRsaPubKey = oldKey.getMessage();
    }
    if(!db.open()){
        responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        return responseJson;
    }
    if(oldDeviceIdentifier.compare("") != 0){
        bool oldDeviceIdentifierValid = isOldDeviceIdentifierValid(oldDeviceIdentifier);
        if(!oldDeviceIdentifierValid){
            responseJson.setStatusCode(FORBIDDEN);
            return responseJson;
        }
    }
    bool credentialsCorrect = areLoginCredentialsCorrect(username, password);
    if(credentialsCorrect){
        responseJson.setStatusCode(OK);
        std::string newDeviceIdentifier = CryptoHelper::generateSha256(CryptoHelper::generateRandomKey(32).append(CryptoHelper::saveStringQByteConversion(QUuid::createUuid().toByteArray())));
        QDateTime lastIpUpdate = QDateTime::currentDateTime();
        bool insertOrUpdateDeviceIdentifier;
        if(oldDeviceIdentifier.compare("") == 0){
            insertOrUpdateDeviceIdentifier = insertDeviceInformation(jsonObject, newDeviceIdentifier, ipAddress, lastIpUpdate);
        } else {
            insertOrUpdateDeviceIdentifier = updateDeviceInformation(jsonObject, oldDeviceIdentifier, newDeviceIdentifier, ipAddress, lastIpUpdate);
        }
        if(insertOrUpdateDeviceIdentifier){
            if(rsaPubKeyForClient.find(savedRsaPubKey) == std::string::npos){
                responseJson.setStatusCode(ACCEPTED);
            }
            std::string encryptedDeviceIdentifier;
            if(rsaPubKeyForClient.compare("") != 0){
                int publicEncryption = RSAWrapper::publicEncrypt(newDeviceIdentifier, encryptedDeviceIdentifier, rsaPubKeyForClient);
                if(publicEncryption == -1) {
                    responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
                }
            } else {
                encryptedDeviceIdentifier = newDeviceIdentifier;
            }
            responseJson.setMessage(encryptedDeviceIdentifier);
        } else {
            responseJson.setStatusCode(INTERNAL_SERVER_ERROR);
        }
        return responseJson;
    } else {
        responseJson.setStatusCode(UNAUTHORIZED);
        return responseJson;
    }
}

int MysqlWrapper::isDeviceIdentifierValid(RequestJsonWrapper &jsonObject)
{
    if(!db.open()){
        return INTERNAL_SERVER_ERROR;
    }
    QSqlQuery query(db);

    QDateTime arriveTime = QDateTime::currentDateTime();

    QDateTime lastIpUpdate = QDateTime::currentDateTime();

    query.prepare("SELECT tokenExpirationDate, deviceIdentifier FROM `registered_devices` "
                  "WHERE  username = :username;");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter()));
    if(query.exec()){
        QDateTime tokenExpirationDate;
        std::string hashedDeviceIdentifier;
        while( query.next() ){
            hashedDeviceIdentifier = CryptoHelper::calculateAbstractDeviceIdentifier(jsonObject.getTransmitter(), jsonObject.getTimestamp(), jsonObject.getMessage(), CryptoHelper::saveStringConversion(query.value("deviceIdentifier").toString()));
            if(hashedDeviceIdentifier.compare(jsonObject.getDeviceIdentifier()) == 0){
                qDebug() << "Token found";
                tokenExpirationDate = query.value("tokenExpirationDate").toDateTime();
                if(tokenExpirationDate < arriveTime){
                    return UNAUTHORIZED;
                } else {
                    return OK;
                }
            }
        }
        qDebug() << "No deviceIdentifier matched: " << jsonObject.getDeviceIdentifier().c_str();
        return UNAUTHORIZED;
    }
    return INTERNAL_SERVER_ERROR;
}

bool MysqlWrapper::isTransmitterTrustedServer(std::string address)
{
    if(!db.open()){
        return false;
    }
    QSqlQuery query(db);
    query.prepare("SELECT count(*) FROM known_servers WHERE serverAddress = :address");
    query.bindValue(":address", CryptoHelper::saveStringQByteConversion(address));

    if(query.exec()){
        if(query.next()){
            return query.value(0).toInt() > 0;
        }
        return false;
    } else {
        qDebug() << query.lastError();
        return false;
    }
}

int MysqlWrapper::loadIntDBConfig(std::string key)
{
    int result = -1;
    if(!db.open()){
        return result;
    }
    QSqlQuery query(db);
    query.prepare("SHOW VARIABLES LIKE :key;");
    query.bindValue(":key", CryptoHelper::saveStringConversion(key));
    if(query.exec()){
        if(query.next()){
            return query.value("Value").toInt();
        }
    } else {
        return result;
    }
    return result;
}

std::vector<std::string> MysqlWrapper::getIpAddressesForUser(std::string username)
{
    std::vector<std::string> result;
    if(!db.open()){
        return result;
    }
    QSqlQuery query(db);
    query.prepare("SELECT DISTINCT(ipAddress) FROM registered_devices WHERE username = :username");
    query.bindValue(":username", CryptoHelper::saveStringConversion(username));
    if(query.exec()){
        while( query.next() ){
            result.push_back(CryptoHelper::saveStringConversion(query.value("ipAddress").toString()));
        }
    } else {
        qDebug() << "getipAddressesForUser: " << query.lastError();
    }
    return result;
}

bool MysqlWrapper::areLoginCredentialsCorrect(std::string username, std::string password)
{
    if(!db.open()){
        return false;
    }
    QSqlQuery query(db);
    query.prepare("SELECT count(*) FROM `users` WHERE username = :username AND password = :password");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(CryptoHelper::parseUsernameOutOfUsername(username)));
    query.bindValue(":password", CryptoHelper::saveStringQByteConversion(password));

    if(query.exec()){
        if(query.next()){
            return query.value(0).toInt() > 0;
        }
        return false;
    } else {
        qDebug() << query.lastError();
        return false;
    }
}

bool MysqlWrapper::isOldDeviceIdentifierValid(std::string oldDeviceIdentifier)
{
    if(!db.open()){
        return false;
    }
    QSqlQuery query(db);
    query.prepare("SELECT count(*) FROM `registered_devices` WHERE deviceIdentifier = :deviceIdentifier");
    query.bindValue(":deviceIdentifier", CryptoHelper::saveStringQByteConversion(oldDeviceIdentifier));

    if(query.exec()){
        if(query.next()){
            return query.value(0).toInt() > 0;
        }
        return false;
    } else {
        qDebug() << query.lastError();
        return false;
    }
}

QString MysqlWrapper::getLastExecutedQuery(const QSqlQuery &query)
{
    QString str = query.lastQuery();
    QMapIterator<QString, QVariant> it(query.boundValues());
    while (it.hasNext()){
        it.next();
        str.replace(it.key(),it.value().toString());
    }
    return str;
}

bool MysqlWrapper::insertDeviceInformation(RequestJsonWrapper &jsonObject, std::string deviceIdentifier, std::string ipAddress, QDateTime lastIpUpdate)
{
    QSqlQuery query(db);
    QDateTime tokenExiprationDate = QDateTime::currentDateTime().addSecs(3600);
    query.prepare("INSERT INTO registered_devices (deviceIdentifier, username, ipAddress, lastHeartbeatDate, ipAddressChangedDate, tokenExpirationDate) VALUES (:uuid, :username, :ipAddress, :lastHeartbeatDate, :ipAddressChangedDate, :tokenExpirationDate) "
                  "ON DUPLICATE KEY UPDATE username = :username, ipAddress = :ipAddress, lastHeartbeatDate = :lastHeartbeatDate, ipAddressChangedDate = :ipAddressChangedDate, tokenExpirationDate = :tokenExpirationDate;");
    query.bindValue(":uuid", CryptoHelper::saveStringQByteConversion(deviceIdentifier));
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter()));
    query.bindValue(":ipAddress", CryptoHelper::saveStringQByteConversion(ipAddress));
    query.bindValue(":lastHeartbeatDate", QDateTime::currentDateTime());
    query.bindValue(":ipAddressChangedDate", lastIpUpdate);
    query.bindValue(":tokenExpirationDate", tokenExiprationDate);
    if(query.exec()){
        return true;
    } else {
        qDebug() << "ERROR: " << query.lastError() <<  getLastExecutedQuery(query);
        return false;
    }
}

bool MysqlWrapper::updateDeviceInformation(RequestJsonWrapper &jsonObject, std::string oldDeviceIdentifier, std::string newDeviceIdentifier, std::string ipAddress, QDateTime lastIpUpdate)
{
    QSqlQuery query(db);
    QDateTime tokenExiprationDate = QDateTime::currentDateTime().addSecs(3600);
    query.prepare("UPDATE registered_devices "
                  "SET deviceIdentifier = :newDeviceIdentifier, username = :username, ipAddress = :ipAddress, lastHeartbeatDate = :lastHeartbeatDate, ipAddressChangedDate = :ipAddressChangedDate, tokenExpirationDate = :tokenExpirationDate "
                  "WHERE deviceIdentifier = :oldDeviceIdentifier;");
    query.bindValue(":newDeviceIdentifier", CryptoHelper::saveStringQByteConversion(newDeviceIdentifier));
    query.bindValue(":oldDeviceIdentifier", CryptoHelper::saveStringQByteConversion(oldDeviceIdentifier));
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter()));
    query.bindValue(":ipAddress", CryptoHelper::saveStringQByteConversion(ipAddress));
    query.bindValue(":lastHeartbeatDate", QDateTime::currentDateTime());
    query.bindValue(":ipAddressChangedDate", lastIpUpdate);
    query.bindValue(":tokenExpirationDate", tokenExiprationDate);
    if(query.exec()){
        qDebug() << getLastExecutedQuery(query);
        return true;
    } else {
        qDebug() << "ERROR: " << query.lastError() <<  getLastExecutedQuery(query);
        return false;
    }
}

std::string MysqlWrapper::getLastDeviceIdentifier(RequestJsonWrapper &jsonObject, std::string &ipAddress){
    QSqlQuery query(db);
    query.prepare("SELECT deviceIdentifier FROM registered_devices WHERE ipAddress = :ipAddress AND username = :username;");
    query.bindValue(":ipAddress", CryptoHelper::saveStringQByteConversion(ipAddress));
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(jsonObject.getTransmitter()));
    if(query.exec() && query.next()){
        return CryptoHelper::saveStringQByteConversion(query.value(0).toByteArray());
    } else {
        return "";
    }
}
