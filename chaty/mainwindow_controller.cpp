#include "mainwindow_controller.h"
#include "imageprocessor.h"

MainwindowController::MainwindowController(CurrentConfiguration *currentConfiguration)
{
    this->localStorage = new LocalStorage("mainThreadConnection");
    this->backendStatus = localStorage->getStatus();
    if(!backendStatus){
        QMessageBox msgBox;
        msgBox.setText("Could not initialize Database. Database corrupted or not accessible!");
        msgBox.show();
        return;
    }
    this->configuration = currentConfiguration;
    this->configuration->setController(this);
    triesToLogin = false;
    loadConfiguration();
    initializeRSAKeys();

    bool loggedIn = false;
    loggedIn = loginToServer();

    if(!loggedIn){
        LoginDialog login(this, 0);
        login.exec();
        this->backendStatus = this->backendStatus && login.isLoggedIn();
        if(!backendStatus){
            return;
        }
    } else {
        if(configuration->getCurrentServerRSAPublicKey().compare("") == 0){
            QDateTime date;
            date.setTime_t(0);
            RequestJsonWrapper request(configuration->getCurrentUsernameAtDomainPair(), "", date, "" ,GET_RSA_KEY_FOR_SERVER);
            TcpClient communication(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()));
            communication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(request.getJsonString()));
            ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(communication.getResponse()));
            if(response.getStatusCode() == OK){
                qDebug() << "Saving Rsa Pub Key from Server";
                configuration->setServerRSAPublicKey(response.getMessage());
            }
        }
    }

    clientServerSetup = new ClientServerSetup();
    connect(clientServerSetup,
                     SIGNAL(messageReceived(QString)),
                     this,
                     SLOT(onMessageReceived(QString)));

    backgroundScheduler = new BackgroundScheduler(currentConfiguration);
    backgroundScheduler->setLoggedIn(true);
    updateSchedulerIntervalls();
    connect(backgroundScheduler, SIGNAL(messagesAvailable(QString)), this,
                              SLOT(onMessagesAvailable(QString)));
    connect(backgroundScheduler, SIGNAL(authenticationRequired()), this,
                              SLOT(onAuthenticationRequired()));
    connect(backgroundScheduler, SIGNAL(loggedOut()), this,
                              SLOT(onLoggedOut()));
    connect(backgroundScheduler, SIGNAL(refreshIpAdresses(QString)), this,
                               SLOT(onRefreshIpAdresses(QString)));
    connect(backgroundScheduler, SIGNAL(pullMessagesByUsername(QString)), this,
                              SLOT(onPullMessagesByUsername(QString)));
    backgroundScheduler->start();
}

std::string MainwindowController::getOsName()
{
    //https://wiki.qt.io/Get-OS-name-in-Qt
    #if defined(Q_OS_ANDROID)
    return std::string("android");
    #elif defined(Q_OS_BLACKBERRY)
    return std::string("blackberry");
    #elif defined(Q_OS_IOS)
    return std::string("ios");
    #elif defined(Q_OS_MAC)
    return std::string("macos");
    #elif defined(Q_OS_WINCE)
    return std::string("wince");
    #elif defined(Q_OS_WIN)
    return std::string("windows");
    #elif defined(Q_OS_LINUX)
    return std::string("linux");
    #elif defined(Q_OS_UNIX)
    return std::string("unix");
    #else
    return std::string("unknown");
    #endif
}

bool MainwindowController::shouldWindowFrameBeHidden()
{
    return getOsName().compare("windows") != 0 &&
           getOsName().compare("macos") != 0 &&
           getOsName().compare("unix") != 0 &&
           getOsName().compare("linux") != 0;
}

MainwindowController::~MainwindowController()
{
    if(clientServerSetup != 0){
        delete clientServerSetup;
    }
    if(backgroundScheduler != 0){
        backgroundScheduler->stopScheduler();
        backgroundScheduler->wait();
        delete backgroundScheduler;
    }
    if(localStorage != 0){
        delete localStorage;
    }

}

