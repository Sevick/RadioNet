#include <QDebug>
#include "radioedit.h"
#include "playlistcl.h"
#include "ui_radioedit.h"

RadioEdit::RadioEdit(RadioCL* pRadio,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RadioEdit)
{

    radioData=pRadio;
    ui->setupUi(this);

    ui->eRadioTitle->setText(radioData->GetTitle());
    ui->ePlaylist->setText(radioData->GetPlaylist());
    if (radioData->GetPlaylist()!="")
        ui->pbLoadURLs->setEnabled(true);
    else
        ui->pbLoadURLs->setEnabled(false);

    fillURLs();
}

RadioEdit::~RadioEdit()
{
    delete ui;
}

int RadioEdit::exec(){
    int tResult=QDialog::exec();
    if (tResult==QDialog::Accepted){
        radioData->SetTitle(ui->eRadioTitle->text());        
        radioData->SetPlaylist(ui->ePlaylist->text());
        radioData->GetURLs()->clear();
        QStringList tURLs=ui->eStreams->toPlainText().split("\n");
        for (int i=0; i<tURLs.count() ; i++){
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
    for (int i=0; i<newRadio->GetURLs()->count(); i++){
        ui->eStreams->append(newRadio->GetURL(i));
    }
    delete tPlaylist;
    delete newRadio;
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

void RadioEdit::on_pbClearURLs_clicked()
{
    radioData->GetURLs()->clear();
    fillURLs();
}


