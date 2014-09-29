 
#ifndef IN_NETRADIO
#include "MF_nR_Bridge.h"
#endif

#include <QApplication>
#include <QInputDialog>
#include <QProcess>
#include <QComboBox>
#include <QDebug>
#include <QTime>
#include "mainwindow.h"
//#include "graphicsbutton.h"
#include "MyThread.h"
#include "myglobalhandler.h"
#include "ui_mainwindow.h"
#include "defs.h"



#ifdef IN_MANYFRACTALS
#define _CRT_SECURE_NO_WARNINGS
#pragma _CRT_SECURE_NO_WARNINGS
#endif

 
#define WndShow(wname,bVis)  {wname->setVisible(bVis);}


char proxy[100]=""; // proxy server
bool bUseProxy = false;

// encoder command-lines and output files
#define nCmds   4
const char *commands[nCmds-2]={
        "oggenc -o \"%s\" -",                // oggenc (OGG)
        "lame --preset standard --tt \"%s\" --ta \"%s\" --tc \"netRadio by Zac-Labs\" - \"%s\""
        //
        // --tt (title) --ta (artist)  //  lame (MP3) "lame --alt-preset standard - %s_%s"
};
const char *files[nCmds]={"bassOGG","bassMP3","bassWAV","bassMP4"}; // OGG,MP3,ACM,AAC
const char *exts[nCmds]={".ogg",".mp3",".wav",".mp4"};


QString			Title, Song, Artist;

#ifdef IN_MANYFRACTALS
MainWindow*             pMainWin = NULL;
QApplication*           pApp = NULL;
#endif

bool bNR_Exists = false;
bool cthread = false;

HSTREAM			chan = 0 ;		// the root (radio) stream
HRECORD			rchan = 0 ;		// recording/encoding channel
HSTREAM         chan2 = 0 ;     // the non-radio stream, for non-radio sources

float			m_volume = 1.0f ;
float			inlevel = 0.0f ;		// input level meter
float			level = 0.0f;
bool			bMute = false ;


// a few globals for net-radio
bool			bUseWASAPI_= false ;
bool			bURL_Success = false ;  // set 'true' after fully buffered AND is Playing only... allows 'add to list' to function

// output mixer channel for net radio
int			outdev=-1;	// output device
HSTREAM			outmixer=0;	// mixer for resampling output
HSTREAM			recmixer=0;     // mixer non-decoding, 16 bit int not FP for mp3 recording

// more globals for Record in netRadio ...
int				input, source,
                                inNetRadio;			// current input source
int				encoder = 0;		// current encoder
HENCODE                 	m_Encoder = 0;		// the actual encoder ... sorry about the m_ ... confusion with object member.
int				m_iRecNum = 0;		// the recorded file number, added to the name
int				m_Station = -1;		// selected radio station vector index


bool bIsEncoding = false ;
bool bVolumeNeedsAdjusting = true ;
bool bInitPostInit = false ;
bool bRestartURL = false;

MyThread tURL;
MyGlobalHandler mgh;

// Error codes returned by BASS_ErrorGetCode
QMap<int, QString>  BassErrorMap;
std::vector<std::string>  RSN;  // Radio Station Names ... RSN
std::vector<std::string> urls;  // Radio Stations URLs

QMap<int,QString> OggQualityMap;

void MainWindow::initOggQualityMap(){
    OggQualityMap.insert(0,"64");
    OggQualityMap.insert(1,"80");
    OggQualityMap.insert(2,"96");
    OggQualityMap.insert(3,"112");
    OggQualityMap.insert(4,"128");
    OggQualityMap.insert(5,"160");
    OggQualityMap.insert(6,"192");
    OggQualityMap.insert(7,"224");
    OggQualityMap.insert(8,"256");
    OggQualityMap.insert(9,"320");
    OggQualityMap.insert(10,"500");
}


