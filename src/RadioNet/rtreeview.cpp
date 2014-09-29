#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDir>
#include "rtreeview.h"
#include "treemodel.h"
#include "treeitem.h"
#include "playlistcl.h"

RTreeView::RTreeView(QWidget *parent) :
    QTreeView(parent)
{
    setAcceptDrops(true);
}

void RTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
    //connect(this, SIGNAL(changed(const QMimeData*)),this, SLOT(updateFormatsTable(const QMimeData*)));
}

void RTreeView::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    qDebug()<<"Drop event urls="<<mimeData->urls();
    //qDebug()<<"Drop event html="<<mimeData->html();

    QList<QUrl> urlList = mimeData->urls();
    QString text;
        for (int i = 0; i < urlList.size() && i < 32; ++i) {
            QString url = urlList.at(i).toString();
            text += url + QString("\n");
        }
    //qDebug()<<text;
    QStringList droppedItems=text.split("\n");

    QModelIndex droppedIndex=indexAt(event->pos());
    QModelIndex origIndex=proxyModel->mapToSource(droppedIndex);

    //TreeItem* droppedItem=((TreeModel*)droppedIndex.model())->getItem(droppedIndex);
    TreeItem* droppedItem=sourceModel->getItem(origIndex);
    qDebug()<<"Dropped @"<<droppedItem->data(0);

    QModelIndex titleIndex=droppedIndex.model()->index(droppedIndex.row(),0,droppedIndex.parent());

    bool tAllItemsAcceptes=true;
    for (int i=0; i<droppedItems.count(); i++){
        if (urlList.at(i).toString()=="")
            continue;


        PlaylistCL* tCurList=new PlaylistCL;
        RadioCL* newRadio=tCurList->Parse(urlList.at(i));
        if (newRadio==NULL)
            tAllItemsAcceptes=false;
        else
           AddRadio(newRadio,droppedIndex);
        delete tCurList;
        delete newRadio;
/*


        if (urlList.at(i).isLocalFile()){
            QString tFileName=urlList.at(i).path().mid(1,urlList.at(i).path().length()-1);
            QStringList tFileNameParts=tFileName.split(".");

            QFile drFile(tFileName);
            if (!drFile.open(QIODevice::ReadOnly)){
                qDebug()<<"Unable to open"<<drFile.fileName();
            }
            else{
                drFile.close();
                if ( tFileNameParts.last()=="pls" || tFileNameParts.last()=="m3u"){
                     qDebug()<<"Dropped content is acceptable";
                     if (tFileNameParts.last()=="pls"){
                         RadioCL* newRadio=ParsePls(tFileName);
                         if (newRadio==NULL)
                             tAllItemsAcceptes=false;
                         else
                            AddRadio(newRadio,droppedIndex);
                     }
                     if (tFileNameParts.last()=="m3u"){
                         RadioCL* newRadio=ParseM3U(tFileName);
                         if (newRadio==NULL)
                             tAllItemsAcceptes=false;
                         else
                            AddRadio(newRadio,droppedIndex);
                     }
                     event->dropAction();
                }
                else{
                     qDebug()<<"Dropped content is unacceptable";
                     event->acceptProposedAction();
                }
            }
        }
        else{
            // remote url
            qDebug()<<"Getting playlist from "<<urlList.at(i).url();
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);

            QNetworkRequest req;
            req.setUrl(urlList.at(i));
            QNetworkReply *tRemotePlaylist=manager->get(req);

            QEventLoop loop;
            connect(manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
            loop.exec();
            QString tPlaylist=QString(tRemotePlaylist->readAll());
            qDebug()<<"Got playlist as:"<<tPlaylist;
            qDebug()<<"Headers are:"<<tRemotePlaylist->rawHeaderList();
            qDebug()<<"Content-Disposition:"<<tRemotePlaylist->rawHeader("Content-Disposition");

            QFile tTempFile(GetTempFilePath());
            if (!tTempFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
                qDebug()<<"Unable to open"<<tTempFile.fileName();
            }
            int tFileNamePos=tPlaylist.indexOf("filename=",0,Qt::CaseInsensitive);
            int tFileNameEndPos=tPlaylist.indexOf("\n",tFileNamePos,Qt::CaseInsensitive);
            int tFileBeginningPos=tPlaylist.indexOf("\n\n",tFileNameEndPos);

            QString tFileBody;
            if (tFileNamePos!=-1){
                if (tFileNameEndPos!=-1 && tFileBeginningPos!=-1){
                    QString tPlaylistName=tPlaylist.mid(tFileNamePos+9,tFileNameEndPos-9-tFileNamePos);
                    qDebug()<<"Playlist filename="<<tPlaylistName;
                    tFileBody=tPlaylist.right(tPlaylist.length()-tFileBeginningPos-2);
                }
                else{
                    qDebug()<<"Error in playlist";
                }
            }
            else{
                tFileBody=tPlaylist;
            }
            qDebug()<<"FileBody:"<<tFileBody;
            tTempFile.write(tFileBody.toLocal8Bit(),tFileBody.length());
            tTempFile.close();

            RadioCL* newRadio=NULL;
            if (tFileBody.indexOf("[playlist]",Qt::CaseInsensitive)!=-1){
                newRadio=ParsePls(tTempFile.fileName());
            }
            else{
                newRadio=ParseM3U(tTempFile.fileName());
            }
            newRadio->SetPlaylist(urlList.at(i).url());
            if (newRadio->GetTitle()=="")
                newRadio->SetTitle("New radio");
            AddRadio(newRadio,droppedIndex);
        }
        */
    }

    qDebug()<<"==>"<<titleIndex.data().toString();
    for (int i=0; i<droppedItems.count(); i++){
        qDebug()<<droppedItems.at(i);
    }

    event->acceptProposedAction();
}

