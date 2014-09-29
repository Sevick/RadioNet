#include <QDebug>
#include "connectionprogress.h"
#include "ui_connectionprogress.h"
#include "defs.h"

ConnectionProgress::ConnectionProgress(QString pRadioTitle,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionProgress)
{
    ui->setupUi(this);
    ui->lRadioTitle->setText(pRadioTitle);
}

ConnectionProgress::~ConnectionProgress()
{
    delete ui;
}


void ConnectionProgress::OnConnectionProgressUpdate(int pProgress){
    ui->lConnectBar->setValue(pProgress);
}


void ConnectionProgress::OnConnectionEstablished(){
    accept();
}

void ConnectionProgress::OnConnectionFailed(){
    setResult(CONNECTION_STATUS_FAILED);
}

void ConnectionProgress::ChangeURL(QString pNewURL){
    ui->lURL->setText(pNewURL);
}
