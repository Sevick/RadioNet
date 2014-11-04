#ifndef VISWINCL_H
#define VISWINCL_H

#include <QDialog>

namespace Ui {
class VisWinCL;
}

class VisWinCL : public QDialog
{
    Q_OBJECT

public:
    explicit VisWinCL(QWidget *parent = 0);
    ~VisWinCL();

private:
    Ui::VisWinCL *ui;
};

#endif // VISWINCL_H
