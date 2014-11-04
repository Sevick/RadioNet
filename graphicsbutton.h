#ifndef GRAPHICSBUTTON_H
#define GRAPHICSBUTTON_H

#include <QPushButton>

class MainWindow;

class graphicsButton : public QPushButton
{
    Q_OBJECT
public:
    QPixmap  Spec;
    QString  MyText;
    bool     IsSpecBeingDrawn ;
    explicit graphicsButton(QWidget *parent = 0);

    //QWidget* getParent(){ return MyParent;};
    //void setParent(QWidget* pNewParent){ MyParent=pNewParent; };
    void setSpec(QImage* pSpec){ SpecImage=pSpec; };
    void setIsDrawn(bool pDrawn){ IsSpecBeingDrawn=pDrawn; };
    bool getIsDrawn(){ return IsSpecBeingDrawn; };

signals:

public slots:

protected:
    //QWidget *MyParent;
    QImage* SpecImage;
    void paintEvent( QPaintEvent * event ) ;

    friend class MainWindow;
};

#endif // GRAPHICSBUTTON_H
