#include "login_dialog.h"
#include "ui_login_dialog.h"

LoginDialog::LoginDialog(MainwindowController *controller, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle(QString("Login"));
    if(MainwindowController::shouldWindowFrameBeHidden()){
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    }
    loggedIn = false;
    this->controller = controller;
    ui->loginUsernameInput->setText(CryptoHelper::saveStringConversion(controller->getCurrentConfiguration()->getCurrentUsernameAtDomainPair()));
    ui->loginLoginBtn->focusProxy();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

bool LoginDialog::isLoggedIn()
{
    return loggedIn;
}

void LoginDialog::login(std::string &pwSalt, std::string &username, std::string &plainTextPassword, std::string &serverAddress)
{
    std::string hashedPw = pwSalt;
    std::string sha256PreHashText = pwSalt;
    sha256PreHashText.append(plainTextPassword);
    hashedPw.append(CryptoHelper::generateSha256(sha256PreHashText));
    qDebug() << "Hashed PW: " << hashedPw.c_str();
    // "" for fresh login
    std::string encryptedPw;
    RSAWrapper::publicEncrypt(hashedPw, encryptedPw, controller->getCurrentConfiguration()->getCurrentServerRSAPublicKey());
    std::string myPublicKey = controller->getCurrentConfiguration()->getMyRsaKeysHandler()->getPublicKey();
    LoginMessageJsonWrapper loginObject(encryptedPw, std::string(""), myPublicKey);
    RequestJsonWrapper requestLogin(username, loginObject.getJsonString(), QDateTime::currentDateTime(), "", CHECK_LOGIN_CREDENTIALS);
    TcpClient communicationForLogin(serverAddress);
    communicationForLogin.transmitDataToServer(CryptoHelper::saveStringQByteConversion(requestLogin.getJsonString()));
    ResponseJsonWrapper responseLogin(CryptoHelper::saveStringQByteConversion(communicationForLogin.getResponse()));
    QMessageBox msgBox;
    switch(responseLogin.getStatusCode()){
        case ACCEPTED:
            msgBox.setText("RSA public key differs from server side stored public Key!");
            msgBox.exec();
        case OK:
            qDebug() << "Login successful";
            controller->getCurrentConfiguration()->setCurrentUsernameAtDomainPair(username);
            controller->getCurrentConfiguration()->setCurrentPassword(hashedPw);
            if(myPublicKey.compare("") == 0){
                controller->getCurrentConfiguration()->setCurrentDeviceIdentifier(responseLogin.getMessage());
            } else {
                std::string deviceIdentifier;
                RSAWrapper::privateDecrypt(responseLogin.getMessage(),
                                            deviceIdentifier,
                                            controller->getCurrentConfiguration()->getMyRsaKeysHandler()->getPrivateKey());
                controller->getCurrentConfiguration()->setCurrentDeviceIdentifier(deviceIdentifier);
            }

            loggedIn = true;
            close();
            break;
        case INTERNAL_SERVER_ERROR:
            ui->loginErrorLabel->setText(QString("Internal server error!"));
            break;
        case NOT_FOUND:
            ui->loginErrorLabel->setText(QString("Server not found!"));
            break;
        default:
            ui->loginErrorLabel->setText(QString("Login credentials wrong!"));
            break;
    }

}

void LoginDialog::on_loginCloseApplicationBtn_clicked()
{
    loggedIn = false;
    close();
}

void LoginDialog::on_loginLoginBtn_clicked()
{
    ui->loginErrorLabel->setText(QString(""));
    std::string username = CryptoHelper::saveStringConversion(ui->loginUsernameInput->text());
    std::string plainTextPassword = CryptoHelper::saveStringConversion(ui->loginPasswordInput->text());
    std::string serverAddress = CryptoHelper::parseServerAdressOutOfUsername(username);
    if(CryptoHelper::parseServerAdressOutOfUsername(controller->getCurrentConfiguration()->getCurrentUsernameAtDomainPair()).compare(serverAddress) != 0){
        QDateTime date;
        date.setTime_t(0);
        RequestJsonWrapper request(username, "", date, "",GET_RSA_KEY_FOR_SERVER);
        TcpClient communication(serverAddress);
        communication.transmitDataToServer(CryptoHelper::saveStringQByteConversion(request.getJsonString()));
        ResponseJsonWrapper response(CryptoHelper::saveStringQByteConversion(communication.getResponse()));
        switch(response.getStatusCode()){
            case OK:
                qDebug() << "Saving Rsa Pub Key from Server";
                controller->getCurrentConfiguration()->setServerRSAPublicKey(response.getMessage());
                break;
            case INTERNAL_SERVER_ERROR:
                ui->loginErrorLabel->setText(QString("Internal server error!"));
                return;
                break;
            case NOT_FOUND:
                ui->loginErrorLabel->setText(QString("Server not found!"));
                return;
                break;
            default:
                ui->loginErrorLabel->setText(QString("Login credentials wrong!"));
                return;
                break;
        }
    }
    ui->loginErrorLabel->setText(QString(""));
    TcpClient communicationForSalt(serverAddress);
    RequestJsonWrapper requestSalt(username, "", QDateTime::currentDateTime(), "", GET_PW_SALT_FOR_USER);
    communicationForSalt.transmitDataToServer(CryptoHelper::saveStringQByteConversion(requestSalt.getJsonString()));
    ResponseJsonWrapper responseSalt(CryptoHelper::saveStringQByteConversion(communicationForSalt.getResponse()));
    switch(responseSalt.getStatusCode()){
        case OK:
            if(responseSalt.getMessage().compare("") != 0){
                std::string pwSalt = responseSalt.getMessage();
                qDebug() << "PWSalt" << pwSalt.c_str();
                login(pwSalt, username, plainTextPassword, serverAddress);
            } else {
                ui->loginErrorLabel->setText(QString("Internal server error!"));
            }
            break;
        case INTERNAL_SERVER_ERROR:
            ui->loginErrorLabel->setText(QString("Internal server error!"));
            break;
        case NOT_FOUND:
            ui->loginErrorLabel->setText(QString("Server not found!"));
            break;
        default:
            ui->loginErrorLabel->setText(QString("Login credentials wrong!"));
            break;
    }
}
