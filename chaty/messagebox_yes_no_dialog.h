#ifndef MESSAGEBOXYESNODIALOG_H
#define MESSAGEBOXYESNODIALOG_H

#include <QObject>
#include <QMessageBox>

class MessageBoxYesNoDialog
{
    public:
        MessageBoxYesNoDialog(QString topic, QString question);
        bool show();
    private:
        QString question;
        QString topic;
};

#endif // MESSAGEBOXYESNODIALOG_H
