#include <QDebug>
#include <QPainter>
#include <QPaintEngine>
#include "visualwidgetcl.h"

VisualWidgetCL::VisualWidgetCL(QWidget *parent) :
    QLabel(parent)
{
}


HDC VisualWidgetCL::GetWidgetDC(){
    return NULL;
};

/*
void VisualWidgetCL::paintEvent(QPaintEvent  *event){
    qDebug()<<"VisualWidgetCL::paintEven";
    QPainter painter(this);
    //painter.device()->paintEngine()->getDC();
    //HDC thdc=this->getDC();
}
*/
/*
void VisualWidgetCL::OnDraw(CDC* pDC){
    qDebug()<<"VisualWidgetCL::OnDraw";
}
*/
