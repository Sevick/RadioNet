#ifndef QTREESORTFILTERPROXYMODEL_H
#define QTREESORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class TreeSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    TreeSortFilterProxyModel();

    bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;
};

#endif // QTREESORTFILTERPROXYMODEL_H
