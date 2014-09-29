#include <QFile>
#include <QDebug>
#include <QSize>
#include <QSettings>
#include "showhistory.h"
#include "ui_showhistory.h"
#include "defs.h"

ShowHistory::ShowHistory(QString pFileName,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowHistory)
{
    ui->setupUi(this);

    readSettings();

    QFile file(pFileName);
    file.open(QFile::ReadOnly | QFile::Text);
    ui->teHistory->append(file.readAll());
    file.close();
/*
    while (!file.atEnd() ) {
        file.readLine(buff, sizeof(buff));
        textEdit->appendQString(file.readAll()));
    }
*/

}

ShowHistory::~ShowHistory()
{
    delete ui;
}

void ShowHistory::reject()
{
    writeSettings();
    QDialog::reject();
}



void ShowHistory::OnHistoryChanged(QString pNewLine){
    //qDebug()<<"OnHistoryChanged";
    ui->teHistory->append(pNewLine);
}


void ShowHistory::readSettings()
{
    QSettings settings("FBytes.com", PLAYER_NAME);
    int winSizeX=settings.value("HistWinSizeX",this->size().width()).toInt();
    int winSizeY=settings.value("HistWinSizeY",this->size().height()).toInt();

    this->resize(QSize(winSizeX,winSizeY));
}


void ShowHistory::writeSettings(){
    QSettings settings("FBytes.com", PLAYER_NAME);
    settings.setValue("HistWinSizeX",this->size().width());
    settings.setValue("HistWinSizeY",this->size().height());
}


