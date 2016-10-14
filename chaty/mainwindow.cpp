#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->ui->addUserRsaKeyInput->setVisible(false);
    setWindowTitle("CHATY");
    configuration = new CurrentConfiguration();
    configuration->setSaveMessagesOnServer(ui->saveMessageOnServer->isChecked());
    controller = new MainwindowController(configuration);
    if(!controller->getBackendStatus()){
        isReady = false;
        return;
    } else {
        connect(controller, SIGNAL(forceClose()), this,
                                  SLOT(onForceClose()));
        connect(controller, SIGNAL(forceGuiUpdate()), this,
                                  SLOT(onForceGuiUpdate()));
        connect(controller, SIGNAL(importUserFromScannedQR(QString,QString,QString)), this,
                                  SLOT(onImportUserFromScannedQR(QString,QString,QString)));
        connect(controller, SIGNAL(updateRsaBtns()), this,
                                  SLOT(onUpdateRsaBtns()));
        connect(controller, SIGNAL(feedBackGiven(QString, int)), this,
                                  SLOT(onFeedBackGiven(QString,int)));

        ui->alias->setText(CryptoHelper::saveStringConversion(configuration->getCurrentAlias()));
        ui->username->setText(CryptoHelper::saveStringConversion(configuration->getCurrentUsernameAtDomainPair()));
        ui->hbSpinBox->setValue(configuration->getHeartbeatIntervall());
        ui->ipSpinBox->setValue(configuration->getRefeshIpIntervall());
        ui->refreshActiveChatSpinBox->setValue(configuration->getPullMessagesIntervall());

        configuration->setCurrentReceiverIndex(-1);

        ui->chatHistory->setOpenLinks(false);
        ui->chatHistory->setOpenExternalLinks(false);
        ui->chatHistory->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //ui->chatHistory->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        loadKnownUsers();

        cameraQMLFeed = new QQuickWidget(this->ui->tabQRImport);
        //PreResizing for mobile Phones
        setCameraSize();
        controller->initImageProcessPointer();

        onUpdateRsaBtns();

        isReady = true;
    }
}

MainWindow::~MainWindow()
{
    if(cameraQMLFeed != 0){
        delete cameraQMLFeed;
    }
    if(controller != 0){
        delete controller;
    }
    if(configuration != 0){
        delete configuration;
    }
    if(ui != 0){
        delete ui;
    }
}

void MainWindow::onForceGuiUpdate()
{
    qDebug() << "update Gui";
    ui->username->setText(CryptoHelper::saveStringConversion(configuration->getCurrentUsernameAtDomainPair()));
    ui->alias->setText(CryptoHelper::saveStringConversion(configuration->getCurrentAlias()));
    ui->messageBox->setPlainText(QString(""));
    updateChatHistory();
}

void MainWindow::onImportUserFromScannedQR(QString alias, QString username, QString rsaPubKey)
{
    ui->mainWindowTabs->setCurrentIndex(ui->mainWindowTabs->indexOf(ui->tabContacts));
    ui->addUserAliasInput->setText(alias);
    ui->addUserUsernameInput->setText(username);
    ui->addUserUsernameInput->setReadOnly(true);
    ui->addUserRsaKeyInput->setText(rsaPubKey);
}

void MainWindow::onUpdateKnownUsersList()
{
    loadKnownUsers();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    setCameraSize();
}

void MainWindow::onForceClose()
{
    close();
}

void MainWindow::onUpdateRsaBtns()
{
    if(configuration->getMyRsaKeysHandler()->getPrivateKey().compare("") != 0 && configuration->getMyRsaKeysHandler()->getPublicKey().compare("") != 0 ){
        ui->generateRSAKeysBtn->hide();
        ui->exportRSAKeysBtn->show();
        ui->uploadRSAPubKeyBtn->show();
        ui->verifyPublicKeyBtn->show();
    } else {
        ui->generateRSAKeysBtn->show();
        ui->uploadRSAPubKeyBtn->hide();
        ui->exportRSAKeysBtn->hide();
        ui->verifyPublicKeyBtn->hide();
    }
}

