#include "edit_contacts_dialog.h"

EditContactsDialog::EditContactsDialog(MainwindowController *controller, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditContacts)
{
    ui->setupUi(this);
    setWindowTitle(QString("Edit contacts"));
    if(MainwindowController::shouldWindowFrameBeHidden()){
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    }
    selectedUserIndex = -1;
    this->controller = controller;
    fillList();
}

EditContactsDialog::~EditContactsDialog()
{
    delete ui;
}

void EditContactsDialog::on_deleteUserBtn_clicked()
{
    if(selectedUserIndex > -1){
        controller->deleteUserByUsername(knownUsers[selectedUserIndex].getUsername());
    }
    emit updateKnownUsersList();
    selectedUserIndex = -1;
    ui->editList->clearSelection();
    this->knownUsers = controller->getKnownUsers();
    fillList();
}

void EditContactsDialog::fillList()
{
    ui->editList->clear();
    knownUsers = controller->getKnownUsers();
    for(unsigned int i = 0; i < knownUsers.size(); i++){
        QListWidgetItem* listItem = new QListWidgetItem(CryptoHelper::saveStringConversion(knownUsers[i].getAlias()));
        listItem->setTextAlignment(Qt::AlignCenter);
        ui->editList->addItem(listItem);
    }
}

void EditContactsDialog::on_editList_itemClicked(QListWidgetItem *item)
{
    selectedUserIndex = ui->editList->row(item);
    ui->usernameLabel->setText(CryptoHelper::saveStringConversion(knownUsers[selectedUserIndex].getUsername()));
    ui->alias->setText(CryptoHelper::saveStringConversion(knownUsers[selectedUserIndex].getAlias()));
}

void EditContactsDialog::on_closeBtn_clicked()
{
    close();
}

void EditContactsDialog::on_saveAliasBtn_clicked()
{
    std::string newAlias = CryptoHelper::saveStringConversion(ui->alias->text());
    if(selectedUserIndex > -1 && newAlias.compare("") != 0){
        if(controller->updateAliasForUsername(knownUsers[selectedUserIndex].getUsername(), newAlias)){
            emit updateKnownUsersList();
            QMessageBox msgBox;
            msgBox.setText("Alias updated");
            msgBox.exec();
            fillList();
        } else {
            QMessageBox msgBox;
            msgBox.setText("ERROR: Alias could not get updated");
            msgBox.exec();
        }
    }
}

void EditContactsDialog::on_updateRSAKey_clicked()
{
    if(selectedUserIndex > -1){
        if(controller->updateRSAKeyForUsername(knownUsers[selectedUserIndex].getUsername())){
            emit updateKnownUsersList();
            QMessageBox msgBox;
            msgBox.setText("Received current RSA Public Key for " + CryptoHelper::saveStringConversion(knownUsers[selectedUserIndex].getAlias()) + " from server!");
            msgBox.exec();
        } else {
            QMessageBox msgBox;
            msgBox.setText("ERROR: Could not receive RSA Public Key for " + CryptoHelper::saveStringConversion(knownUsers[selectedUserIndex].getAlias()) + " from server!");
            msgBox.exec();
        }
    }
}

void EditContactsDialog::on_verifyPublicKeyBtn_clicked()
{
    if(selectedUserIndex > -1){
        QMessageBox msgBox;
        QString text("Key Hash: ");
        text.append(CryptoHelper::saveStringConversion(CryptoHelper::generateSha256(knownUsers[selectedUserIndex].getRsaPublicKey())));
        qDebug() << "Legnth!!!!!:>" << knownUsers[selectedUserIndex].getRsaPublicKey().size();
        msgBox.setText(text);
        msgBox.exec();
    }
}
