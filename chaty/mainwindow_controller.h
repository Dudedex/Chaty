#ifndef BACKENDLOGIC_H
#define BACKENDLOGIC_H

#include <QMessageBox>
#include <iostream>
#include <QCameraImageCapture>
#include <QCamera>
#include "local_storage.h"
#include "client_server_setup.h"
#include "../shared/my_rsa_keys_handler.h"
#include "current_configuration.h"
#include "contact.h"
#include "chat_style_helper.h"
#include "../shared/rsa_wrapper.h"
#include "aes_wrapper.h"
#include "background_scheduler.h"
#include "local_storage.h"
#include "background_scheduler.h"
#include "background_transmitter.h"
#include "../shared/file_io_helper.h"
#include "qr_dialog.h"
#include "login_dialog.h"
#include "../shared/tcp_client.h"
#include "../shared/request_json_wrapper.h"
#include "../shared/rsakeys_im_export_json_wrapper.h"
#include "../shared/contact_in_export_json_wrapper.h"
#include "../shared/login_message_json_wrapper.h"
#include "../shared/response_json_wrapper.h"
#include "../shared/mime_json_wrapper.h"
#include "../shared/aes_json_wrapper.h"
#include "../shared/ip_refresh_json_wrapper.h"
#include "../shared/transmissiontype.h"
#include "../shared/status_codes.h"
#include "../shared/crypto_helper.h"
#include "../shared/my_rsa_keys_handler.h"
#include "messagebox_yes_no_dialog.h"
#include <QZXing.h>
#include <imagehandler.h>
#include <QClipboard>
#include <QQuickWidget>

class CurrentConfiguration;
class BackgroundScheduler;

class MainwindowController: public QObject
{
        Q_OBJECT

    public slots:
        void onBackgroundTransmissionFinished(QByteArray messageJsonString, int statusCode, BackgroundTransmitter* finishedThread);
        void onQRCodeScanned(QString qrCodeText);
        void onMessagesAvailable(QString messagesAvailableFromUser);
        void onMessageReceived(QString messageReceived);
        void onAuthenticationRequired();
        void onLoggedOut();
        void onRefreshIpAdresses(QString ipAdressObjectsAsJson);
        void onPullMessagesByUsername(QString username);

    signals:
        void forceGuiUpdate();
        void importUserFromScannedQR(QString alias, QString username, QString rsaPubKey);
        void qrCodeScanned(QString qrCodeText);
        void forceClose();
        void updateRsaBtns();
        void feedBackGiven(QString message, int statusCode);

    public:
        MainwindowController(CurrentConfiguration *configuration);
        ~MainwindowController();
        void initializeRSAKeys();
        Contact getUserForUsername(std::string username);
        std::vector<std::string> getMessagesForChat(Contact *receiver);
        std::vector<Contact> getKnownUsers();
        bool getBackendStatus() const;
        CurrentConfiguration *getCurrentConfiguration() const;
        void sendMessage(Contact *receiver, QByteArray data, std::string filename = std::string(""));
        void sendFile(Contact *receiver, std::string filepath);
        void receiveMessagesFromServer(std::string receiver);
        bool addNewUser(std::string username, std::string alias, std::string rsaPubKey);
        bool generateUserRsaKeys();
        bool uploadPublicKey();
        void exportRSAKeys();
        void importRSAKeysFromFile(std::string filepath);
        void exportContactDetails();
        void initImageProcessPointer();
        void saveConfigurationToDB(const std::string &identifier, std::string &value);
        std::string getConfigurationValueFromDB(const std::string &identifier);
        void loadConfiguration();
        bool deleteUserByUsername(std::string username);
        bool updateAliasForUsername(std::string username, std::string alias);
        bool updateRSAKeyForUsername(std::string username);
        void updateSchedulerIntervalls();
        static bool shouldWindowFrameBeHidden();
        static std::string getOsName();

    private:
        LocalStorage *localStorage = 0;
        BackgroundScheduler* backgroundScheduler = 0;
        bool backendStatus;
        bool triesToLogin;
        CurrentConfiguration *configuration;
        ClientServerSetup *clientServerSetup = 0;
        bool loginToServer();
        void showLoginForm();
        RequestJsonWrapper generateEncryptedMessageForUser(Contact *receiver, QByteArray data, std::string filename = std::string(""));
        void storeEncryptedMessage(std::string messageStr, bool p2pTransmitted = false);
};

#endif // BACKENDLOGIC_H