bool MainWindow::InitBassErrorMap()
{
    // static bool bHasRunOnce = false;
    bool bSuccess = true;
    // if( bHasRunOnce ) return true;
    try
    {
        BassErrorMap.clear();
        BassErrorMap.insert( BASS_OK ,			_T("all is OK") );
        BassErrorMap.insert( BASS_ERROR_MEM ,		_T("memory error") );
        BassErrorMap.insert( BASS_ERROR_FILEOPEN ,      _T("can't open the file") );
        BassErrorMap.insert( BASS_ERROR_DRIVER ,	_T("can't find a free/valid driver") );
        BassErrorMap.insert( BASS_ERROR_BUFLOST ,	_T("the sample buffer was lost") );
        BassErrorMap.insert( BASS_ERROR_HANDLE ,	_T("invalid handle") );
        BassErrorMap.insert( BASS_ERROR_FORMAT ,	_T("unsupported sample format") );
        BassErrorMap.insert( BASS_ERROR_POSITION ,      _T("invalid position") );
        BassErrorMap.insert( BASS_ERROR_INIT ,		_T("BASS_Init has not been successfully called") );
        BassErrorMap.insert( BASS_ERROR_START ,         _T("BASS_Start has not been successfully called") );
        BassErrorMap.insert( BASS_ERROR_ALREADY ,	_T("already initialized/paused/whatever") );
        BassErrorMap.insert( BASS_ERROR_NOCHAN ,	_T("can't get a free channel") );
        BassErrorMap.insert( BASS_ERROR_ILLTYPE ,	_T("an illegal type was specified") );
        BassErrorMap.insert( BASS_ERROR_ILLPARAM ,      _T("an illegal parameter was specified") );
        BassErrorMap.insert( BASS_ERROR_NO3D ,		_T("no 3D support") );
        BassErrorMap.insert( BASS_ERROR_NOEAX ,         _T("no EAX support") );
        BassErrorMap.insert( BASS_ERROR_DEVICE ,	_T("illegal device number") );
        BassErrorMap.insert( BASS_ERROR_NOPLAY ,	_T("not playing") );
        BassErrorMap.insert( BASS_ERROR_FREQ ,		_T("illegal sample rate") );
        BassErrorMap.insert( BASS_ERROR_NOTFILE ,	_T("the stream is not a file stream") );
        BassErrorMap.insert( BASS_ERROR_NOHW ,		_T("no hardware voices available") );
        BassErrorMap.insert( BASS_ERROR_EMPTY ,         _T("the MOD music has no sequence data") );
        BassErrorMap.insert( BASS_ERROR_NONET ,         _T("no internet connection could be opened") );
        BassErrorMap.insert( BASS_ERROR_CREATE ,	_T("couldn't create the file") );
        BassErrorMap.insert( BASS_ERROR_NOFX ,		_T("effects are not available") );
        BassErrorMap.insert( BASS_ERROR_NOTAVAIL ,      _T("requested data is not available") );
        BassErrorMap.insert( BASS_ERROR_DECODE ,	_T("the channel is a \"decoding channel\"") );
        BassErrorMap.insert( BASS_ERROR_DX ,		_T("a sufficient DirectX version is not installed") );
        BassErrorMap.insert( BASS_ERROR_TIMEOUT ,	_T("connection timedout") );
        BassErrorMap.insert( BASS_ERROR_FILEFORM ,      _T("unsupported file format") );
        BassErrorMap.insert( BASS_ERROR_SPEAKER ,	_T("unavailable speaker") );
        BassErrorMap.insert( BASS_ERROR_VERSION ,	_T("invalid BASS version (used by add-ons)") );
        BassErrorMap.insert( BASS_ERROR_CODEC ,         _T("codec is not available/supported") );
        BassErrorMap.insert( BASS_ERROR_ENDED ,         _T("the channel/file has ended") );
        BassErrorMap.insert( BASS_ERROR_BUSY ,		_T("the device is busy") );
        BassErrorMap.insert( BASS_ERROR_UNKNOWN ,	_T("some other mystery problem") );
    }
    catch(...)
    {
        bSuccess = false;
    }
    // bHasRunOnce = bSuccess;
    return bSuccess;
}



UINT MainWindow::MsgSleep(const int& milsec)
{
    UINT rtv=0;
    QElapsedTimer tmr;
    tmr.start();
    do
    {
        QThread::msleep(min(milsec,15));
        rtv = DoEvents();
        if( rtv == 1 || milsec - tmr.elapsed() < 15 ) break;
    } while( tmr.hasExpired(milsec) );
    tmr.invalidate();
    return rtv;
}

