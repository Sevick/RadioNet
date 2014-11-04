#include "graphicsbutton.h"
#include "mainwindow.h"
#include <QPainter>

graphicsButton::graphicsButton(QWidget *parent) :
    QPushButton(parent)
{
    SpecImage=NULL;
    //MyParent = parent;
    // set text color
    QPalette pal =  this->palette();
    pal.setColor(QPalette::ButtonText, Qt::white);
    this->setPalette(pal);
    IsSpecBeingDrawn = false;
}

void graphicsButton::paintEvent ( QPaintEvent * event)
{
        if (!SpecImage)
            return;
        //QPushButton::paintEvent(event);

        if( IsSpecBeingDrawn ) return;  // prevent random deadlock (?)
        QPainter p(this);
        Spec = QPixmap::fromImage(*SpecImage) ;
        QRect r = rect();

        p.drawPixmap(r, Spec );
        if( !MyText.isEmpty() )
        {
            //p.drawText(r.left(), r.top()+3, r.width(), p.fontMetrics().height(),
            p.drawText(r.left(), r.top(), r.width(), r.height(),
                       Qt::AlignHCenter | Qt::AlignVCenter , MyText);
        }
        p.end();

}