void MainWindow::onFeedBackGiven(QString message, int statusCode)
{
    if(statusCode != 200) {
        ui->feedBackLabel->setStyleSheet("QLabel { background-color : red; color : white; }");
    } else {
        ui->feedBackLabel->setStyleSheet("QLabel { background-color : green; color : white; }");
    }
    ui->feedBackLabel->setText(message);
}

void MainWindow::on_receiveMessages_clicked()
{
    controller->receiveMessagesFromServer(knownUsers[configuration->getCurrentReceiverIndex()].getUsername());
    updateChatHistory();
}

//This slot handles all clicks
void MainWindow::linkClickedSlot( QUrl url )
{
    QDesktopServices::openUrl(url);
    qDebug() << "clicked";

}

void MainWindow::on_chatReceiver_currentIndexChanged()
{
    qDebug() << "INDEX CHANGED";
    configuration->setCurrentReceiverIndex(ui->chatReceiver->currentIndex());
    if(ui->chatReceiver->currentIndex() > -1){
        qDebug() << "Current Receiver Index: " << configuration->getCurrentReceiverIndex();
        qDebug() << "CurrentReceiver Alias: " << knownUsers[configuration->getCurrentReceiverIndex()].getAlias().c_str();
        qDebug() << "CurrentReceiver Username: " << knownUsers[configuration->getCurrentReceiverIndex()].getUsername().c_str();
        controller->receiveMessagesFromServer(knownUsers[configuration->getCurrentReceiverIndex()].getUsername());
        updateChatHistory();
    }
}

void MainWindow::on_knownUsersList_itemClicked(QListWidgetItem *item)
{
    configuration->setCurrentReceiverIndex(ui->knownUsersList->row(item));
    qDebug() << "Current Receiver Index: " << configuration->getCurrentReceiverIndex();
    ui->chatReceiver->setCurrentIndex(configuration->getCurrentReceiverIndex());
    //Open Chat Tab
    ui->mainWindowTabs->setCurrentIndex(ui->mainWindowTabs->indexOf(ui->tabCurrentChat));
}

void MainWindow::on_CloseApplication_clicked()
{
    close();
}

void MainWindow::on_generateRSAKeysBtn_clicked()
{
    bool check = controller->generateUserRsaKeys();
    if(check){
        ui->generateRSAKeysBtn->hide();
    } else {
        QMessageBox msgBox;
        msgBox.setText(QString("Keys could NOT be saved"));
        msgBox.exec();
    }
}

void MainWindow::on_addUserBtn_clicked()
{
    QString username = ui->addUserUsernameInput->toPlainText().trimmed();
    QString alias = ui->addUserAliasInput->toPlainText().trimmed();
    QString rsaPubKey = ui->addUserRsaKeyInput->toPlainText().trimmed();

    bool check = controller->addNewUser(CryptoHelper::saveStringConversion(username), CryptoHelper::saveStringConversion(alias), CryptoHelper::saveStringConversion(rsaPubKey));

    if(check){
        ui->addUserUsernameInput->setText("");
        ui->addUserAliasInput->setText("");
        ui->addUserRsaKeyInput->setText("");
        ui->addUserUsernameInput->setReadOnly(false);
        ui->addUserRsaKeyInput->hide();
        loadKnownUsers();
    } else {
        QMessageBox msgBox;
        msgBox.setText("Could NOT add User!");
        msgBox.exec();
    }
}

