#ifndef DELCONFIRMDIALOGCL_H
#define DELCONFIRMDIALOGCL_H

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>

namespace Ui {
class DelConfirmDialogCL;
}

class DelConfirmDialogCL : public QDialog
{
    Q_OBJECT

public:
    explicit DelConfirmDialogCL(QWidget *parent = 0);
    ~DelConfirmDialogCL();

    void setRemovingItemStr(QString pStr);

private:
    Ui::DelConfirmDialogCL *ui;
    QPoint mLastMousePosition;

protected:
    void setShadow(QPushButton* pButton);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
private slots:
    void on_pbOk_clicked();
    void on_pbCancel_clicked();
};

#endif // DELCONFIRMDIALOGCL_H
