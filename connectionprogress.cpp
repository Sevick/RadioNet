#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include "connectionprogress.h"
#include "ui_connectionprogress.h"
#include "defs.h"

ConnectionProgress::ConnectionProgress(QString pRadioTitle,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionProgress)
{
    ui->setupUi(this);
    ui->lRadioTitle->setText(pRadioTitle);
    setShadow(ui->pbCancel);
}

ConnectionProgress::~ConnectionProgress(){
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

void ConnectionProgress::setTitle(QString pTitle){
    ui->lRadioTitle->setText(pTitle);
}

void ConnectionProgress::setShadow(QPushButton* pButton){
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(2); //Adjust accordingly
    effect->setOffset(3,3); //Adjust accordingly
    pButton->setGraphicsEffect(effect);
}

void ConnectionProgress::on_pbCancel_clicked()
{
    reject();
}