void MainWindow::updateChatHistory()
{
    ui->chatHistory->setHtml("");
    ChatStyleHelper chatStyle;
    qDebug() << "CurrentReceiverIndext" << configuration->getCurrentReceiverIndex();
    if(configuration->getMyRsaKeysHandler()->getPrivateKey().compare("") != 0 && configuration->getCurrentReceiverIndex() > -1){
        std::vector<std::string> responseLines = controller->getMessagesForChat(&knownUsers[configuration->getCurrentReceiverIndex()]);
        for(unsigned int i = 0; i < responseLines.size(); i++){
            RequestJsonWrapper currentLine(responseLines[i]);
            std::string timestamp = CryptoHelper::saveStringConversion(currentLine.getTimestamp().toString());
            std::string transmitter = currentLine.getTransmitter();
            std::string aesValueForUser;
            int ret = RSAWrapper::privateDecrypt(currentLine.getAesValuesForUsers()[configuration->getCurrentUsernameAtDomainPair()],
                                                    aesValueForUser,
                                                    configuration->getMyRsaKeysHandler()->getPrivateKey());
            if(ret == -1){
                continue;
            }
            AESJsonWrapper aesKeyAndInitVector(aesValueForUser);
            std::string plaintext = AESWrapper::decrypt(currentLine.getMessage(), aesKeyAndInitVector.getKey(), aesKeyAndInitVector.getInitVector());
            qDebug() << "Plaintext << " << plaintext.c_str();
            if(plaintext.compare("") == 0){
                continue;
            }
            MimeJsonWrapper mimeJson(plaintext);
            std::string messageStr = CryptoHelper::saveStringQByteConversion(CryptoHelper::decodeBase64(mimeJson.getDataAsString()));
            if(transmitter.compare(configuration->getCurrentUsernameAtDomainPair()) == 0){
                ui->chatHistory->append(chatStyle.getFormattedUserMessage(timestamp, messageStr));
            } else {
                std::string displayName = knownUsers[configuration->getCurrentReceiverIndex()].getAlias();
                ui->chatHistory->append(chatStyle.getFormattedReceivedMessage(displayName, timestamp, messageStr));
            }
        }
    }
}

std::vector<Contact> MainWindow::getKnownUsers() const
{
    return knownUsers;
}

bool MainWindow::getIsReady() const
{
    return isReady;
}

void MainWindow::loadKnownUsers()
{
    configuration->setCurrentReceiverIndex(-1);
    ui->knownUsersList->clear();
    ui->chatReceiver->clear();
    knownUsers = controller->getKnownUsers();
    for(unsigned int i = 0; i < knownUsers.size(); i++){
        QString alias = CryptoHelper::saveStringConversion(knownUsers[i].getAlias());
        QListWidgetItem* listItem = new QListWidgetItem(alias);
        listItem->setIcon(QIcon(":/resources/resources/msg.jpg"));
        listItem->setTextAlignment(Qt::AlignCenter);
        ui->knownUsersList->addItem(listItem);
        ui->knownUsersList->setIconSize(QSize(250,250));
        ui->chatReceiver->addItem(alias);
    }
}

void MainWindow::on_saveMessageOnServer_stateChanged()
{
    qDebug() << "Box changed";
    configuration->setSaveMessagesOnServer(ui->saveMessageOnServer->isChecked());
}

void MainWindow::on_exportRSAKeysBtn_clicked()
{
    controller->exportRSAKeys();
}

void MainWindow::on_importRSAKEysBtn_clicked()
{
    ui->mainWindowTabs->setCurrentIndex(ui->mainWindowTabs->indexOf(ui->tabQRImport));
}

void MainWindow::on_exportMyContactDataBtn_clicked()
{
    controller->exportContactDetails();
}

void MainWindow::on_addUserRsaKeyInput_textChanged()
{
    bool visible = ui->addUserRsaKeyInput->toPlainText().compare("") != 0;
    ui->addUserRsaKeyInput->setVisible(visible);
}

void MainWindow::on_importContactBtn_clicked()
{
    ui->mainWindowTabs->setCurrentIndex(ui->mainWindowTabs->indexOf(ui->tabQRImport));
}

void MainWindow::on_mainWindowTabs_tabBarClicked(int index)
{
    Q_UNUSED(index);
}

void MainWindow::on_cameraRestartBtn_clicked()
{
    setCameraSize();
}

void MainWindow::setCameraSize()
{
    if(cameraQMLFeed){
        ImageProcessor::getInstance()->setHeight(ui->mainWindowTabs->height() - 40);
        ImageProcessor::getInstance()->setWidth(ui->mainWindowTabs->width() - 10);
        qmlRegisterType<ImageProcessor>("ImageProcessor", 1, 0, "ImageProcessor");
        cameraQMLFeed->setSource(QUrl(QStringLiteral("qrc:/declarative-camera.qml")));
        cameraQMLFeed->showMaximized();
        controller->initImageProcessPointer();
    }
}

void MainWindow::on_addUserDeleteInputBtn_clicked()
{
    ui->addUserAliasInput->setText("");
    ui->addUserUsernameInput->setText("");
    ui->addUserRsaKeyInput->setText("");
    ui->addUserUsernameInput->setReadOnly(false);
}

