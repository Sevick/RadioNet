#include "radiocl.h"

RadioCL::RadioCL()
{
   Title="";
   Playlist="";
   URLs=NULL;
}

RadioCL::RadioCL(QString pTitle,QStringList* pURLs){

    Title=pTitle;
    URLs=pURLs;
}

RadioCL::RadioCL(QString pTitle,QString pPlaylist,QStringList* pURLs){
    Playlist=pPlaylist;
    Title=pTitle;
    URLs=pURLs;
}

QString RadioCL::GetURL(int pNum){
    if (URLs->count()>pNum)
        return URLs->at(pNum);
    else
        return "";
};


void RadioCL::SetURLs(QStringList* pURLs){
    if (URLs)
        delete URLs;
    URLs=pURLs;
}


void RadioCL::AddURLs(QStringList* pURLs){
    for (int i=0;i<pURLs->count();i++){
        URLs->append(pURLs->at(i));
    }
}