void RTreeView::SetProxyModel(QAbstractItemModel* pNewModel){
    sourceModel=(TreeModel*) model();
    proxyModel=(TreeSortFilterProxyModel*)pNewModel;
    setModel(pNewModel);
}

RadioCL* RTreeView::ParsePls(QString pFileName){

    QSettings tPlsSettings(pFileName,QSettings::IniFormat);

    int tEntries=tPlsSettings.value("playlist/NumberOfEntries").toInt();

    QString radioTitle="";

    QStringList* radioURLs=new QStringList;

    for (int i=0; i<tEntries; i++){
        if (radioTitle==""){
            radioTitle=tPlsSettings.value("playlist/Title"+QString::number(i+1)).toString();
        }
        radioURLs->append(tPlsSettings.value("playlist/File"+QString::number(i+1)).toString());
    }

    qDebug()<<"Number of entries:"<<tEntries;
    qDebug()<<"Title="<<radioTitle;
    qDebug()<<"URLs="<<radioURLs;

    RadioCL* tNewRadio=new RadioCL(radioTitle,pFileName,radioURLs);

    return tNewRadio;
}

RadioCL* RTreeView::ParseM3U(QString pFileName){
    QFile tListFile(pFileName);
    if (!tListFile.open(QIODevice::ReadOnly)){
            qDebug()<<"Unable to open "<<pFileName;
            return NULL;
    }

    QStringList* radioURLs=new QStringList(QString(tListFile.readAll()).split(QString("\n")));
    qDebug()<<radioURLs;
    QString radioTitle="New radio";
    RadioCL* tNewRadio=new RadioCL(radioTitle,pFileName,radioURLs);

    return tNewRadio;
}

void RTreeView::AddRadio(RadioCL* pNewRadio,QModelIndex pParent){

    qDebug()<<"Adding radio: ";
    emit AddStationSig(pParent,pNewRadio);
}


void RTreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    qDebug()<<"dragEventLeave";
    event->accept();
}

void RTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug()<<"dragEnterEvent";

    event->acceptProposedAction();
    emit changed(event->mimeData());
}

QString RTreeView::GetTempFilePath(){
    QDir tDir;
    qDebug()<<"GetTempFilePath: "<<tDir.absoluteFilePath(PLAYLIST_TEMP);
    return (tDir.absoluteFilePath(PLAYLIST_TEMP));
}
