#ifndef CURRENTRECEIVER_H
#define CURRENTRECEIVER_H

#include <iostream>

class Contact
{
    public:
        Contact();
        Contact(std::string username, std::string alias, std::string rsaPublicKey);
        std::string getAlias() const;
        std::string getUsername() const;
        std::string getRsaPublicKey() const;
        void setAlias(const std::string &value);
        void setUsername(const std::string &value);
        void setRsaPublicKey(const std::string &value);

    private:
        std::string alias;
        std::string username;
        std::string rsaPublicKey;
};

#endif // CURRENTRECEIVER_H