void MainwindowController::initializeRSAKeys()
{
    //Memory is released in the Destructor of currentConfiguration
    MyRsaKeysHandler* myRsaKeys = new MyRsaKeysHandler();
    int code = myRsaKeys->init();
    if(code != RSA_NO_ERROR){
        QMessageBox msgBox;
        msgBox.setText(QString("RSA Keys could not get found"));
        msgBox.exec();
    }
    configuration->setMyRsaKeysHandler(myRsaKeys);
    emit updateRsaBtns();
}

Contact MainwindowController::getUserForUsername(std::string username)
{
    return localStorage->getContactByUsername(username);
}

std::vector<std::string> MainwindowController::getMessagesForChat(Contact *receiver)
{
    return localStorage->getMessagesForChat(configuration->getCurrentUsernameAtDomainPair(), receiver->getUsername());
}

std::vector<Contact> MainwindowController::getKnownUsers()
{
    return localStorage->getKnownContacts();
}

bool MainwindowController::getBackendStatus() const
{
    return backendStatus;
}

CurrentConfiguration *MainwindowController::getCurrentConfiguration() const
{
    return configuration;
}

void MainwindowController::onBackgroundTransmissionFinished(QByteArray messageJsonString, int statusCode, BackgroundTransmitter *finishedThread)
{
    qDebug() << "FINISHED called!";
    if(statusCode == UNAUTHORIZED){
        emit feedBackGiven("Unauthorized - trying again!", statusCode);
        qDebug() << "UNAUTHORIZED";
        onAuthenticationRequired();
        RequestJsonWrapper updatedRequest(finishedThread->getMessageJsonObject());
        updatedRequest.setDeviceIdentifier(configuration->getCurrentDeviceIdentifier());
        BackgroundTransmitter* bgThread = new BackgroundTransmitter(finishedThread->getServerAdress(), finishedThread->getPort(), updatedRequest.getJsonString());
        connect(bgThread, SIGNAL(backgroundTransmissionFinished(QByteArray, int, BackgroundTransmitter*)), this,
                                  SLOT(onBackgroundTransmissionFinished(QByteArray,int, BackgroundTransmitter*)));
        bgThread->start();
    }
    if(statusCode == OK){
        emit feedBackGiven("Message transmitted!" , statusCode);
        storeEncryptedMessage(CryptoHelper::saveStringQByteConversion(messageJsonString), false);
        emit forceGuiUpdate();
    } else {
        emit feedBackGiven("Error transmitting message!" , statusCode);
    }
    delete finishedThread;
}

void MainwindowController::onQRCodeScanned(QString qrCodeText)
{
    qDebug() << "QRCodeScanned";
    std::string qrCodeJsonString = CryptoHelper::saveStringConversion(qrCodeText);
    if(RSAKeysImExportJsonWrapper::isValidRsaKeysObject(qrCodeJsonString)){
        MessageBoxYesNoDialog msgbox("Import RSA Keys", "Are you sure that you want to import the new RSA Keys.\r\nYou won't be able to read your old messages!");
        if(!msgbox.show()){
            return;
        }
        RSAKeysImExportJsonWrapper rsaKeysData(qrCodeJsonString);
        std::string privateKey;
        std::string publicKey;
        bool check = RSAWrapper::generateRsaKeyPairFileData(rsaKeysData, privateKey, publicKey);
        if(check){
            bool writeCheck = RSAWrapper::saveRSAKeysToFiles(publicKey, privateKey);
            if(writeCheck){
                QMessageBox msgBox;
                msgBox.setWindowTitle(QString("SUCCESS!"));
                msgBox.setText(QString("RSA Keys import succesful!"));
                msgBox.exec();
                initializeRSAKeys();
                emit forceGuiUpdate();
            } else {
                QMessageBox msgBox;
                msgBox.setWindowTitle(QString("FAIL!"));
                msgBox.setText(QString("RSA Keys import failed!"));
                msgBox.exec();
            }
        } else {
            QMessageBox msgBox;
            msgBox.setWindowTitle(QString("ERROR!"));
            msgBox.setText(QString("Could not convert RSA Json Object to Openssl RSA"));
            msgBox.exec();
        }
    } else if(ContactInExportJsonWrapper::isValidContactObject(qrCodeJsonString)) {
        ContactInExportJsonWrapper contact(qrCodeJsonString);
        emit importUserFromScannedQR(CryptoHelper::saveStringConversion(contact.getAlias()),
                                     CryptoHelper::saveStringConversion(contact.getUsername()),
                                     CryptoHelper::saveStringConversion(contact.getPublicKey()));
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString("No data found!"));
        msgBox.setText(QString("QR not known: " + CryptoHelper::saveStringConversion(qrCodeJsonString)));
        msgBox.exec();
    }
}

