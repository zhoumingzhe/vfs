#include <QtCore/QCoreApplication>
#include "downloadmanager.h"
#include "updatemanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    DownloadManager manager;
    HashChecker checker;
    UpdateManager updatemgr(&manager, &checker);

    updatemgr.StartUpdate();
    //QObject::connect(&manager, SIGNAL(finished()), &app, SLOT(quit()));
    return app.exec();
}