void MainWindow::MsgBox(QString _msg, QString _title, bool bModal, int xPos, int yPos, bool bParent)
{	 
    // To Do: the following line and m_msgbox need improvement ... like making m_msgbox a std::vector<QMessageBox*>
    if( m_msgbox != NULL && m_msgbox->isHidden() ) delete m_msgbox;
    // ... the && isHidden prevents delete if user hasn't 'exited' the previous modeless dialog, but now it's unreferable
    // ... luckily, QT keeps it from being an orphin when the App exits, and 'deletes' it for us, IF the parent is specified!
    m_msgbox = new QMessageBox((bParent)? this : NULL);
    m_msgbox->setText(_msg);
    if( _title.isNull() ) _title = _T(PLAYER_NAME);
    m_msgbox->setWindowTitle(_title);

    if( xPos >= 0 && yPos >= 0 )
        m_msgbox->move(xPos, yPos);
    if( !bModal )
    {
        m_msgbox->setModal(bModal);
        m_msgbox->show();
        m_msgbox->raise();
        m_msgbox->activateWindow();
    }
    else
    {
        m_msgbox->exec();
        delete m_msgbox;
        m_msgbox = NULL ;
    }
}

// display error messages
void MainWindow::Error(const char *es, int ec, bool bModal)
{
    QString mes;
    int BEC = ((ec==-1234)? BASS_ErrorGetCode() : ec );
    QString mys = *BassErrorMap.find(BEC);
    mes.sprintf("\n%s   \n(error code: %d)   \n%s   \n", es, BEC, qPrintable(mys) );
    qDebug()<<"Error: "<<mes;
    MsgBox(mes, _T(PLAYER_NAME), bModal);
}

void MainWindow::on_Error(const char* es, int ec, bool bModal)
{
    Error(es, ec, bModal);
}

DWORD MainWindow::ErrorMsgBox(LPCSTR lpszFunction, DWORD err_code, bool GetError, bool /*ErrCode_is_HRESULT*/)
{
	// handle err_code as an HRESULT error from COM+ ...
//	if( ErrCode_is_HRESULT )
//	{
//		IErrorInfo* pErrInfo = NULL ;
//		_com_error err((HRESULT)err_code, pErrInfo, false);
//                m_ErrorMsg.sprintf(_T("\n %s \n %s \n %s \n  bailout = %x  \n"),lpszFunction, err.Description(), err.ErrorMessage(), err_code);
//		ErrorCSMsgBox();
//		return(err_code);
//	}
#ifdef Q_OS_WIN
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();
    if( !GetError ) dw = err_code;

    FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR) &lpMsgBuf,
                0, NULL );

    QString ErrorMsg;
    ErrorMsg.sprintf("\n %s \n Failed with Error %d: \n %s \n", lpszFunction, (int)dw, (char*)lpMsgBuf);

    MsgBox(ErrorMsg, _T(PLAYER_NAME));

    LocalFree(lpMsgBuf);
    return(dw);
#else
    QString ErrorMsg;
    ErrorMsg.sprintf("\n %s \n Failed with Error %d: \n", lpszFunction, (int)err_code);

    MsgBox(ErrorMsg, _T("netRadio - Error"));

    return ((GetError)? 0x4242 : err_code);
#endif
}


// update stream title from metadata
void DoMeta()
{
    bool bRestart = false ;
    if( !Title.isEmpty() && m_Encoder != 0 ) { // new song title has started ...
        bRestart = true;
    }

    Song.clear();
    Artist.clear();
    const char *meta=BASS_ChannelGetTags(chan,BASS_TAG_META);
    if (meta) { // got Shoutcast metadata
        qDebug()<<"Shoutcast meta="<<meta;
        char *p = (char*)strstr(meta, "StreamTitle='");
        if (p) {
            p = _strdup(p+13);
            strchr(p,';')[-1]=0;
            Title.sprintf(_T("%s"), p);
            int sep = Title.indexOf(QString(" - "));
            Artist = Title.left(sep);
            sep += 3;
            Song = Title.right(Title.length() - sep);
            free(p);
        }
    } else {
        meta=BASS_ChannelGetTags(chan,BASS_TAG_OGG);
        qDebug()<<"Meta="<<meta;
        if (meta) { // got Icecast/OGG tags
            const char *artist=NULL,*title=NULL,*p=meta;
            for (;*p;p+=strlen(p)+1) {
                if (!_strnicmp(p,"artist=",7)) // found the artist
                    artist=p+7;
                if (!_strnicmp(p,"title=",6)) // found the title
                    title=p+6;
            }
            if (artist) {
                Title.sprintf("%s - %s", artist, title);
                Song = title;
                Artist = artist;
            } else if (title) {
                Title.sprintf("%s", title);
                Song = title;
            }
            else
                Title.sprintf(_T("Track %d"), ++m_iRecNum);
        }
        else
            Title.sprintf(_T("Track %d"), ++m_iRecNum);
    }
    mgh.SendUpdTrackInfo(Title);
/*
    mgh.SetLabel4(Title, false);
    if( bRestart )
        mgh.ClickRecord();  // restart with new title
*/
}