void MainWindow::on_logoutAndCloseBtn_clicked()
{
    configuration->setCurrentUsernameAtDomainPair(std::string(""));
    configuration->setCurrentPassword(std::string(""));
    close();
}

void MainWindow::on_sendBtn_clicked()
{
    QString message = ui->messageBox->toPlainText();
    if(message.length() == 0){
        return;
    }
    if(!knownUsers.empty()){
        std::string messageStr = CryptoHelper::saveStringConversion(message);
        controller->sendMessage(&knownUsers[configuration->getCurrentReceiverIndex()], CryptoHelper::saveStringQByteConversion(messageStr));
        updateChatHistory();
    } else {
        QMessageBox msgBox;
        msgBox.setText("No Receiver selected!");
        msgBox.exec();
    }
}

void MainWindow::on_sendFileBtn_clicked()
{
    if(!knownUsers.empty()){
        QFileDialog dialog;
        dialog.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setReadOnly(true);
        dialog.exec();
        if(!dialog.selectedFiles().empty()){
            QString filepath = dialog.selectedFiles().first();
            qDebug() << "Sending File: " << filepath;
            if(filepath.compare("") != 0){
                onFeedBackGiven(QString("SENDING FILE"), 200);
                controller->sendFile(&knownUsers[configuration->getCurrentReceiverIndex()], CryptoHelper::saveStringConversion(filepath));
            }
        }
    } else {
        QMessageBox msgBox;
        msgBox.setText("No Receiver selected!");
        msgBox.exec();
    }
}

void MainWindow::on_editContactsBtn_clicked()
{
    EditContactsDialog* editContacts;
    editContacts = new EditContactsDialog(controller, 0);
    connect(editContacts, SIGNAL(updateKnownUsersList()), this,
                              SLOT(onUpdateKnownUsersList()));
    editContacts->exec();
    delete editContacts;
}

void MainWindow::on_uploadRSAPubKeyBtn_clicked()
{
    controller->uploadPublicKey();
}

void MainWindow::on_hbSpinBox_valueChanged(int arg1)
{
    configuration->setHeartbeatIntervall(arg1);
    controller->updateSchedulerIntervalls();
}

void MainWindow::on_ipSpinBox_valueChanged(int arg1)
{
    configuration->setRefeshIpIntervall(arg1);
    controller->updateSchedulerIntervalls();
}

void MainWindow::on_refreshActiveChatSpinBox_valueChanged(int arg1)
{
    configuration->setPullMessagesIntervall(arg1);
    controller->updateSchedulerIntervalls();
}

void MainWindow::on_alias_textEdited(const QString &arg1)
{
    this->configuration->setCurrentAlias(CryptoHelper::saveStringConversion(arg1));
}

void MainWindow::on_importRSAKeysFromFile_clicked()
{
    QFileDialog dialog;
    dialog.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setReadOnly(true);
    dialog.setNameFilter(tr("Image Files (*.png *.jpg *.bmp)"));
    dialog.exec();
    if(!dialog.selectedFiles().empty()){
        QString filepath = dialog.selectedFiles().first();
        qDebug() << "Loading QR File: " << filepath;
        if(filepath.compare("") != 0){
            controller->importRSAKeysFromFile(CryptoHelper::saveStringConversion(filepath));
        }
    }

}

void MainWindow::on_showDeviceIdentifierBtn_clicked()
{
    QMessageBox msgBox;
    msgBox.setText(CryptoHelper::saveStringConversion(configuration->getCurrentDeviceIdentifier()));
    msgBox.exec();
}

void MainWindow::on_verifyPublicKeyBtn_clicked()
{
    QMessageBox msgBox;
    QString text("Key Hash: ");
    text.append(CryptoHelper::saveStringConversion(CryptoHelper::generateSha256(configuration->getMyRsaKeysHandler()->getPublicKey().substr(0,configuration->getMyRsaKeysHandler()->getPublicKey().size() - 1))));
    qDebug() << "Legnth!!!!!:>" << configuration->getMyRsaKeysHandler()->getPublicKey().size();
    msgBox.setText(text);
    msgBox.exec();
}
