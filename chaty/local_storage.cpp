#include "local_storage.h"
#include <QMessageBox>

LocalStorage::LocalStorage(std::string connectionName)
{
    db = QSqlDatabase::addDatabase("QSQLITE",
            CryptoHelper::saveStringConversion(connectionName));
    db.setDatabaseName(DATABASE_NAME);
    //bla

    if (!db.open()) {
        status = false;
        return;
    }
    bool tempStatus = true;
    QSqlQuery query(db);

    bool createContactsTable = query.exec("CREATE TABLE IF NOT EXISTS \"contacts\" "
                                       "(\"username\" TEXT UNIQUE, \"alias\" "
                                       "TEXT UNIQUE, \"rsaPublicKey\" TEXT);");
    tempStatus = tempStatus && createContactsTable;
    if(!createContactsTable){
        qDebug() << query.lastError();
    }

    bool createMessagesTable =query.exec("CREATE TABLE IF NOT EXISTS \"messages\" ( \"sha256\" TEXT PRIMARY KEY NOT NULL UNIQUE"
                                         ", \"transmitter\" TEXT, \"receiver\" TEXT, \"message\" TEXT, \"timestamp\" DATETIME);");
    tempStatus = tempStatus && createMessagesTable;
    if(!createMessagesTable){
        qDebug() << query.lastError();
    }

    bool createDeviceInformationTable = query.exec("CREATE TABLE IF NOT EXISTS \"device_informations\" (\"key\" TEXT PRIMARY KEY  NOT NULL  UNIQUE, \"value\" TEXT);");
    tempStatus = tempStatus && createDeviceInformationTable;
    if(!createDeviceInformationTable){
        qDebug() << query.lastError();
    }

    bool createIpAddressTable = query.exec("CREATE TABLE IF NOT EXISTS \"ip_addresses\" (\"username\" TEXT NOT NULL , \"ipAddress\" VARCHAR PRIMARY KEY  NOT NULL  UNIQUE )");
    tempStatus = tempStatus && createIpAddressTable;
    if(!createIpAddressTable){
        qDebug() << query.lastError();
    }

    bool createLastMessagePullTable = query.exec("CREATE TABLE IF NOT EXISTS \"last_message_pull\" (\"username\" TEXT PRIMARY KEY NOT NULL UNIQUE, \"timestamp\" DATETIME)");
    tempStatus = tempStatus && createLastMessagePullTable;
    if(!createLastMessagePullTable){
        qDebug() << query.lastError();
    }

    status = tempStatus;
}

bool LocalStorage::getStatus() const
{
    return status;
}

bool LocalStorage::insertMessagesInDB(ResponseJsonWrapper response)
{
    MultiLinesResponseJsonWrapper lines(response.getMessage());
    std::vector<std::string> responseLines = lines.getLines();
    bool check = true;
    db.transaction();
    for(unsigned int i = 0; i < responseLines.size(); i++){
        QSqlQuery query(db);
        query.prepare("INSERT OR IGNORE INTO messages(sha256, transmitter, receiver, message, timestamp) "
                      "VALUES(:sha256, :transmitter,:receiver,:message, :timestamp)");
        RequestJsonWrapper currentLine(responseLines[i]);
        query.bindValue(":sha256", CryptoHelper::saveStringQByteConversion(CryptoHelper::generateSha256(currentLine.getJsonString())));
        query.bindValue(":transmitter", CryptoHelper::saveStringQByteConversion(currentLine.getTransmitter()));
        query.bindValue(":receiver", CryptoHelper::saveStringQByteConversion(currentLine.getReceivers()[0]));
        query.bindValue(":message", CryptoHelper::saveStringQByteConversion(currentLine.getJsonString()));
        query.bindValue(":timestamp", currentLine.getTimestamp());
        if(!query.exec()){
            qDebug() << "insertReceivedMessages: " << query.lastError();
            /*QMessageBox msgBox;
            msgBox.setText(query.lastError().text());
            msgBox.exec();*/
            check = false;
        }
        query.clear();
    }
    if(check){
        db.commit();
    } else {
        qDebug() << "rolled back transaction in insert Messages";
        db.rollback();
    }
    return check;
}

bool LocalStorage::insertMessagesInDB(RequestJsonWrapper message)
{
    std::vector<std::string> messages;
    messages.push_back(message.getJsonString());
    MultiLinesResponseJsonWrapper multiLine(messages);
    ResponseJsonWrapper mySqlDummy;
    mySqlDummy.setMessage(multiLine.getJsonString());
    mySqlDummy.setStatusCode(OK);
    return insertMessagesInDB(mySqlDummy);
}

