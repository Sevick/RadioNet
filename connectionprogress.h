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
    void setTitle(QString pTitle);

private:
    Ui::ConnectionProgress *ui;
    void setShadow(QPushButton* pButton);
public slots:
    void OnConnectionProgressUpdate(int pProgress);
    void OnConnectionEstablished();
    void OnConnectionFailed();
private slots:
    void on_pbCancel_clicked();
};

#endif // CONNECTIONPROGRESS_H
