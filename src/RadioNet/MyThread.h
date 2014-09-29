#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QElapsedTimer>
#include "mainwindow.h"

class MyThread : public QThread
{
    Q_OBJECT

    friend class MainWindow;
 protected:
    void OpenURL();
 public:
     void run();
     void MSleep(const unsigned long& milliSEC) { QThread::msleep(milliSEC); }
     void Error(const char* es, bool bModal=true);
     QString url;
 signals:
    void ShowError(const char *es, int ec, bool bModal);
    void ConnectionStartFailed();
};

////////////////////////////////////////////////////////////////////////////



#endif // MYTHREAD_H