void MainwindowController::onMessagesAvailable(QString messagesAvailableFromUser)
{
    ResponseJsonWrapper response(CryptoHelper::saveStringConversion(messagesAvailableFromUser));
    MultiLinesResponseJsonWrapper heartbeat(response.getMessage());
    for(unsigned int i = 0; i < heartbeat.getLines().size(); i++){
        qDebug() << "Loading Messages for User: " << heartbeat.getLines()[i].c_str();
        Contact currentUser = getUserForUsername(heartbeat.getLines()[i]);
        if(currentUser.getUsername().compare("") == 0){
            QMessageBox msgBox;
            msgBox.setText("Message received from unknown user '"
                           + CryptoHelper::saveStringConversion(heartbeat.getLines()[i])
                           + "'! Add him to read his messages!");
            msgBox.exec();
        } else {
            receiveMessagesFromServer(heartbeat.getLines()[i]);
        }
    }
}

void MainwindowController::onMessageReceived(QString messageReceived)
{
    storeEncryptedMessage(CryptoHelper::saveStringConversion(messageReceived), true);
    emit forceGuiUpdate();
}

void MainwindowController::onAuthenticationRequired()
{
    qDebug() << "New Authentication";
    if(!triesToLogin){
        triesToLogin = true;
        backgroundScheduler->setLoggedIn(false);
        bool loggedIn = loginToServer();
        if(!loggedIn){
            showLoginForm();
        } else {
            backgroundScheduler->setLoggedIn(true);
        }
        triesToLogin = false;
    }
}

void MainwindowController::onLoggedOut()
{
    qDebug() << "Logged out!";
    configuration->setCurrentPassword("");
    showLoginForm();
}

void MainwindowController::onRefreshIpAdresses(QString ipAdressObjectsAsJson)
{
    qDebug() << "Update IP List";
    MultiLinesResponseJsonWrapper lines(CryptoHelper::saveStringConversion(ipAdressObjectsAsJson));
    qDebug() << "Number of response lines " << lines.getLines().size();
    for(unsigned int i = 0; i < lines.getLines().size(); i++){
        IpRefreshJsonWrapper ipObject(lines.getLines()[i]);
        localStorage->refreshIpAddressesByUsername(ipObject);
    }
}

void MainwindowController::onPullMessagesByUsername(QString username)
{
    qDebug() << "Pulling messages for user " << username;
    receiveMessagesFromServer(CryptoHelper::saveStringConversion(username));
}

void MainwindowController::sendMessage(Contact *receiver, QByteArray data, std::string filename)
{
    RequestJsonWrapper toSendMessage = generateEncryptedMessageForUser(receiver, data, filename);
    qDebug() << "SEND MESSAGE: " << toSendMessage.getJsonString().c_str();
    if(toSendMessage.getMessage().compare("") == 0){
        return;
    }
    if(configuration->getSaveMessagesOnServer()){
        //self destructing object
        BackgroundTransmitter* bgThread = new BackgroundTransmitter(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()), SERVER_PORT, toSendMessage);
        connect(bgThread, SIGNAL(backgroundTransmissionFinished(QByteArray, int, BackgroundTransmitter*)), this,
                                  SLOT(onBackgroundTransmissionFinished(QByteArray,int, BackgroundTransmitter*)));
        bgThread->start();
    } else {
        for(unsigned int i = 0; i < toSendMessage.getReceivers().size(); i++){
            std::vector<std::string> ipAdresses = localStorage->getIpAddressesByUsername(toSendMessage.getReceivers()[i]);
            if(ipAdresses.size() == 0){
                QMessageBox msgBox;
                msgBox.setText("No IP- Addresses found for contact " + CryptoHelper::saveStringConversion(toSendMessage.getReceivers()[i]));
                msgBox.exec();
            }
            for(unsigned int j = 0; j < ipAdresses.size(); j++){
                //self destructing object
                BackgroundTransmitter* bgThread = new BackgroundTransmitter(ipAdresses[i], P2P_PORT, toSendMessage);
                connect(bgThread, SIGNAL(backgroundTransmissionFinished(QByteArray, int, BackgroundTransmitter*)), this,
                                          SLOT(onBackgroundTransmissionFinished(QByteArray, int, BackgroundTransmitter*)));
                bgThread->start();
            }
        }
    }
}

