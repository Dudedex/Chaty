#include "server_setup.h"

int MysqlWrapper::maxAllowedPacket = -1;

ServerSetup::ServerSetup()
{
    QString folder = CryptoHelper::saveStringConversion(MESSAGE_MEDIA_FOLDER);
    if(!QDir(folder).exists()){
        qDebug() << "try creating directory: " << folder;
        if(!QDir().mkdir(folder)){
            qDebug() << "Could not create Server Media Folder";
            return;
        }
    }

    MysqlWrapper* loadDbConfig = new MysqlWrapper();
    MysqlWrapper::maxAllowedPacket = loadDbConfig->loadIntDBConfig("max_allowed_packet");;
    delete loadDbConfig;
    if(MysqlWrapper::maxAllowedPacket  == -1){
        qDebug() << "Could not load DB Config... closing now!";
        return;
    } else {
        qDebug() << "MaxAllowedPackage Size: " << MysqlWrapper::maxAllowedPacket;
    }

    int code = 0;
    bool close = false;
    do{
        myRsaKeysHandler = new MyRsaKeysHandler(true);
        code = myRsaKeysHandler->init();
        if(code != RSA_NO_ERROR){
            qDebug() << "Loading RSA Key failed with Code: " << code;
            if(code % RSA_PRIVATE_KEY_NOT_FOUND == 0){
                qDebug() << "Private Key not found!";
            }
            if(code % RSA_PUBLIC_KEY_NOT_FOUND == 0){
                qDebug() << "Public Key not found!";
            }
            if(code % RSA_FOLDER_NOT_FOUND == 0){
                close = true;
                qDebug() << "SERVER .ssh Folder not found!";
            }
            if(code % RSA_KEYS_CORRUPT == 0){
                qDebug() << "SERVER RSA Keys corrupt can't init Keys";
            }
            if(close){
                qDebug() << "SERVER RSA Keys could not be loaded.. Stopping server!";
                return;
            } else {
                bool checked = false;
                do{
                    qDebug() << "Should Keys get created? (y/n)";
                    std::string str;
                    std::getline(std::cin, str);
                    if(str.compare("y") == 0 || str.compare("Y") == 0){
                        qDebug() << "calculating keys ...";
                        std::string publicKey;
                        std::string privateKey;
                        bool isKeysCreated = RSAWrapper::generateRsaKeysPair(publicKey, privateKey, 4096);
                        if(isKeysCreated){
                            qDebug() << "Keys Created";
                            bool isKeysSaved = RSAWrapper::saveRSAKeysToFiles(publicKey, privateKey, true);
                            if(isKeysSaved){
                                checked = true;
                                qDebug() << "Keys saved";
                            } else {
                                qDebug() << "Keys could not get stored";
                            }
                        } else {
                            qDebug() << "Keys could NOT get created";
                        }
                    } else if(str.compare("n") == 0 || str.compare("N") == 0){
                        checked = true;
                        close = true;
                        qDebug() << "CLOSING SERVER!";
                    }
                }while(!checked);
            }
        }
    } while (code != RSA_NO_ERROR && !close);

    if(code != RSA_NO_ERROR){
        qDebug() << "Closing server with no further actions.";
        return;
    }

    server.setMyRsaKeysHandler(this->myRsaKeysHandler);

    if (!server.listen(QHostAddress::Any, SERVER_PORT)) {
        qDebug()<< "Unable to start Server " << server.errorString();
        return;
    }

    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    qDebug() << "The server is reachable on:";
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).toIPv4Address()) {
            qDebug() << "  - " << ipAddressesList.at(i).toString();
        }
    }
    qDebug() << "Port: "<< server.serverPort();
    initialisationComplete = true;
}

ServerSetup::~ServerSetup()
{
    qDebug() << "ServerSetup Destructor called";
}

MyRsaKeysHandler *ServerSetup::getMyRsaKeysHandler()
{
    return myRsaKeysHandler;
}

bool ServerSetup::isInitialized() const
{
    return initialisationComplete;
}
