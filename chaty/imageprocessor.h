#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QMessageBox>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include <QDebug>
#include <QZXing.h>
#include <imagehandler.h>
#include "mainwindow_controller.h"

class ImageProcessor : public QObject
{
    Q_OBJECT
    public:
        explicit ImageProcessor(QObject *parent = 0);
        static ImageProcessor* getInstance();
        static void setHeight(int value);
        static void setWidth(int value);


    public slots:
        void processImage( const QString& image);
        static int getHeight();
        static int getWidth();

    signals:
        void qrCodeScanned(QString qrCodeText);

    private:
        static int height;
        static int width;
        static ImageProcessor* instance;

};
#endif // IMAGEPROCESSOR_H
