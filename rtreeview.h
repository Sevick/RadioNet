#ifndef RTREEVIEW_H
#define RTREEVIEW_H

#include <QTreeView>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDebug>
#include <QMimeData>

#include "radiocl.h"
#include "treesortfilterproxymodel.h"
#include "treemodel.h"

class RTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit RTreeView(QWidget *parent = 0);
    void SetProxyModel(QAbstractItemModel* pNewModel);

signals:
    void changed(const QMimeData *mimeData = 0);
    void AddStationSig(QModelIndex pParent,RadioCL* pRadio);
    void treeSelectionChanged(const QModelIndex & pCurrent, const QModelIndex & pPrevious);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

    void currentChanged(const QModelIndex & pCurrent, const QModelIndex & pPrevious);
public slots:


private:
    TreeSortFilterProxyModel* proxyModel;
    TreeModel* sourceModel;


    RadioCL* ParsePls(QString pFileName);
    RadioCL* ParseM3U(QString pFileName);
    void AddRadio(RadioCL* pNewRadio,QModelIndex pParent);
    QString GetTempFilePath();
};

#endif // RTREEVIEW_H
