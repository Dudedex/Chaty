#include "qr_dialog.h"
#include "ui_qr_dialog.h"

//Table after http://www.qrcode.com/en/about/version.html
const int BYTE_CAPACITY[40][4] = {
    {  17,   14,   11,	  7},{  32,   26,   20,   14},{  53,   42,   32,   24},{  78,   62,   46,   34},
    { 106,   84,   60,   44},{ 134,  106,   74,   58},{ 154,  122,   86,   64},{ 192,  152,  108,   84},
    { 230,  180,  130,   98},{ 271,  213,  151,  119},{ 321,  251,  177,  137},{ 367,  287,  203,  155},
    { 425,  331,  241,  177},{ 458,  362,  258,  194},{ 520,  412,  292,  220},{ 586,  450,  322,  250},
    { 644,  504,  364,  280},{ 718,  560,  394,  310},{ 792,  624,  442,  338},{ 858,  666,  482,  382},
    { 929,  711,  509,  403},{1003,  779,  565,  439},{1093,  857,  611,  461},{1171,  911,  661,  511},
    {1273,  997,  715,  535},{1367, 1059,  751,  593},{1465, 1125,  805,  625},{1528, 1190,  868,  658},
    {1628, 1264,  908,  698},{1732, 1370,  982,  742},{1840, 1452, 1030,  790},{1952, 1538, 1112,  842},
    {2068, 1628, 1168,  898},{2188, 1722, 1228,  958},{2303, 1809, 1283,  983},{2431, 1911, 1351, 1051},
    {2563, 1989, 1423, 1093},{2699, 2099, 1499, 1139},{2809, 2213, 1579, 1219},{2953, 2331, 1663, 1273}
};

const int PADDING_FOR_MODE[2] = { 8, 16};

const int INDICATOR_MODE_BITS = 4;

QRDialog::QRDialog(std::string qrCodeText, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QRDialog)
{
    ui->setupUi(this);
    setWindowTitle(QString("QR- Code generator"));
    if(MainwindowController::shouldWindowFrameBeHidden()){
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    }
    this->qrCodeText = qrCodeText;
    qDebug() << "LENGTH: " << qrCodeText.length();
    reDrawQRCode();
}

QRDialog::~QRDialog()
{
    if(qr != NULL) {
        QRcode_free(qr);
    }
    delete ui;
}

void QRDialog::on_closeButton_clicked()
{
    close();
}

void QRDialog::reDrawQRCode()
{
    int finalVersion = 0;
    int qrEcLevel = ui->qrEcLevelDropdown->currentIndex();
    for(int version = 0; version <= 40; version++){
        int bits = BYTE_CAPACITY[version][qrEcLevel] * 8;
        int digits = 0;
        bits -= INDICATOR_MODE_BITS;
        if (version < 10){
            digits = PADDING_FOR_MODE[0];
        } else {
            digits = PADDING_FOR_MODE[1];
        }
        int modebits = bits - digits;
        unsigned int maxPossibleCharacterDigits = modebits / 8;
        if(maxPossibleCharacterDigits >= qrCodeText.length()){
            finalVersion = version;
            break;
        }
    }

    qDebug() << "Final version: " << finalVersion;

    qr = QRcode_encodeString(qrCodeText.data(),
                             finalVersion,
                             static_cast<QRecLevel>(qrEcLevel),
                             QR_MODE_8,
                             1);
    this->setContentsMargins(40,40,40,40);

    int imageSize =  this->ui->imageFrame->width() > this->ui->imageFrame->height()? this->ui->imageFrame->height(): this->ui->imageFrame->width();
    QRect imageRect(0, 0, imageSize, imageSize);
    this->ui->imageFrame->setGeometry(imageRect);
    this->ui->imageFrame->setPixmap( QPixmap::fromImage(drawQRCodeToImage(imageSize)));
}

QImage QRDialog::drawQRCodeToImage(int size)
{
    if(size > 0){
        QImage image(size, size, QImage::Format_Mono);
        QPainter painter(&image);
        QColor background(Qt::white);
        painter.setBrush(background);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, size, size);
        if(qr != NULL)
        {
            QColor foreground(Qt::black);
            painter.setBrush(foreground);
            const int qr_width = qr->width > 0 ? qr->width : 1;
            double scale_x = size / qr_width;
            double scale_y = size / qr_width;
            for( int y = 0; y < qr_width; y ++){
                for(int x = 0; x < qr_width; x++){
                    unsigned char b = qr->data[y * qr_width + x];
                    if(b & 0x01){
                        QRectF r(x * scale_x, y * scale_y, scale_x, scale_y);
                        painter.drawRects(&r, 1);
                    }
                }
            }
        }
        return image;
    } else {
        return QImage();
    }
}

void QRDialog::on_qrEcLevelDropdown_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    reDrawQRCode();
}
