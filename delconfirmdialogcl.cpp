#include <QGraphicsDropShadowEffect>
#include "delconfirmdialogcl.h"
#include "ui_delconfirmdialogcl.h"

DelConfirmDialogCL::DelConfirmDialogCL(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DelConfirmDialogCL)
{
    ui->setupUi(this);
    setShadow(ui->pbOk);
    setShadow(ui->pbCancel);
    //this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
}

DelConfirmDialogCL::~DelConfirmDialogCL()
{
    delete ui;
}

void DelConfirmDialogCL::setRemovingItemStr(QString pStr){
    ui->lItemTitle->setText(pStr);

}

void DelConfirmDialogCL::setShadow(QPushButton* pButton){
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(2); //Adjust accordingly
    effect->setOffset(3,3); //Adjust accordingly
    pButton->setGraphicsEffect(effect);
}

void DelConfirmDialogCL::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mLastMousePosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void DelConfirmDialogCL::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - mLastMousePosition);
        event->accept();
    }
}

void DelConfirmDialogCL::on_pbOk_clicked()
{
    accept();
}

void DelConfirmDialogCL::on_pbCancel_clicked()
{
    reject();
}
