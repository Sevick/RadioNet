#ifndef MYGLOBALHANDLER_H
#define MYGLOBALHANDLER_H

#include <QObject>
#include <QElapsedTimer>

class MyGlobalHandler : public QObject
{
    Q_OBJECT
public:
    void Error(const char* es, bool bModal=true);
    void UpdTrackInfo(QString pTrackInfo);
    void UpdRadioInfo(QString pRadioTitle, QString pRadioWeb, QString pGenre, QString pBPS);
    void SyncLost();
    void PlaybackStarts();

    bool   IsConnecting;
    bool   TriggerPrebuf;
    QElapsedTimer  m_tmr;

signals:
    void SendError(const char* es, int ec, bool bModal);    
    void SendUpdTrackInfo(QString pTrackInfo);
    void SendUpdRadioInfo(QString pRadioTitle, QString pRadioWeb, QString pGenre, QString pBPS);
    void SendSyncLost();
    void SendPlaybackStarts();

};

extern MyGlobalHandler mgh;

#endif // MYGLOBALHANDLER_H
