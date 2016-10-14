#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QDir>
#include <QDesktopServices>
#include "current_configuration.h"
#include "contact.h"
#include "chat_style_helper.h"
#include "../shared/rsa_wrapper.h"
#include "edit_contacts_dialog.h"
#include "aes_wrapper.h"
#include "../shared/file_io_helper.h"
#include "mainwindow_controller.h"
#include "../shared/crypto_helper.h"
#include "../shared/rsakeys_im_export_json_wrapper.h"
#include "../shared/my_rsa_keys_handler.h"
#include <iostream>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <stdlib.h>
#include <iostream>
#include "ui_mainwindow.h"
#include "imageprocessor.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    public:
        bool getIsReady() const;
        std::vector<Contact> getKnownUsers() const;

    public slots:
        void onForceGuiUpdate();
        void onImportUserFromScannedQR(QString alias, QString username, QString rsaPubKey);
        void onUpdateKnownUsersList();
        void resizeEvent ( QResizeEvent * event );
        void onForceClose();
        void onUpdateRsaBtns();
        void onFeedBackGiven(QString message, int statusCode);

    private slots:
        void on_receiveMessages_clicked();
        void linkClickedSlot(QUrl url);
        void on_chatReceiver_currentIndexChanged();
        void on_CloseApplication_clicked();
        void on_generateRSAKeysBtn_clicked();
        void on_addUserBtn_clicked();
        void on_knownUsersList_itemClicked(QListWidgetItem *item);
        void on_saveMessageOnServer_stateChanged();
        void on_exportRSAKeysBtn_clicked();
        void on_importRSAKEysBtn_clicked();
        void on_mainWindowTabs_tabBarClicked(int index);
        void on_exportMyContactDataBtn_clicked();
        void on_addUserRsaKeyInput_textChanged();
        void on_importContactBtn_clicked();
        void on_cameraRestartBtn_clicked();
        void on_addUserDeleteInputBtn_clicked();
        void on_logoutAndCloseBtn_clicked();
        void on_sendBtn_clicked();
        void on_sendFileBtn_clicked();
        void on_editContactsBtn_clicked();
        void on_uploadRSAPubKeyBtn_clicked();
        void on_hbSpinBox_valueChanged(int arg1);
        void on_ipSpinBox_valueChanged(int arg1);
        void on_refreshActiveChatSpinBox_valueChanged(int arg1);
        void on_alias_textEdited(const QString &arg1);
        void on_importRSAKeysFromFile_clicked();
        void on_showDeviceIdentifierBtn_clicked();
        void on_verifyPublicKeyBtn_clicked();

private:
        void setCameraSize();
        void loadKnownUsers();
        void initializeRSAKeys();
        void updateChatHistory();
        bool isReady;
        CurrentConfiguration *configuration = 0;
        Ui::MainWindow *ui;
        MainwindowController *controller = 0;
        std::vector<Contact> knownUsers;
        QQuickWidget* cameraQMLFeed = 0;
};

#endif // MAINWINDOW_H