void MainwindowController::sendFile(Contact *receiver, std::string filepath)
{
    QByteArray data;
    QFileInfo fileInfo(CryptoHelper::saveStringConversion(filepath));
    QString filename(fileInfo.fileName());
    bool isRead = FileIOHelper::readDataFromFile(filepath, data);
    if(isRead && (data.size() > 0)){
        sendMessage(receiver, data, CryptoHelper::saveStringConversion(filename));
    }
}

void MainwindowController::receiveMessagesFromServer(std::string receiver)
{
    std::string transmitter = configuration->getCurrentUsernameAtDomainPair();
    std::vector<std::string> receivers;
    receivers.push_back(receiver);
    QDateTime latestTimeStamp = localStorage->getLatestsTimeStampForChat(transmitter);
    RequestJsonWrapper toSendMessage(transmitter, receivers, latestTimeStamp, configuration->getCurrentDeviceIdentifier(), GET_ALL_MESSAGES_FOR_CHAT);
    qDebug() << "RECMESSAGES: " << toSendMessage.getJsonString().c_str();
    TcpClient communication(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()));
    communication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(toSendMessage.getJsonString()));
    ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(communication.getResponse()));
    if(response.getStatusCode() == OK){
        MultiLinesResponseJsonWrapper lines(response.getMessage());
        qDebug() << "Amount of Lines: " << lines.getLines().size();
        for(unsigned int i = 0; i < lines.getLines().size(); i++){
            storeEncryptedMessage(lines.getLines()[i]);
            if(i == (lines.getLines().size() - 1)){
                RequestJsonWrapper latestMessage(lines.getLines()[i]);
                localStorage->setLatestsTimeStampForChat(latestMessage.getTimestamp(), transmitter);
            }
        }
        emit forceGuiUpdate();
    } else {
        if(response.getStatusCode() == UNAUTHORIZED){
            onAuthenticationRequired();
            receiveMessagesFromServer(receiver);
        }
    }
}

bool MainwindowController::addNewUser(std::string username, std::string alias, std::string rsaPubKey)
{
    if(CryptoHelper::parseServerAdressOutOfUsername(username).compare("") == 0){
        username.append("@");
        username.append(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()));
        qDebug() << "New username " << username.c_str();
    }
    std::vector<std::string> receivers;
    receivers.push_back(username);
    if(rsaPubKey.compare("") == 0){
        RequestJsonWrapper request(configuration->getCurrentUsernameAtDomainPair(), receivers, QDateTime::currentDateTime(), configuration->getCurrentDeviceIdentifier(), GET_RSA_KEY_FOR_USER);
        TcpClient communication(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()));
        communication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(request.getJsonString()));
        ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(communication.getResponse()));
        if(response.getStatusCode() == OK){
            if(response.getMessage().compare("") != 0){
                receiveMessagesFromServer(username);
                return localStorage->addNewContact(username, alias, response.getMessage());
            } else {
                return false;
            }
        } else {
            if(response.getStatusCode() == UNAUTHORIZED){
                onAuthenticationRequired();
                return addNewUser(username, alias, rsaPubKey);
            }
            return false;
        }
    } else {
        receiveMessagesFromServer(username);
        return localStorage->addNewContact(username,alias, rsaPubKey);
    }
}

