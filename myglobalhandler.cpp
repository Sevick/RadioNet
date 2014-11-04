#include "myglobalhandler.h"
#include "bass/bass.h"

void MyGlobalHandler::Error(const char* es, bool bModal)
{
    emit SendError(es, BASS_ErrorGetCode(), bModal);
}


 void MyGlobalHandler::UpdTrackInfo(QString pTrackInfo){
     emit SendUpdTrackInfo(pTrackInfo);
 }

 void MyGlobalHandler::PlaybackStarts(){
     emit SendPlaybackStarts();
 }

 void MyGlobalHandler::SyncLost(){
     emit SendSyncLost();
 }


void MyGlobalHandler::UpdRadioInfo(QString pRadioTitle, QString pRadioWeb, QString pGenre, QString pBPS){
    emit SendUpdRadioInfo(pRadioTitle,pRadioWeb, pGenre, pBPS);
}


void MyGlobalHandler::ClickRecord(){
    emit ClickRecord();
}
