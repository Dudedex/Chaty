#include "imageprocessor.h"
//STATIC initialization
ImageProcessor* ImageProcessor::instance = 0;
int ImageProcessor::height = 0;
int ImageProcessor::width = 0;

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
{
    ImageProcessor::instance = this;
}

ImageProcessor *ImageProcessor::getInstance()
{
    if( instance == 0 ){
        instance = new ImageProcessor();
    }
    return instance;
}

void ImageProcessor::processImage( const QString& path)
{
    QUrl imageUrl(path);
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();
    QQmlImageProviderBase* imageProviderBase = engine->imageProvider( imageUrl.host());
    QQuickImageProvider* imageProvider = static_cast<QQuickImageProvider*>
         (imageProviderBase);

    QSize imageSize;
    QString imageId = imageUrl.path().remove(0,1);
    QImage image = imageProvider->requestImage(imageId, &imageSize, imageSize);
    if( !image.isNull()) {
        QZXing decoder;
        decoder.setDecoder(0);
        QString qrCodeText = decoder.decodeImage(image);
        qDebug() << "Scanned Text:" << qrCodeText;
        qDebug() << "Pointer 3 " << this;
        if(qrCodeText.compare("") != 0){
            emit qrCodeScanned(qrCodeText);
        } else {
            QMessageBox qrCode;
            qrCode.setWindowTitle(QString("Scanned QR Code"));
            qrCode.setText(QString("No QR Code found!"));
            qrCode.exec();
        }
    }
}

int ImageProcessor::getHeight()
{
    return height;
}

int ImageProcessor::getWidth()
{
    return width;
}

void ImageProcessor::setWidth(int value)
{
    width = value;
}

void ImageProcessor::setHeight(int value)
{
    height = value;
}
