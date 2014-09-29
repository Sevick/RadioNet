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

private slots:

    void on_pbLoadURLs_clicked();

    void on_ePlaylist_textChanged(const QString &arg1);

    void on_pbClearURLs_clicked();

private:
    Ui::RadioEdit *ui;
    RadioCL* radioData;

    void fillURLs(bool pClear = true);
};

#endif // RADIOEDIT_H
