#ifndef BACKGROUNDTRANSMITTER_H
#define BACKGROUNDTRANSMITTER_H

#include <QThread>
#include "tcp_communication.h"
#include "../shared/request_json_wrapper.h"
#include "../shared/crypto_helper.h"

class BackgroundTransmitter: public QThread
{
    Q_OBJECT
    public:
        BackgroundTransmitter(std::string serverAdress, int port, RequestJsonWrapper messageJsonObject);
        std::string getServerAdress() const;
        int getPort() const;
        RequestJsonWrapper getMessageJsonObject() const;

    private:
        void run();
        std::string serverAdress;
        int port;
        RequestJsonWrapper messageJsonObject;

    signals:
        void backgroundTransmissionFinished(QByteArray message, int statusCode, BackgroundTransmitter* finishedThread);
};

#endif // BACKGROUNDTRANSMITTER_H
