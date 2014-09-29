#ifndef SHOWHISTORY_H
#define SHOWHISTORY_H

#include <QDialog>

namespace Ui {
class ShowHistory;
}

class ShowHistory : public QDialog
{
    Q_OBJECT

public:
    explicit ShowHistory(QString pFileName,QWidget *parent = 0);
    ~ShowHistory();

private:
    Ui::ShowHistory *ui;

    void readSettings();
    void writeSettings();
    void reject();


public slots:
    void OnHistoryChanged(QString pNewLine);

};

#endif // SHOWHISTORY_H