bool MainwindowController::generateUserRsaKeys()
{
    MessageBoxYesNoDialog msgbox("Create RSA Keys", "Are you sure that you want to create new RSA Keys instead of importing some?");
    if(!msgbox.show()){
        return false;
    }
    std::string publicKey;
    std::string privateKey;
    bool isKeysCreated = RSAWrapper::generateRsaKeysPair(publicKey, privateKey);
    if(isKeysCreated){
        bool areKeysSaved = RSAWrapper::saveRSAKeysToFiles(publicKey, privateKey);
        if(uploadPublicKey() && areKeysSaved){
            return true;
        } else {
            QMessageBox msgBox;
            msgBox.setText(QString("Keys could NOT be uploaded"));
            msgBox.exec();
        }
    } else {
        QMessageBox msgBox;
        msgBox.setText(QString("Keys could NOT get created"));
        msgBox.exec();
    }
    return false;
}

bool MainwindowController::uploadPublicKey()
{
    configuration->getMyRsaKeysHandler()->init();
    if(configuration->getMyRsaKeysHandler()->getPublicKey().compare("") != 0){
        RequestJsonWrapper request(configuration->getCurrentUsernameAtDomainPair(), configuration->getMyRsaKeysHandler()->getPublicKey(), QDateTime::currentDateTime(), configuration->getCurrentDeviceIdentifier(), SET_RSA_KEY_FOR_USER);
        TcpClient communication(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()));
        communication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(request.getJsonString()));
        ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(communication.getResponse()));
        QMessageBox msgBox;
        if(response.getStatusCode() != OK){
            if(response.getStatusCode() == METHOD_NOT_ALLOWED){
                msgBox.setText("ERROR: Could NOT upload public Key to server, because another Key is already stored! If you want to set a new one, delete the old over the webconfiguration mask!");
                msgBox.exec();
            } else if(response.getStatusCode() != UNAUTHORIZED) {
                onAuthenticationRequired();
                uploadPublicKey();
            } else {
                msgBox.setText("ERROR: Could NOT upload public Key to server! STATUS CODE: " + QString::number(response.getStatusCode()));
                msgBox.exec();
            }
            return false;
        } else {
            msgBox.setText("INFO: Keys created and public key is published on the server!");
            msgBox.exec();
            return true;
        }
    } else {
        QMessageBox msgBox;
        msgBox.setText(QString("Keys could NOT get loaded"));
        msgBox.exec();
    }
    return false;
}

void MainwindowController::exportRSAKeys()
{
    MessageBoxYesNoDialog msgbox("Export RSA Keys", "Are you sure that you want to export your RSA Key Data");
    if(!msgbox.show()){
        return;
    }
    RSA* rsaObj = RSAWrapper::createRSA(CryptoHelper::saveStringCharArrayConversion(configuration->getMyRsaKeysHandler()->getPrivateKey()), false);
    RSAKeysImExportJsonWrapper json(
                CryptoHelper::encodeBigNumBase64(rsaObj->p),
                CryptoHelper::encodeBigNumBase64(rsaObj->q),
                CryptoHelper::encodeBigNumBase64(rsaObj->e),
                CryptoHelper::encodeBigNumBase64(rsaObj->d));
    qDebug() << json.getJsonString().c_str();
    QRDialog dialog(std::string(json.getJsonString()));
    dialog.exec();
    RSA_free(rsaObj);
}

void MainwindowController::importRSAKeysFromFile(std::string filepath)
{
    QImage image;
    QByteArray data;
    bool check = FileIOHelper::readDataFromFile(filepath, data);
    if(!check) {
        QMessageBox msgBox;
        msgBox.setText("Could not load File");
        msgBox.exec();
        return;
    }
    //data as QByteArray
    image = QImage::fromData(data);
    QZXing decoder;
    //Decoder Format 0 = QR Code
    decoder.setDecoder(0);
    QString qrCodeText = decoder.decodeImage(image);
    qDebug() << "QR CODE:" << qrCodeText;
    if(qrCodeText.compare("") != 0){
        onQRCodeScanned(qrCodeText);
    } else {
        QMessageBox msgBox;
        msgBox.setText("No QR Code found!");
        msgBox.exec();
    }
}

