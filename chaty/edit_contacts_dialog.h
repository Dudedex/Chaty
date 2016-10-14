#ifndef EDIT_CONTACTS_H
#define EDIT_CONTACTS_H

#include <QDialog>
#include "mainwindow_controller.h"
#include "ui_edit_contacts_dialog.h"

namespace Ui {
class EditContacts;
}

class EditContactsDialog : public QDialog
{
    Q_OBJECT

    signals:
    void updateKnownUsersList();

    public:
        explicit EditContactsDialog(MainwindowController *controller, QWidget *parent = 0);
        ~EditContactsDialog();

    private slots:
        void on_deleteUserBtn_clicked();

        void on_editList_itemClicked(QListWidgetItem *item);

        void on_closeBtn_clicked();

        void on_saveAliasBtn_clicked();

        void on_updateRSAKey_clicked();

        void on_verifyPublicKeyBtn_clicked();

private:
        void fillList();
        Ui::EditContacts *ui;
        MainwindowController *controller;
        std::vector<Contact> knownUsers;
        int selectedUserIndex;
};

#endif // EDIT_CONTACTS_H
