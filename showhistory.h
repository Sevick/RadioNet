#ifndef SHOWHISTORY_H
#define SHOWHISTORY_H

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>

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

    QFile* historyFile;
    qint64 historySize;

    void reject();

    QPoint mLastMousePosition;


public slots:
    void OnHistoryChanged(QString pNewLine);

protected:
    void readNextBlock();
    void readSettings();
    void writeSettings();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void onEditScroll(int pScrollPos);
};

#endif // SHOWHISTORY_H