void CALLBACK MetaSync(HSYNC /*handle*/, DWORD /*channel*/, DWORD /*data*/, void * /*user*/)
{
    DoMeta();
}

void CALLBACK EndSync(HSYNC /*handle*/, DWORD /*channel*/, DWORD /*data*/, void * /*user*/)
{
    //mgh.SendUpdTrackInfo("Restarting ...");
    qDebug()<<"EndSync";
    mgh.SendSyncLost();
    bRestartURL = true ;
}

void CALLBACK StatusProc(const void *buffer, DWORD length, void * /*user*/)
{
    if (buffer && !length)
    {
        //mgh.SetLabel6((const char*)buffer, false); // display connection status
    }
}

//////////////////////////////////////////////////////////

 void MyThread::run()
 {
     OpenURL();
 }

void MyThread::OpenURL()
{
    cthread = true;
    BASS_StreamFree(chan); // close old stream
    chan = 0;
    ////// FGZ: added
    DWORD  dwFlags;
    if( bUseWASAPI_ ) dwFlags = BASS_STREAM_BLOCK | BASS_STREAM_STATUS | BASS_STREAM_DECODE ;
    else
        dwFlags = BASS_STREAM_BLOCK | BASS_STREAM_STATUS | BASS_STREAM_AUTOFREE ;
    // The following line can take many seconds to complete.
    qDebug()<<"Creating stream";
    chan = BASS_StreamCreateURL(qPrintable(url), 0, dwFlags, StatusProc, 0); // open URL
    mgh.IsConnecting = false;
    mgh.m_tmr.invalidate();


    if (!chan) { // failed to open
        //mgh.SetLabel4("not playing", false);
        Error("\n Can't open the url stream \n");
        qDebug()<<"Can't open the url stream";
        emit ConnectionStartFailed();
    } else{
        mgh.TriggerPrebuf = true;
        qDebug()<<"Stream opened";
        mgh.PlaybackStarts();
    }
    cthread = false;
}

void MyThread::Error(const char* es, bool bModal)
{
    emit ShowError(es, BASS_ErrorGetCode(), bModal);
}


////////////////////////////////////////////////////////////////////////////////////

// WASAPI output processing function
DWORD CALLBACK OutWasapiProc(void *buffer, DWORD length, void * /*user*/)
{
    // this 'ties' wasapi into the mixer ...
    int c = BASS_ChannelGetData(outmixer, buffer, length);
    return c;
}

// Note: Be careful if ever using BASS_ChannelGetData and/or BASS_StreamPutData in more than one place.
//      As these callbacks happen in different threads than the MainWindow, and as a result
//      Deadlock can and will occur! The use of BASS_ChannelLock becomes necessary to prevent it.

// WASAPI input processing function
DWORD CALLBACK InWasapiProc(void* buffer, DWORD length, void* /*user*/)
{
    BASS_StreamPutData(chan2, buffer, length); // this requires the stream (chan2) to be played
    return 1;   // continue recording
}

// Recording callback for non-radio sources - not doing anything with the data
BOOL CALLBACK DuffRecording(HRECORD /*handle*/, const void* /*buffer*/, DWORD /*length*/, void* /*user*/)
{
    return 1; // continue recording
}

void MainWindow::StartURL(QString pRadioURL)
{
    char *url;
    if (cthread) { // already connecting
        //QApplication::beep();
        return;
    }

    url=_strdup(qPrintable(pRadioURL));

    // open URL in a new thread (so that main thread is free)
    PreBuf_timer->stop();   // stop prebuffer monitoring
    //on_SetLabel4("connecting...", true);
    mgh.IsConnecting = true;  // set flag
    mgh.m_tmr.start();        // start timing clock
    tURL.url = url;           // set url text
    free(url);                // clean up

    connect(&tURL,SIGNAL(ConnectionStartFailed()),this,SLOT(OnConnectionStartFailed()));

    tURL.start();             // start new thread with start() which calls run() which calls OpenURL()
}


