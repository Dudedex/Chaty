#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include "mainwindow_controller.h"

namespace Ui {
class LoginDialog;
}

class MainwindowController;

class LoginDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit LoginDialog(MainwindowController *controller, QWidget *parent = 0);
        ~LoginDialog();
        bool isLoggedIn();

    private slots:
        void on_loginCloseApplicationBtn_clicked();
        void on_loginLoginBtn_clicked();

    private:
        void login(std::string &pwSalt, std::string &username, std::string &plainTextPassword, std::string &serverAddress);
        Ui::LoginDialog *ui;
        MainwindowController *controller;
        bool loggedIn;
        void handleAcceptedLoginRequest();
        void handleSuccessfulLogin();
};

#endif // LOGIN_DIALOG_H