std::string LocalStorage::getConfigurationKey(std::string key)
{
    QSqlQuery query(db);
    query.prepare("SELECT value FROM device_informations WHERE key=:key");
    query.bindValue(":key", CryptoHelper::saveStringQByteConversion(key));
    if(query.exec() && query.next()){
        return CryptoHelper::saveStringQByteConversion(query.value(0).toByteArray());
    } else {
        return "";
    }
}

bool LocalStorage::setConfigurationKey(std::string key, std::string value)
{
    QSqlQuery query(db);
    db.transaction();
    query.prepare("INSERT OR REPLACE INTO device_informations VALUES (:key,:value);");
    query.bindValue(":key", CryptoHelper::saveStringQByteConversion(key));
    query.bindValue(":value", CryptoHelper::saveStringQByteConversion(value));
    if(query.exec()){
        db.commit();
        return true;
    } else {
        db.rollback();
        return false;
    }
}

std::vector<std::string> LocalStorage::getMessagesForChat(std::string user, std::string chatReceiver)
{
    QSqlQuery query(db);
    query.prepare("SELECT message FROM messages WHERE (LOWER(transmitter)=LOWER(:chatReceiver) OR LOWER(transmitter)=LOWER(:user)) AND (LOWER(receiver)=LOWER(:chatReceiver) OR LOWER(receiver)=LOWER(:user)) ORDER BY timestamp");
    query.bindValue(":user", CryptoHelper::saveStringQByteConversion(user));
    query.bindValue(":chatReceiver", CryptoHelper::saveStringQByteConversion(chatReceiver));
    std::vector<std::string> result;
    if(query.exec()){
        while( query.next() ){
            RequestJsonWrapper currentLine(CryptoHelper::saveStringQByteConversion(query.value(0).toByteArray()));
            result.push_back( CryptoHelper::saveStringQByteConversion(query.value(0).toByteArray()));
        }
    } else {
        qDebug() << "getMessagesForChat: " << query.lastError();
    }
    return result;
}

QDateTime LocalStorage::getLatestsTimeStampForChat(std::string chatReceiver)
{
    QSqlQuery query(db);
    query.prepare("SELECT timestamp FROM last_message_pull WHERE username = :chatReceiver");
    query.bindValue(":chatReceiver", CryptoHelper::saveStringQByteConversion(chatReceiver));
    QDateTime latestTimeStamp;
    latestTimeStamp.setTime_t(0);
    if(query.exec()){
        if(query.next()){
            qDebug() << "GetLatestsTimeStamp" << query.value(0).toDateTime();
            return query.value(0).toDateTime();
        } else {
            qDebug() << "GetLatestsTimeStamp" << latestTimeStamp;
            return latestTimeStamp;
        }
    }
    qDebug() << "GetLatestsTimeStamp Error" << query.lastError();
    return latestTimeStamp;
}

bool LocalStorage::setLatestsTimeStampForChat(QDateTime dateTime, std::string chatReceiver)
{
    QSqlQuery query(db);
    query.prepare("INSERT OR REPLACE INTO last_message_pull VALUES (:chatReceiver, :timestamp);");
    query.bindValue(":chatReceiver", CryptoHelper::saveStringQByteConversion(chatReceiver));
    query.bindValue(":timestamp", dateTime);
    qDebug() << "Latest Message Timestamp for " << chatReceiver.c_str() << " => " << dateTime;
    if(query.exec()){
        return true;
    }
    qDebug() << "SetLatestsTimeStamp Error: " << query.lastError();
    return false;
}

std::vector<Contact> LocalStorage::getKnownContacts()
{
    QSqlQuery query(db);
    query.prepare("SELECT username, alias, rsaPublicKey FROM contacts");
    std::vector<Contact> result;
    if(query.exec()){
        while( query.next() ){
            Contact currentReceiver;
            currentReceiver.setUsername(CryptoHelper::saveStringQByteConversion(query.value("username").toByteArray()));
            currentReceiver.setAlias(CryptoHelper::saveStringQByteConversion(query.value("alias").toByteArray()));
            currentReceiver.setRsaPublicKey(CryptoHelper::saveStringQByteConversion(query.value("rsaPublicKey").toByteArray()));
            result.push_back(currentReceiver);
        }
    } else {
        qDebug() << "getMessagesForChat: " << query.lastError();
    }
    return result;
}

Contact LocalStorage::getContactByUsername(std::string username)
{
    QSqlQuery query(db);
    query.prepare("SELECT username, alias, rsaPublicKey FROM contacts WHERE username = :username;");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(username));
    Contact result;
    if(query.exec()){
        while( query.next() ){
            result.setUsername(CryptoHelper::saveStringQByteConversion(query.value("username").toByteArray()));
            result.setAlias(CryptoHelper::saveStringQByteConversion(query.value("alias").toByteArray()));
            result.setRsaPublicKey(CryptoHelper::saveStringQByteConversion(query.value("rsaPublicKey").toByteArray()));
        }
    } else {
        qDebug() << "getUserByUsername: " << query.lastError();
    }
    return result;
}

