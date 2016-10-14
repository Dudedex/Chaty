#include "client_server_setup.h"

void ClientServerSetup::onMessageReceived(QString message)
{
    qDebug() << "ClientServerSetup emit message";
    emit messageReceived(message);
}

ClientServerSetup::ClientServerSetup()
{
    if (!clientServer.listen(QHostAddress::Any, P2P_PORT)) {
        qDebug()<< "Unable to start Server " << clientServer.errorString();
        QMessageBox msgBox;
        msgBox.setText(QString("Unable to start Server ") + clientServer.errorString());
        msgBox.exec();
        return;
    }

    connect(&clientServer, SIGNAL(messageReceived(QString)), this,
                              SLOT(onMessageReceived(QString)));

    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    QString serverStatus;
    serverStatus += QString("The server is reachable on:");
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).toIPv4Address()) {
            serverStatus += QString("\n   - ") + ipAddressesList.at(i).toString();
        }
    }
    serverStatus += QString("\nPort: ");
    serverStatus += QString::number(clientServer.serverPort());
    qDebug() << serverStatus;
}