void MainwindowController::exportContactDetails()
{
    ContactInExportJsonWrapper json(configuration->getCurrentAlias(), configuration->getCurrentUsernameAtDomainPair(), configuration->getMyRsaKeysHandler()->getPublicKey());
    qDebug() << json.getJsonString().c_str();
    QRDialog dialog(std::string(json.getJsonString()));
    dialog.exec();
}

void MainwindowController::initImageProcessPointer()
{
    qDebug() << "Registered Pointer" << ImageProcessor::getInstance();
    connect(ImageProcessor::getInstance(), SIGNAL(qrCodeScanned(QString)), this, SLOT(onQRCodeScanned(QString)));
}

void MainwindowController::saveConfigurationToDB(const std::string &identifier, std::string &value)
{
    localStorage->setConfigurationKey(identifier, value);
}

std::string MainwindowController::getConfigurationValueFromDB(const std::string &identifier)
{
    return localStorage->getConfigurationKey(identifier);
}

void MainwindowController::loadConfiguration()
{
    configuration->initConfigurationValues();
    QString chatyMediaFolder = QStandardPaths::locate(QStandardPaths::PicturesLocation, QString("chatyMedia"), QStandardPaths::LocateDirectory);
    if(chatyMediaFolder.compare("") == 0){
        chatyMediaFolder = QStandardPaths::locate(QStandardPaths::PicturesLocation, QString(""), QStandardPaths::LocateDirectory);
        chatyMediaFolder.append("chatyMedia");
        if(!QDir().mkdir(chatyMediaFolder)){
            qDebug() << "Could not create Media Folder!";
            chatyMediaFolder = QStandardPaths::locate(QStandardPaths::PicturesLocation, QString(""), QStandardPaths::LocateDirectory);
        }
    }
    configuration->setCurrentMediaPath(CryptoHelper::saveStringConversion(chatyMediaFolder));
}

bool MainwindowController::deleteUserByUsername(std::string username)
{
    return localStorage->deleteContactByUsername(username);
}

bool MainwindowController::updateAliasForUsername(std::string username, std::string alias)
{
    return localStorage->updateContactAliasByUsername(username, alias);
}