bool LocalStorage::addNewContact(std::string username, std::string alias, std::string rsaPublicKey)
{
    bool check = true;
    QSqlQuery query(db);
    if(alias.compare("") == 0){
        alias = CryptoHelper::parseUsernameOutOfUsername(username);
    }
    db.transaction();
    query.prepare("INSERT INTO contacts (username, alias, rsaPublicKey) values(:username, :alias, :rsaPublicKey)");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(username));
    query.bindValue(":alias", CryptoHelper::saveStringQByteConversion(alias));
    query.bindValue(":rsaPublicKey", CryptoHelper::saveStringQByteConversion(rsaPublicKey));
    if(!query.exec()){
        QMessageBox msgBox;
        msgBox.setText(query.lastError().text());
        msgBox.exec();
        check = false;
    }
    if(check){
        db.commit();
    } else {
        db.rollback();
        qDebug() << getLastExecutedQuery(query);
    }   
    return check;
}

bool LocalStorage::refreshIpAddressesByUsername(IpRefreshJsonWrapper ipObject)
{
    QSqlQuery query(db);
    std::string user = ipObject.getUser();
    db.transaction();
    QString deleteQuery("DELETE FROM ip_addresses WHERE username = \"" +
                        CryptoHelper::saveStringConversion(user) + "\";");
    query.prepare(deleteQuery);
    if(!query.exec()){
        qDebug() << "IP ERROR: " << query.lastError().text();
        return false;
    }
    query.clear();

    QString queryString("INSERT INTO ip_addresses (username, ipAddress) VALUES ");
    unsigned int numberOfipAddresses = ipObject.getIpAddressesAsJsonArrayForUser().size();
    if(numberOfipAddresses > 0){
        for(unsigned int i = 0; i < numberOfipAddresses; i++){
            queryString.append(QString(" (\"") +
                               CryptoHelper::saveStringConversion(user) +
                               QString("\", \"") +
                               CryptoHelper::saveStringConversion(ipObject.getIpAddressesAsJsonArrayForUser()[i]) +
                               "\")");
            if(i != (numberOfipAddresses - 1)){
                queryString.append(",");
            } else {
                queryString.append(";");
            }
        }
        query.prepare(queryString);
        if(!query.exec()){
            qDebug() << "IP ERROR 2: " << query.lastError().text();
            db.rollback();
            qDebug() << "Rollback ipRefresh";
            return false;
        } else {
            db.commit();
            qDebug() << "Commited ipRefresh";
            return true;
        }
    }
    return true;
}

std::vector<std::string> LocalStorage::getIpAddressesByUsername(std::string username)
{
    QSqlQuery query(db);
    query.prepare("SELECT ipAddress FROM ip_addresses WHERE username LIKE :username");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(CryptoHelper::parseUsernameOutOfUsername(username) + "@%"));
    std::vector<std::string> result;
    if(query.exec()){
        qDebug() << "IPADDRESS!!!! " << getLastExecutedQuery(query);
        while( query.next() ){
            result.push_back(CryptoHelper::saveStringConversion(query.value("ipAddress").toString()));
        }
    } else {
        qDebug() << getLastExecutedQuery(query);
        qDebug() << "getIpAddressesByUsername: " << query.lastError();
    }
    return result;
}

bool LocalStorage::deleteContactByUsername(std::string username)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM contacts WHERE username = :username");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(username));
    if(query.exec()){
        return true;
    } else {
        qDebug() << query.lastError();
        return false;
    }
}

bool LocalStorage::updateContactAliasByUsername(std::string username, std::string alias)
{
    QSqlQuery query(db);
    query.prepare("UPDATE contacts SET alias = :alias WHERE username = :username");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(username));
    query.bindValue(":alias", CryptoHelper::saveStringQByteConversion(alias));
    if(query.exec()){
        return true;
    } else {
        qDebug() << query.lastError();
        return false;
    }
}

bool LocalStorage::updateContactRSAKeyByUsername(std::string username, std::string rsaKey)
{
    QSqlQuery query(db);
    query.prepare("UPDATE contacts SET rsaPublicKey = :rsaPublicKey WHERE username = :username");
    query.bindValue(":username", CryptoHelper::saveStringQByteConversion(username));
    query.bindValue(":rsaPublicKey", CryptoHelper::saveStringQByteConversion(rsaKey));
    if(query.exec()){
        return true;
    } else {
        qDebug() << query.lastError();
        return false;
    }
}

QString LocalStorage::getLastExecutedQuery(const QSqlQuery &query)
{
    QString str = query.lastQuery();
    QMapIterator<QString, QVariant> it(query.boundValues());
    while (it.hasNext()){
        it.next();
        str.replace(it.key(),it.value().toString());
    }
    return str;
}

