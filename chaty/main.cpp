#include "mainwindow.h"
#include "local_storage.h"

QString LocalStorage::DATABASE_NAME("chaty.sqlite");

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString name = qgetenv("USER");
    if (name.isEmpty()){
        name = qgetenv("USERNAME");
    }
    if(!name.isEmpty()){
        LocalStorage::DATABASE_NAME = name.trimmed() + QString(".sqlite");
    }
    MainWindow w;
    if(w.getIsReady()){
        w.show();
        return a.exec();
    } else {
        return 0;
    }
}
