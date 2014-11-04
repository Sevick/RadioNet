#include "radiodialog.h"

RadioDialog::RadioDialog(QWidget *parent) :
    QDialog(parent)
{
}


void RadioDialog::setShadow(QPushButton* pButton){
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(2); //Adjust accordingly
    effect->setOffset(3,3); //Adjust accordingly
    pButton->setGraphicsEffect(effect);
}
