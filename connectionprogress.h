#ifndef CONNECTIONPROGRESS_H
#define CONNECTIONPROGRESS_H

#include <QDialog>

namespace Ui {
class ConnectionProgress;
}

class ConnectionProgress : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionProgress(QString pRadioTitle,QWidget *parent = 0);
    ~ConnectionProgress();

    void ChangeURL(QString pNewURL);

private:
    Ui::ConnectionProgress *ui;

public slots:
    void OnConnectionProgressUpdate(int pProgress);
    void OnConnectionEstablished();
    void OnConnectionFailed();
};

#endif // CONNECTIONPROGRESS_H
