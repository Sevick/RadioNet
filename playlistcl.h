#ifndef PLAYLISTCL_H
#define PLAYLISTCL_H

#include <QObject>
#include <QUrl>
#include "radiocl.h"

class PlaylistCL : public QObject
{
public:
    PlaylistCL();

    RadioCL* Parse(QUrl pURL);

private:

    RadioCL* ParsePls(QString pFileName);
    RadioCL* ParseM3U(QString pFileName);
    QString  GetTempFilePath();
};

#endif // PLAYLISTCL_H