void MainWindow::updateTimeout(){
    //qDebug()<<"updateTimer";

    if( mgh.IsConnecting )
    {
        QString eTime;
        eTime.sprintf(" %.3f Seconds of 30 ", double(mgh.m_tmr.elapsed())/1000 );
        qDebug()<<eTime;
        //on_SetLabel5(eTime, false);
    }
    if( mgh.TriggerPrebuf )
    {
        mgh.TriggerPrebuf = false;
        qDebug()<<"Starting prepuf timer";
        PreBuf_timer->start(50); // start prebuffer monitoring
    }
    if( Finished_Init )// win != NULL ) // only after app has initiallized but before app begins exit ...
    {
        // is restart needed, signaled from end-sync
        if( bRestartURL )
        {
            bRestartURL = false;
            // MsgBox(_T(" bRestartURL has fired "), _T("debug 3"), false);
            // the above works without crash, but has delay of showing text msg. why delay ?
            // the following just seems to be 'breezed' over, no buffering, and does pick up and continue ...
            on_MainWindow_clickOff();
            if (currentRadio)
                StartURL(currentRadio->GetURL(0));
        }
    }
    if( ((input != inNetRadio)? chan2 : chan) != 0 )
    {
        // update the input level meter
        float Factor;
#ifdef Q_OS_WIN
        if( bUseWASAPI_ )
        {
            Factor = 100.0f ;
            if( input == inNetRadio )
            {
                DWORD tmp = BASS_Mixer_ChannelGetLevel( chan );
                level = min(1.0f, max(0.0f, float((DWORD(LOWORD(tmp))+HIWORD(tmp))/2)/ (32768) ));
            }
            else
            {
                // level=BASS_WASAPI_GetDeviceLevel(source,-1);
                DWORD tmp = BASS_ChannelGetLevel( chan2 );
                level = min(1.0f, max(0.0f, float((DWORD(LOWORD(tmp))+HIWORD(tmp))/2)/ (32768) ));
            }
        }
        else
#endif
        {
            DWORD tmp = BASS_ChannelGetLevel( ((input==inNetRadio)? chan : chan2) );
            level = min(1.0f, max(0.0f, float((DWORD(LOWORD(tmp))+HIWORD(tmp))/2)/ (32768/2) ));
            Factor = 70.0f ;
        }
        inlevel = ((inlevel > 0.05f)? inlevel - 0.05f : 0.05f );  // spectrum needs the inlevel value !
        if( level > inlevel ) inlevel = level;
        //ui->progressBar->setValue( max(5, min(100, int(inlevel * Factor))) );
    }





    long bitRate;
    //DWORD curPos=BASS_ChannelGetPosition(chan,BASS_POS_BYTE);
    DWORD curPos=BASS_ChannelGetPosition(chan,BASS_POS_DECODE);
    if (curPos>0){
        if (PrevBitratePos==0){
            PrevBitrateTime.start();
            PrevBitratePos=curPos;
            bitRate=0;
        }
        else{
            long tDelta=PrevBitrateTime.elapsed();
            bitRate=-1*(int)(((float)PrevBitratePos-curPos)*8/tDelta);
            PrevBitratePos=curPos;
            PrevBitrateTime.restart();

            if (bitRateCurAvg<BITRATE_AVG_SAMPLES){
                bitRateAvg=(bitRateAvg+bitRate)/(bitRateCurAvg);
                bitRateCurAvg++;
            }
            else{
                bitRateAvg=(bitRateAvg+bitRate)/BITRATE_AVG_SAMPLES;
                //qDebug()<<"Bitrate="<<bitRateAvg;
                setBitrateDisplay(bitRateAvg);
            }

        }
    }

    /*
    QWORD cur_filepos = BASS_StreamGetFilePosition(chan, BASS_FILEPOS_DOWNLOAD) ;
    DWORD alllen = BASS_StreamGetFilePosition(chan,BASS_FILEPOS_CURRENT);
    DWORD bufpos = BASS_StreamGetFilePosition(chan,BASS_FILEPOS_BUFFER);
    DWORD endpos = BASS_StreamGetFilePosition(chan,BASS_FILEPOS_END);
    qDebug()<<"STREAMDATA:  "<<"CurPos="<<cur_filepos<<"  alllen="<<alllen<<"   bufpos="<<bufpos<<"   endpos"<<endpos;
    */

    /*
    DWORD curDec=BASS_ChannelGetPosition(chan,BASS_POS_DECODE);
    qDebug()<<"CHANDATA: "<<"curPos="<<curPos<<" curDec="<<curDec;
    */


}

