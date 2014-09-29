#include <QFile>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QEventLoop>
#include <QSettings>

#include "playlistcl.h"
#include "defs.h"

PlaylistCL::PlaylistCL()
{
}


RadioCL* PlaylistCL::Parse(QUrl pURL){

    RadioCL* newRadio=NULL;

    if (pURL.isLocalFile()){
        QString tFileName=pURL.path().mid(1,pURL.path().length()-1);
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
                     newRadio=ParsePls(tFileName);
                 }
                 if (tFileNameParts.last()=="m3u"){
                     newRadio=ParseM3U(tFileName);
                 }
            }
            else{
                 qDebug()<<"Parse works with pls and m3u only";
            }
        }
    }
    else{
        // remote url
        qDebug()<<"Getting playlist from "<<pURL.url();
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);

        if (Proxy!=""){
            manager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,Proxy,ProxyPort.toInt(),ProxyUser,ProxyPass));
        }


        QNetworkRequest req;
        req.setUrl(pURL);
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

        if (tFileBody.indexOf("[playlist]",Qt::CaseInsensitive)!=-1){
            newRadio=ParsePls(tTempFile.fileName());
        }
        else{
            newRadio=ParseM3U(tTempFile.fileName());
        }
        newRadio->SetPlaylist(pURL.url());
    }

    return(newRadio);
}


RadioCL* PlaylistCL::ParsePls(QString pFileName){

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

RadioCL* PlaylistCL::ParseM3U(QString pFileName){
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


QString PlaylistCL::GetTempFilePath(){
    return (AppPath+PLAYLIST_TEMP);
}
