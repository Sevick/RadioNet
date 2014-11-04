#ifndef RADIOEDIT_H
#define RADIOEDIT_H

#include <QDialog>
#include "radiocl.h"

namespace Ui {
class RadioEdit;
}

class RadioEdit : public QDialog
{
    Q_OBJECT

public:
    explicit RadioEdit(RadioCL* pRadio,QWidget *parent = 0);
    ~RadioEdit();

    int exec();
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
private slots:

    void on_pbLoadURLs_clicked();

    void on_ePlaylist_textChanged(const QString &arg1);



private:
    Ui::RadioEdit *ui;
    RadioCL* radioData;

    bool mMoving;
    QPoint mLastMousePosition;

    void fillURLs(bool pClear = true);
    void setShadow(QPushButton* pButton);
};

#endif // RADIOEDIT_H
