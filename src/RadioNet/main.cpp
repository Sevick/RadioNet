#include "mainwindow.h"
#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <qtsingleapplication.h>

QString AppPath;

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(Radiola);

    //QApplication a(argc, argv);
    QtSingleApplication a(argc, argv);
    if (a.sendMessage("Wakeup!"))
        return 0;

    AppPath=QDir::toNativeSeparators(QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0)+"/");
    QDir tAppDataDir(AppPath);
    if (!tAppDataDir.exists())
        tAppDataDir.mkdir(AppPath);

    a.addLibraryPath(QApplication::applicationDirPath());
    MainWindow w;
    a.setActivationWindow(&w);
    QObject::connect(&a, SIGNAL(messageReceived(const QString&)),
             &w, SLOT(activateMainWin(const QString&)));
    QApplication::setQuitOnLastWindowClosed(false);

    /*
    qDebug()<<"ArgsCount="<<argc;
    for (int i=0; i<argc; i++){
        qDebug()<<argv[i];
    }
    */

    qDebug()<<"AppPath="<<AppPath;
    w.show();
    w.INITDIALOG();

    return a.exec();
}





