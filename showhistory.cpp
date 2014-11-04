#include <QFile>
#include <QDebug>
#include <QSize>
#include <QSettings>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include "showhistory.h"
#include "ui_showhistory.h"
#include "defs.h"

ShowHistory::ShowHistory(QString pFileName,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowHistory)
{
    ui->setupUi(this);

    readSettings();

    historyFile=new QFile(pFileName);
    historyFile->open(QFile::ReadOnly | QFile::Text);

    if (!historyFile->seek(historyFile->size())){
        qDebug()<<"Error positioning history file its end";
    }

    readNextBlock();

    QScrollBar* tEditScroll=ui->teHistory->verticalScrollBar();
    connect((QObject*) ui->teHistory->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(onEditScroll(int)));
}

ShowHistory::~ShowHistory()
{
    historyFile->close();
    delete historyFile;
    delete ui;
}

void ShowHistory::reject()
{
    writeSettings();
    QDialog::reject();
}

void ShowHistory::onEditScroll(int pScrollPos){
    //qDebug()<<"Scroll value="<<pScrollPos;
    if (pScrollPos<HISTORY_AULOAD_SCROLL){
        readNextBlock();
    }
}

void ShowHistory::readNextBlock(){
    qDebug()<<"Loading next block of history";
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (historyFile->pos()>0){
        qint64 readSize=(historyFile->pos()>HISTORY_READBLOCK_SIZE) ? HISTORY_READBLOCK_SIZE : historyFile->pos();
        qint64 pBlockStart=historyFile->pos()-readSize;
        qDebug()<<"readSize="<<readSize;

        if (!historyFile->seek(historyFile->pos()-readSize)){
            qDebug()<<"Error positioning in history file";
        }
        else{
            QTextCursor tHistCursor = ui->teHistory->textCursor();
            tHistCursor.setPosition(0); // Bring cursor to the beginning of the text edit
            ui->teHistory->setTextCursor(tHistCursor);

            ui->teHistory->insertPlainText(historyFile->read(readSize));
            tHistCursor.setPosition(readSize);
            ui->teHistory->setTextCursor(tHistCursor);
            historyFile->seek(pBlockStart);
        }
    }

    QApplication::restoreOverrideCursor();
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

void ShowHistory::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mLastMousePosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void ShowHistory::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - mLastMousePosition);
        event->accept();
    }
}


