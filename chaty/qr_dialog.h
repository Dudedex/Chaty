#ifndef QR_DIALOG_H
#define QR_DIALOG_H

#include <QDialog>
#include <QPainter>
#include <QLabel>
#include <QImage>
#include <iostream>
#include "qrencode/qrencode.h"
#include "mainwindow_controller.h"
#include "../shared/crypto_helper.h"

namespace Ui {
class QRDialog;
}

class QRDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit QRDialog(std::string qrCodeText = std::string(""), QWidget *parent = 0);
        ~QRDialog();

    private slots:
        void on_closeButton_clicked();
        void on_qrEcLevelDropdown_currentIndexChanged(int index);

    private:
        void reDrawQRCode();
        QImage drawQRCodeToImage(int size);
        Ui::QRDialog *ui;
        std::string qrCodeText;
        QRcode* qr;
};

#endif // QR_DIALOG_H
