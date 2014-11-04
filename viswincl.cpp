#include "viswincl.h"
#include "ui_viswincl.h"

VisWinCL::VisWinCL(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VisWinCL)
{
    ui->setupUi(this);
}

VisWinCL::~VisWinCL()
{
    delete ui;
}
