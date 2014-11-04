#include <QDebug>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include "radioedit.h"
#include "playlistcl.h"
#include "ui_radioedit.h"

RadioEdit::RadioEdit(RadioCL* pRadio,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RadioEdit)
{


    radioData=pRadio;
    ui->setupUi(this);

    connect(ui->pbOk,SIGNAL(clicked()),this,SLOT(accept()));
    connect(ui->pbCancel,SIGNAL(clicked()),this,SLOT(reject()));


    ui->eRadioTitle->setText(radioData->GetTitle());
    ui->ePlaylist->setText(radioData->GetPlaylist());
    if (radioData->GetPlaylist()!="")
        ui->pbLoadURLs->setEnabled(true);
    else
        ui->pbLoadURLs->setEnabled(false);

    fillURLs();

    setShadow(ui->pbOk);
    setShadow(ui->pbCancel);
    setShadow(ui->pbLoadURLs);
}

RadioEdit::~RadioEdit()
{
    delete ui;
}

int RadioEdit::exec(){
    int tResult=QDialog::exec();
    if (tResult==QDialog::Accepted){
        radioData->SetTitle(ui->eRadioTitle->text());
        QString tPlaylist=ui->ePlaylist->text();
        if (!tPlaylist.contains("://") &&tPlaylist!=""){
            tPlaylist=QString("http://")+tPlaylist;
        }
        tPlaylist=tPlaylist.trimmed();
        radioData->SetPlaylist(tPlaylist);

        radioData->GetURLs()->clear();
        QStringList tURLs=ui->eStreams->toPlainText().split("\n");
        for (int i=0; i<tURLs.count() ; i++){
            if (!tURLs.at(i).contains("://") && tURLs.at(i)!=""){
                tURLs.replace(i,QString("http://")+tURLs.at(i));
            }
            tURLs.replace(i,tURLs.at(i).trimmed());
            radioData->GetURLs()->append(tURLs.at(i));
        }
    }
    return (tResult);
}

void RadioEdit::on_pbLoadURLs_clicked()
{
    qDebug()<<"RadioEdit::on_pbLoadURLs_clicked";

    PlaylistCL* tPlaylist=new PlaylistCL;
    RadioCL* newRadio=tPlaylist->Parse(QUrl(ui->ePlaylist->text()));
    if (newRadio){
        if (ui->eRadioTitle->text()=="")
            ui->eRadioTitle->setText(newRadio->GetTitle());
        for (int i=0; i<newRadio->GetURLs()->count(); i++){
            ui->eStreams->append(newRadio->GetURL(i));
        }
        delete newRadio;
    }
    delete tPlaylist;

}

void RadioEdit::fillURLs(bool pClear){
    if (pClear)
        ui->eStreams->clear();
    for (int i=0; i<radioData->GetURLs()->count(); i++){
        ui->eStreams->append(radioData->GetURL(i));
    }
}

void RadioEdit::on_ePlaylist_textChanged(const QString &arg1)
{
    if (arg1!="")
        ui->pbLoadURLs->setEnabled(true);
    else
        ui->pbLoadURLs->setEnabled(false);
}



void RadioEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mLastMousePosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void RadioEdit::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - mLastMousePosition);
        event->accept();
    }
}

void RadioEdit::setShadow(QPushButton* pButton){
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(2); //Adjust accordingly
    effect->setOffset(3,3); //Adjust accordingly
    pButton->setGraphicsEffect(effect);
}