int MainWindow::GetBufferStatus(){
    QWORD cur_filepos = BASS_StreamGetFilePosition(chan, BASS_FILEPOS_DOWNLOAD /* BASS_FILEPOS_BUFFER */  ) ;
    DWORD progress = (DWORD)Round(( (double)cur_filepos * 1000.0) // percentage of buffer filled
                                  / (double)BASS_StreamGetFilePosition(chan,BASS_FILEPOS_END));


    //qDebug()<<"Len="<<BASS_StreamGetFilePosition(chan,BASS_FILEPOS_END);
    return(100*BASS_StreamGetFilePosition(chan,BASS_FILEPOS_BUFFER)/BASS_StreamGetFilePosition(chan,BASS_FILEPOS_END));

    return(progress);

}

void MainWindow::updateBufferStatus(){
    int progress = GetBufferStatus();
    pbBufferStatus->setValue(progress);
}

void MainWindow::prebufTimeout()
{   // monitor prebuffering progress
    //qDebug()<<"prebufTimeout";
    static int cnt = 0;
    static QWORD last_progress = 0;
    static bool aintbeenhere = true;

    //   BASS_ChannelSetSync  BASS_SYNC_DOWNLOAD
    QWORD cur_filepos = BASS_StreamGetFilePosition(chan, BASS_FILEPOS_DOWNLOAD /* BASS_FILEPOS_BUFFER */  ) ;
    DWORD progress = (DWORD)Round(( (double)cur_filepos * 1000.0) // percentage of buffer filled
                                  / (double)BASS_StreamGetFilePosition(chan,BASS_FILEPOS_END));
    if( cnt == 0 ) last_progress = progress ;
    qDebug()<<"Buffering progress: "<<progress;
    emit ConnectionProgressUpdate(progress);
    QString progressStr;
    progressStr.sprintf(_T("Buffering %d%"),progress/10);
    trayIcon->setToolTip(progressStr);
    if( progress>750 || !BASS_StreamGetFilePosition(chan,BASS_FILEPOS_CONNECTED) )
    {   // over 75% full (or end of download)
        // if( !(cnt>22 && last_progress == progress) ) {
            this->PreBuf_timer->stop(); // finished prebuffering, stop monitoring

            emit ConnectionEstablished();
            AddToHistory(currentRadio->GetTitle(),true);

            cnt = 0;
            last_progress = 0;
            aintbeenhere = true;
        // }
        { // get the broadcast name and URL
            const char *oggtags=BASS_ChannelGetTags(chan,BASS_TAG_OGG);
            if (oggtags){                
                for (;*oggtags;oggtags+=strlen(oggtags)+1)
                   qDebug()<<"OGG tags:"<<oggtags;
                const char *oggtags=BASS_ChannelGetTags(chan,BASS_TAG_OGG);
            }

            const char *icy=BASS_ChannelGetTags(chan,BASS_TAG_ICY);
            if (!icy) icy=BASS_ChannelGetTags(chan,BASS_TAG_HTTP); // no ICY tags, try HTTP
            if (icy) {
                // print all tags
                const char* tStrSave=icy;
                for (;*icy;icy+=strlen(icy)+1)
                   qDebug()<<"Broadcast tags:"<<icy;
                icy=tStrSave;

                QString radioURL="";
                QString radioName="";
                QString radioBitRate="";
                QString radioGenre="";
                for (;*icy;icy+=strlen(icy)+1) {
                    if (!_strnicmp(icy,"icy-name:",9)) {
                        qDebug()<<"RadioName:"<<icy+9;
                        radioName=(icy+9);
                    }
                    if (!_strnicmp(icy,"icy-url:",8)) {
                        radioURL.sprintf("<a href=\"%s\">%s</a>", (icy+8), (icy+8) );
                        qDebug()<<"RadioURL:"<<radioURL;
                    }
                    if (!_strnicmp(icy,"icy-br:",7)) {
                        radioBitRate=icy+7;
                        qDebug()<<"RadioBPB:"<<radioBitRate;
                    }
                    if (!_strnicmp(icy,"icy-genre:",10)) {
                        radioGenre=icy+10;
                        qDebug()<<"RadioGenre:"<<radioGenre;
                    }
                }
                int tRadioQ=radioBitRate.indexOf("Quality ");
                if (tRadioQ!=-1){
                    int tQ=(int) radioBitRate.mid(tRadioQ).toFloat();
                    radioBitRate=OggQualityMap.value(tQ);
                }

                mgh.SendUpdRadioInfo(radioName,radioURL,radioGenre,radioBitRate);
            } else{
                //ui->label_5->setText(QString(""));
            }
        }
        // get the stream title and set sync for subsequent titles
        //if( !(cnt>22 && last_progress == progress) )
            DoMeta();
        //else {
        //    Title = _T("Please wait 2.5 minutes");
        //    on_SetLabel4(Title, false);
        //}
        BASS_ChannelSetSync(chan,BASS_SYNC_META,0,&MetaSync,0); // Shoutcast
        BASS_ChannelSetSync(chan,BASS_SYNC_OGG_CHANGE,0,&MetaSync,0); // Icecast/OGG
        // set sync for end of stream
        BASS_ChannelSetSync(chan,BASS_SYNC_END,0,&EndSync,0);
        // play it!
        if( !bUseWASAPI_ )
        {
            if( BASS_ChannelPlay(chan,FALSE) == FALSE )
                Error(_T("\n BASS_ChannelPlay of URL failed \n"));
            else
                bURL_Success = true;
        }
        else
        {
#ifdef Q_OS_WIN
            DWORD tmpdev = BASS_WASAPI_GetDevice();
            // this ties the url-stream into the mixer ...
            if( !BASS_Mixer_StreamAddChannel(outmixer, chan, BASS_MIXER_BUFFER) )
                Error("\n BASS_Mixer_StreamAddChannel \n Failed ... \n");
            if (!BASS_WASAPI_SetDevice(outdev) || !BASS_WASAPI_Start()) {
                Error("\n BASS_WASAPI_Start() \n Can't start Playing \n");
            }
            else
                bURL_Success = true;
            BASS_WASAPI_SetDevice( tmpdev );
#else
            MsgBox(_T("\n bUseWASAPI is true \n on a NON-Windows Build! \n"), _T("Bass WASAPI Error"));
#endif
        }
        bVolumeNeedsAdjusting = true;
        if( BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, ((!bMute)? m_volume : 0.0f)) == 1 )
                bVolumeNeedsAdjusting = false;
    } else {
        if( aintbeenhere )
        {
            if( cnt%7 == 0 && cnt != 0 && last_progress == progress ) // buffering has stalled, could be thread priority issue.
            {
                cnt = 0;
                last_progress = 0;
                aintbeenhere = false;
                PreBuf_timer->stop();   // stop prebuffer monitoring
                //on_SetLabel4("connecting...", true);
                mgh.IsConnecting = true;  // set flag
                mgh.m_tmr.start();        // start timing clock
                tURL.OpenURL();   // try opening from main thread ...
                return;
            }
        }
        QString text;
        size_t BufSize = 256;
        char buf[BufSize];
#ifdef _WIN32
        snprintf(buf, BufSize, "buffering... %.1f%% (%d) (%I64u)", float(progress)*4.0f/30.0f, cnt++ ,
                     (unsigned int) BASS_StreamGetFilePosition(chan,  BASS_FILEPOS_DOWNLOAD /* BASS_FILEPOS_BUFFER */  ) ); // stops buffering at 75%, scale to 100%
        text = QString::fromLatin1(buf);
#else
        snprintf(buf, BufSize, "buffering... %.1f%% (%d) (%llu)", float(progress)*4.0f/30.0f, cnt++ ,
                     (unsigned int) BASS_StreamGetFilePosition(chan,  BASS_FILEPOS_DOWNLOAD /* BASS_FILEPOS_BUFFER */  ) ); // stops buffering at 75%, scale to 100%
        text = QString::fromLatin1(buf);
#endif
        if( cnt%7 == 0 && cnt != 0 ) last_progress = progress ;
        text.sprintf("buffering... %d%% (%d) (%d) (%.0f)", Round(float(progress)*4.0f/3.0f), cnt++ , (int)BASS_StreamGetFilePosition(chan, BASS_FILEPOS_DECODE),
                     (double)BASS_StreamGetFilePosition(chan,  BASS_FILEPOS_DOWNLOAD /* BASS_FILEPOS_BUFFER */  ) ); // stops buffering at 75%, scale to 100%

        //qDebug()<<text;
        //on_SetLabel5(text, false);
        bURL_Success = false;
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    on_MainWindow_close();
    event->accept();
}




