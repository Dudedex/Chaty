#include <QCoreApplication>
#include "server_setup.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);   
    ServerSetup serverSetup;
    if(serverSetup.isInitialized()){
        return a.exec();
    } else {
        return -1;
    }
}
