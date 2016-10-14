#include "contact.h"

Contact::Contact()
{
    this->alias = "";
    this->username = "";
    this->rsaPublicKey = "";
}

Contact::Contact(std::string username, std::string alias, std::string rsaKey)
{
    this->alias = alias;
    this->username = username;
    this->rsaPublicKey = rsaKey;
}

std::string Contact::getAlias() const
{
    return alias;
}

void Contact::setAlias(const std::string &value)
{
    alias = value;
}

std::string Contact::getUsername() const
{
    return username;
}

void Contact::setUsername(const std::string &value)
{
    username = value;
}

std::string Contact::getRsaPublicKey() const
{
    return rsaPublicKey;
}

void Contact::setRsaPublicKey(const std::string &value)
{
    rsaPublicKey = value;
}