void MainWindow::on_MainWindow_clickOff()
{
    qDebug()<<"clickOff";
    BufferStatusUpdate_timer->stop();


    if( mgh.IsConnecting ) return;
        PreBuf_timer->stop();
        BASS_StreamFree(chan); // close old stream
        chan = 0;
#ifdef Q_OS_WIN
    if( bUseWASAPI_ ) {
        DWORD tmpdev = BASS_WASAPI_GetDevice(); // grab id for 'no sound' device needed for spectrum
        BASS_WASAPI_SetDevice(outdev);
        BASS_WASAPI_Stop(TRUE); // flush the output device buffer
        BASS_WASAPI_SetDevice( tmpdev );
    }
#endif
    bURL_Success = false;
    bVolumeNeedsAdjusting = true;
    //on_SetLabel4(_T("not playing"), true);
    Finished_Init = false;
    Finished_Init = true;


}

void MainWindow::INITDIALOG()
{
    // win = ?? ;
    Title = _T("") ;
#ifdef Q_OS_WIN
    bUseWASAPI_ = (Runtime_OS_Version >= OS_Vista) ;
#else
    bUseWASAPI_ = false ;
#endif

    bInitPostInit = true ;
    bURL_Success = false ;

    ///////
    BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST,1); // enable playlist processing
    BASS_SetConfig(BASS_CONFIG_NET_PREBUF,0); // minimize automatic pre-buffering, so we can do it (and display it) instead
    BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT, STREAM_CONNECT_TIMEOUT);  // sky.fm now needs a longer timeout than 5000
    BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY, NULL);
    // BASS_SetConfig( BASS_CONFIG_SPLIT_BUFFER, 5000);
    BASS_SetConfig( BASS_CONFIG_FLOATDSP, TRUE );
    BASS_SetConfig( BASS_CONFIG_UPDATEPERIOD, 40 ); // the spec and update timers are 40 ms also.

