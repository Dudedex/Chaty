#ifndef CHATSTYLEHELPER_H
#define CHATSTYLEHELPER_H

#include "../shared/request_json_wrapper.h"
#include <QDebug>
#include "../shared/crypto_helper.h"

class ChatStyleHelper
{
    public:
        ChatStyleHelper();
        QString getFormattedUserMessage(std::string timestamp, std::string message);
        QString getFormattedReceivedMessage(std::string transmitter, std::string timestamp, std::string message);
};

#endif // CHATSTYLEHELPER_H