bool MainwindowController::updateRSAKeyForUsername(std::string username)
{
    std::vector<std::string> receivers;
    receivers.push_back(username);
    RequestJsonWrapper request(configuration->getCurrentUsernameAtDomainPair(), receivers, QDateTime::currentDateTime(), "", GET_RSA_KEY_FOR_USER);
    TcpClient communication(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()));
    communication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(request.getJsonString()));
    ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(communication.getResponse()));
    if(response.getStatusCode() == OK){
        if(response.getMessage().compare("") != 0){
            return localStorage->updateContactRSAKeyByUsername(username, response.getMessage());
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void MainwindowController::updateSchedulerIntervalls()
{
    backgroundScheduler->setPullMessagesIntervall(configuration->getPullMessagesIntervall());
    backgroundScheduler->setRefreshIpIntervall(configuration->getRefeshIpIntervall());
    backgroundScheduler->setHeartbeatIntervall(configuration->getHeartbeatIntervall());
}

bool MainwindowController::loginToServer()
{
    if(configuration->getCurrentUsernameAtDomainPair().compare("") != 0 && configuration->getCurrentPassword().compare("") != 0 && !triesToLogin){
        triesToLogin = true;
        qDebug() << "logging in to server";
        std::string serverPublicKey = configuration->getCurrentServerRSAPublicKey();
        if(serverPublicKey.compare("") == 0){
            return false;
        }
        std::string encryptedPassword;
        RSAWrapper::publicEncrypt(configuration->getCurrentPassword(), encryptedPassword, serverPublicKey);
        LoginMessageJsonWrapper loginObject(encryptedPassword, configuration->getCurrentDeviceIdentifier(), configuration->getMyRsaKeysHandler()->getPublicKey());
        RequestJsonWrapper requestLogin(configuration->getCurrentUsernameAtDomainPair(), loginObject.getJsonString(), QDateTime::currentDateTime(), configuration->getCurrentDeviceIdentifier(), CHECK_LOGIN_CREDENTIALS);
        TcpClient communicationForLogin(CryptoHelper::parseServerAdressOutOfUsername(configuration->getCurrentUsernameAtDomainPair()));
        communicationForLogin.transmitDataToServer(CryptoHelper::saveStringQByteConversion(requestLogin.getJsonString()));
        ResponseJsonWrapper responseLogin(CryptoHelper::saveStringQByteConversion(communicationForLogin.getResponse()));
        QMessageBox msgBox;
        switch(responseLogin.getStatusCode()){
            case ACCEPTED:
            case OK:
                qDebug() << "Login successful loginToServer";
                if(configuration->getMyRsaKeysHandler()->getPublicKey().compare("") == 0){
                    configuration->setCurrentDeviceIdentifier(responseLogin.getMessage());
                } else {
                    std::string deviceIdentifier;
                    RSAWrapper::privateDecrypt(responseLogin.getMessage(),
                                                deviceIdentifier,
                                                configuration->getMyRsaKeysHandler()->getPrivateKey());
                    configuration->setCurrentDeviceIdentifier(deviceIdentifier);
                }
                triesToLogin = false;
                return true;
                break;
            case FORBIDDEN:
                configuration->setCurrentDeviceIdentifier("");
                triesToLogin = false;
                return false;
                break;
            default:
                triesToLogin = false;
                return false;
                break;
        }
    }
    triesToLogin = false;
    return false;
}

void MainwindowController::showLoginForm()
{
    LoginDialog login(this, 0);
    login.exec();
    if(!login.isLoggedIn()){
        backgroundScheduler->setLoggedIn(false);
        emit forceClose();
    } else {
        backgroundScheduler->setLoggedIn(true);
    }
}

RequestJsonWrapper MainwindowController::generateEncryptedMessageForUser(Contact *receiver, QByteArray data, std::string filename)
{
    std::string iv = AESWrapper::generateRandomInitVektor();
    std::string key = AESWrapper::generateRandomKey();
    AESJsonWrapper aesJson(iv, key);
    std::vector<std::string> receivers;
    receivers.push_back(receiver->getUsername());
    std::map<std::string, std::string> aesValuesForUsers;
    std::string encryptedAesValueForUser;
    RSAWrapper::publicEncrypt(aesJson.getJsonString(), encryptedAesValueForUser, configuration->getMyRsaKeysHandler()->getPublicKey());
    aesValuesForUsers[configuration->getCurrentUsernameAtDomainPair()] = encryptedAesValueForUser;
    for(unsigned int i=0; i < receivers.size(); i++){
        std::string encryptedAesValue;
        int result = RSAWrapper::publicEncrypt(aesJson.getJsonString(), encryptedAesValue, receiver->getRsaPublicKey());
        if(result == -1){
            std::string message("Error: RSA Key for User ");
            message.append(receiver->getAlias());
            message.append(" corrupt!");
            emit feedBackGiven(CryptoHelper::saveStringConversion(message), 500);
            RequestJsonWrapper emptyRequest;
            return emptyRequest;

        }
        aesValuesForUsers[receivers[i]] = encryptedAesValue;
    }
    std::string dataAsString = CryptoHelper::encodeBase64(data);
    MimeJsonWrapper messageJson(dataAsString, filename);
    qDebug() << "MIME Nachricht: " << messageJson.getJsonString().c_str();
    std::string ciphertext = AESWrapper::encrypt(messageJson.getJsonString(), aesJson.getKey(), aesJson.getInitVector());
    std::string hmac = CryptoHelper::generateHMAC(aesJson.getJsonString(), ciphertext);
    return RequestJsonWrapper(configuration->getCurrentUsernameAtDomainPair(),
                              receivers,
                              aesValuesForUsers,
                              ciphertext,
                              QDateTime::currentDateTime(),
                              configuration->getCurrentDeviceIdentifier(),
                              hmac,
                              SEND_MESSAGE);
}

void MainwindowController::storeEncryptedMessage(std::string messageStr, bool p2pTransmitted)
{
    qDebug() << "Store Encrypted Message";
    RequestJsonWrapper message(messageStr);
    if(message.getMessage().compare("") == 0){
        return;
    }
    if(p2pTransmitted){
        message.setTimestamp(QDateTime::currentDateTime());
    }
    std::string timestamp = CryptoHelper::saveStringConversion(message.getTimestamp().toString());
    std::string transmitter = message.getTransmitter();
    std::string aesValueForUser;
    int result = RSAWrapper::privateDecrypt(message.getAesValuesForUsers()[configuration->getCurrentUsernameAtDomainPair()],
                                aesValueForUser,
                                configuration->getMyRsaKeysHandler()->getPrivateKey());
    if(result == -1){
        return;
    }
    std::string generatedHmac = CryptoHelper::generateHMAC(aesValueForUser, message.getMessage());
    if(message.getHmac().compare(generatedHmac) != 0){
        QMessageBox msgBox;
        msgBox.setText("Received Message with corrupted aesKey from user: " +
                       CryptoHelper::saveStringConversion(message.getTransmitter()) +
                       " => Message won't be stored!");
        qDebug() << "message Hmac: " << message.getHmac().c_str();
        qDebug() << "valid.  Hmac: " << CryptoHelper::generateHMAC(aesValueForUser, message.getMessage()).c_str();
        msgBox.exec();
        return;
    } else {
        qDebug() << "Message ok!";
    }
    AESJsonWrapper aesKeyAndInitVector(aesValueForUser);
    std::string messagePlaintext = AESWrapper::decrypt(message.getMessage(), aesKeyAndInitVector.getKey(), aesKeyAndInitVector.getInitVector());
    if(messagePlaintext.compare("") == 0){
        return;
    }
    MimeJsonWrapper messageMimeJson(messagePlaintext);
    message.setDeviceIdentifier("");
    if(messageMimeJson.getMimeType().inherits("text/plain")){
        qDebug() << "MESSAGE INHERITS TEXT";
        bool check = localStorage->insertMessagesInDB(message);
        if(!check){
            qDebug() << "Could not insert Messages";
        }
    } else {
        qDebug() << "MESSAGE INHERITS FILE";
        std::string dataAsString = messageMimeJson.getDataAsString();
        qDebug() << "Path: " << configuration->getCurrentMediaPath().c_str()  << messageMimeJson.getFilename().c_str();
        std::string savePath;
        savePath.append(configuration->getCurrentMediaPath());
        savePath.append("/");
        savePath.append(CryptoHelper::saveStringConversion(QString::number(message.getTimestamp().toTime_t())));
        savePath.append(messageMimeJson.getFilename());
        if(savePath.compare("") != 0){
            bool checkSave = FileIOHelper::writeDataToFile(savePath, CryptoHelper::decodeBase64(dataAsString));
            if(checkSave){
                std::string mimeHtml;
                mimeHtml.append("Path: ");
                mimeHtml.append(savePath);
                mimeHtml.append("<br>");
                mimeHtml.append("<img height=\"450\" width=\"450\" src=\"");
                mimeHtml.append(savePath);
                mimeHtml.append("\"/>");

                qDebug()<< "SavePath" << mimeHtml.c_str();
                messageMimeJson.setDataAsString(CryptoHelper::saveStringQByteConversion(CryptoHelper::encodeBase64(mimeHtml)));
                message.setMessage(AESWrapper::encrypt(messageMimeJson.getJsonString(), aesKeyAndInitVector.getKey(), aesKeyAndInitVector.getInitVector()));
                qDebug() << "SAVED";
                bool check = localStorage->insertMessagesInDB(message);
                if(!check){
                    qDebug() << "Could not insert Messages";
                }
            } else {
                qDebug() << "Could not store Data on Drive";
            }
        }
    }
}
