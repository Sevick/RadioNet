#ifndef RADIODIALOG_H
#define RADIODIALOG_H

#include <QDialog>

class RadioDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RadioDialog(QWidget *parent = 0);

signals:

public slots:

protected:
    void setShadow(QPushButton* pButton);

};

#endif // RADIODIALOG_H
