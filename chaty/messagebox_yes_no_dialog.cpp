#include "messagebox_yes_no_dialog.h"

MessageBoxYesNoDialog::MessageBoxYesNoDialog(QString topic, QString question)
{
    this->topic = topic;
    this->question = question;
}

bool MessageBoxYesNoDialog::show()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(Q_NULLPTR, topic, question,
                            QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        return true;
    } else {
        return false;
    }
}
