/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "treeitem.h"
#include "treemodel.h"
#include "defs.h"

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{

    QString radioListFile=GetAppDataDir();
    rFile.setFileName(radioListFile);


    if (!rFile.open(QIODevice::ReadOnly)){        
        qDebug()<<"Unable to open "<<radioListFile;
        if (!rFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
            qDebug()<<"Unable to create "<<radioListFile;
        }
        else{
            rFile.close();
        }
        rFile.setFileName(":/radiolist.txt");
        rFile.open(QIODevice::ReadOnly);
    }
    QStringList headers;
    headers << tr("Title");// << tr("kBPS");

    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;
    rootItem = new TreeItem(rootData,NULL);

    setupModelData(QString(rFile.readAll()).split(QString("\n")), rootItem);
    rFile.close();
    if (rFile.fileName()!=radioListFile){
        rFile.setFileName(radioListFile); // needed when open list from resource on first run
        saveToDisk();
    }


}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}


QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem *item = getItem(index);

    return item->data(index.column());
}


Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if (   ((TreeItem*)((TreeModel*)index.model())->getItem(index))->getIsFolder() )
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    else
        return Qt::ItemIsEditable | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}


QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}


QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}


bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount(),NULL);
    endInsertRows();

    return success;
}


//! [7]
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}
//! [7]

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

//! [8]
int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}
//! [8]

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
{
    QList<TreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;
    int childCount =0;

    qDebug()<<"Setting up model data";

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].mid(position, 1) != " ")
                break;
            ++position;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            //QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QStringList columnStrings = lineData.split("\t");
            QVector<QVariant> columnData;
            //QString curRadioURL;


            for (int column = 0; column < columnStrings.count(); ++column)
                    columnData << columnStrings[column];
            if (columnStrings.count()>1)
                    childCount++;

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }





            // Append a new item to the current parent's list of children.
            TreeItem *parent = parents.last();

            QString tPlaylist;
            if (columnStrings.count()>1){
                tPlaylist=columnStrings[1];
            }


            QStringList* tURLs=new QStringList;
            for (int tCol=2; tCol<columnStrings.count(); tCol++){
                tURLs->append(columnStrings[tCol]);
            }


            /*
            QStringList* tURLs=new QStringList;
            for (int tCol=1; tCol<columnStrings.count(); tCol++){
                tURLs->append(columnStrings[tCol]);
            }
            */

            RadioCL* tRadioData=new RadioCL(columnData[0].toString(),tPlaylist,tURLs);
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount(),tRadioData);
            parent->unsetRadio();


            parent->child(parent->childCount() - 1)->setData(1, columnData[1]);
        }

        ++number;
    }
    qDebug()<<"Model data setup completed";
}

void TreeModel::saveToDisk(){

    qDebug()<<"Saving model to disk";
    if (!rFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug()<<"Unable to open "<<rFile.fileName();
        return;
    }

    for (int tRow=0; tRow<rowCount(); tRow++){
        TreeItem* tCurItem=getItem(index(tRow,0));
        saveBranchToDisk(tCurItem,"");
    }

    rFile.close();
}


void TreeModel::saveBranchToDisk(TreeItem* pRoot,QString pIndent){

    QTextStream rFileWriteStream(&rFile);

    if (pRoot->getRadio()==NULL){
        // this is a folder
        //qDebug()<<"Item with NULL Radio";
        rFileWriteStream<<pIndent<<pRoot->data(0).toString()<<endl;
    }
    else{
        rFileWriteStream<<pIndent<<pRoot->getRadio()->GetTitle()<<"\t";
        rFileWriteStream<<pRoot->getRadio()->GetPlaylist()<<"\t";
        for (int i=0; i<pRoot->getRadio()->GetURLs()->count(); i++){
            rFileWriteStream<<pRoot->getRadio()->GetURL(i)<<"\t";
        }
        rFileWriteStream<<endl;
    }

    for (int tCurChild=0; tCurChild< pRoot->childCount(); tCurChild++){
        saveBranchToDisk(pRoot->child(tCurChild),pIndent+" ");
    }
}


Qt::DropActions TreeModel::supportedDropActions() const{
    qDebug()<<"supportedDropActions";
    return Qt::CopyAction | Qt::MoveAction;
}


QString TreeModel::GetAppDataDir(){
    //QDir tDir;
    //qDebug()<<"GetAppDataDir: "<<tDir.absoluteFilePath("radiolist.txt");
    //return (tDir.absoluteFilePath("radiolist.txt"));

    return(AppPath+"radiolist.txt");
}