#ifdef Q_OS_WIN
    // setup output device
    if( bUseWASAPI_ )
    {
        // setup BASS "no sound" device ... for spectrum ... wasapi only
        // BASS_WASAPI_BUFFER requires 'no sound device' and BASS_WASAPI_GetData requires wasapi-buffer
        BASS_Init(0, 44100, 0, NULL, NULL);

        // initialize default WASAPI output device for playback
        // DWORD tmpdev = BASS_WASAPI_GetDevice();
        if (BASS_WASAPI_Init(-1, 0, 0, 0, 0.4f, 0.04f, OutWasapiProc, NULL))
        {
            outdev=BASS_WASAPI_GetDevice();
            // create a mixer to feed the (wasapi) output device
            BASS_WASAPI_INFO wi;
            BASS_WASAPI_GetInfo(&wi);
            // NOTE: if wi.chans is greater than 2(or not 2) then lame won't record from the outmixer stream
            //       if wi.chans is replaced with a hard coded 2, the output on a 5.1 system sounds like chipmonks :-)
            outmixer = BASS_Mixer_StreamCreate(wi.freq, wi.chans,
                                             BASS_SAMPLE_FLOAT|BASS_STREAM_DECODE /*|BASS_MIXER_END|BASS_MIXER_POSEX*/ );
        }
        // BASS_WASAPI_SetDevice( tmpdev );
    }
    else
#endif
    {
        outmixer =  BASS_Mixer_StreamCreate(44100, 2,
                                            BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE  // ... MF uses decode flag
                                            /* | BASS_MIXER_END | BASS_MIXER_POSEX */
                                            );
    }

    Finished_Init = true;
	bNR_Exists = true;

    //Update_timer->start(40);
}


void MainWindow::Cleanup(){
    if( chan != 0 ) on_MainWindow_clickOff();
    if( chan2 != 0 )
    {
#ifdef Q_OS_WIN
        if( bUseWASAPI_ ) {
            if( input != inNetRadio )
            {
                DWORD tmpdev = BASS_WASAPI_GetDevice();
                BASS_WASAPI_SetDevice(source);
                BASS_WASAPI_Free();
                BASS_StreamFree(chan2);
                chan2 = 0;
                BASS_WASAPI_SetDevice(tmpdev);
            }
        } else
#endif
        {
            if( input != inNetRadio )
            {
                BASS_ChannelStop(chan2);
                chan2 = 0;
            }
        }
    }
    BASS_Free();
	BASS_RecordFree();
#ifdef Q_OS_WIN
	BASS_WASAPI_Free();
#endif
	bIsEncoding = false ;
	bVolumeNeedsAdjusting = true ;
	bInitPostInit = false ;
	bRestartURL = false;
	rchan = 0;
	BASS_StreamFree(outmixer);
	outmixer = 0;
	bNR_Exists = false;
}
