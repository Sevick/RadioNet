#ifndef RADIOCL_H
#define RADIOCL_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QUrl>


class RadioCL : public QObject
{
private:
    QString Title;
    QStringList *URLs;

public:

    QString Playlist;

    RadioCL();
    RadioCL(QString pTitle,QStringList* pURLs);
    RadioCL(QString pTitle,QString pPlaylist,QStringList* pURLs);

    QString GetTitle(){return Title;};
    QStringList* GetURLs(){ return URLs; };
    QString GetURL(int pNum);
    QString GetPlaylist(){ return Playlist; };
    void SetTitle(QString pTitle){ Title=pTitle; };
    void SetPlaylist(QString pPlaylist){ Playlist=pPlaylist;};
    void SetURLs(QStringList* pURLs);
    void AddURLs(QStringList* pURLs);
};

#endif // RADIOCL_H
