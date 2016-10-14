#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QtNetwork>
#include <QDebug>

class ConnectionThread: public QThread
{
    Q_OBJECT

    public:
        ConnectionThread(int socketDescriptor, QObject *parent);
        void run();
    private:
        int socketDescriptor;
};
#endif // CONNECTIONTHREAD_H
