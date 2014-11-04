#include <QApplication>
#include <QInputDialog>
#include <QProcess>
#include <QDebug>
#include <QTime>
#include <QGraphicsOpacityEffect>
#include "mainwindow.h"
#include "MyThread.h"
#include "myglobalhandler.h"
#include "ui_mainwindow.h"
#include "defs.h"
#include "graphicsbutton.h"


#ifdef Q_OS_WIN
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <qt_windows.h>
    #include <QtWinExtras>
#endif


char proxy[100]=""; // proxy server
bool bUseProxy = false;

// encoder command-lines and output files
#define nCmds   4
const char *commands[nCmds-2]={
        "oggenc -o \"%s\" -",                // oggenc (OGG)
        "lame --preset standard --tt \"%s\" --ta \"%s\" --tc \"%s\" - \"%s\""
        //
        // --tt (title) --ta (artist)  //  lame (MP3) "lame --alt-preset standard - %s_%s"
};
const char *files[nCmds]={"bassOGG","bassMP3","bassWAV","bassMP4"}; // OGG,MP3,ACM,AAC
const char *exts[nCmds]={".ogg",".mp3",".wav",".mp4"};


QString			Title, Song, Artist;

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
int				encoder = 1;		// current encoder
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
        mgh.SendClickRecord();
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

    if( bRestart )
        mgh.SendClickRecord();  // restart with new title
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
        qDebug()<<"Can't open the url stream. BASS Error code="<<BASS_ErrorGetCode();
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
    //mgh.m_tmr.start();        // start timing clock
    tURL.url = url;           // set url text
    free(url);                // clean up

   //connect(&tURL,SIGNAL(ConnectionStartFailed()),this,SLOT(OnConnectionStartFailed()));

    tURL.start();             // start new thread with start() which calls run() which calls OpenURL()
}


void MainWindow::updateTimeout(){
    //qDebug()<<"updateTimer";

    if (bInitPostInit){
        bInitPostInit = false ;
    }

    if( mgh.IsConnecting )
    {
        QString eTime;
        eTime.sprintf(" %.3f Seconds of 30 ", double(mgh.m_tmr.elapsed())/1000 );
        qDebug()<<eTime;
        //on_SetLabel5(eTime, false);
    }
    if( mgh.TriggerPrebuf ){
        mgh.TriggerPrebuf = false;
        qDebug()<<"Starting prepuf timer";
        PreBuf_timer->start(50); // start prebuffer monitoring
    }
    if( Finished_Init ){// win != NULL ) // only after app has initiallized but before app begins exit ...

        // is restart needed, signaled from end-sync
        if( bRestartURL ){
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
        if( bUseWASAPI_ ){
            Factor = 100.0f ;
            if( input == inNetRadio ){
                DWORD tmp = BASS_Mixer_ChannelGetLevel( chan );
                level = min(1.0f, max(0.0f, float((DWORD(LOWORD(tmp))+HIWORD(tmp))/2)/ (32768) ));
            }
            else{
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

    QWORD curPos=BASS_ChannelGetPosition(chan,BASS_POS_DECODE);

    /*
    QWORD cur_filepos = BASS_StreamGetFilePosition(chan, BASS_FILEPOS_DOWNLOAD);
    DWORD alllen = BASS_StreamGetFilePosition(chan,BASS_FILEPOS_CURRENT);
    DWORD bufpos = BASS_StreamGetFilePosition(chan,BASS_FILEPOS_BUFFER);
    DWORD endpos = BASS_StreamGetFilePosition(chan,BASS_FILEPOS_END);
    qDebug()<<"STREAMDATA:  "<<"CurPos="<<cur_filepos<<"  alllen="<<alllen<<"   bufpos="<<bufpos<<"   endpos"<<endpos;
    DWORD curDec=BASS_ChannelGetPosition(chan,BASS_POS_DECODE);
    qDebug()<<"CHANDATA: "<<"curPos="<<curPos<<" curDec="<<curDec;
    */

    long bitRate;
    //DWORD curPos=BASS_ChannelGetPosition(chan,BASS_POS_BYTE);
    //DWORD curPos=BASS_ChannelGetPosition(chan,BASS_POS_DECODE);
    curPos=BASS_StreamGetFilePosition(chan,BASS_FILEPOS_DOWNLOAD);

    BASS_CHANNELINFO tChanInfo;
    BASS_ChannelGetInfo(chan,&tChanInfo);
    int tBitPerSample=16;
    if (tChanInfo.flags&BASS_SAMPLE_8BITS>0)
        tBitPerSample=8;
    if (tChanInfo.flags&BASS_SAMPLE_FLOAT>0)
        tBitPerSample=32;
    //qDebug()<<"tBitPerSample="<<tBitPerSample;

    if (curPos>0){
        if (PrevBitratePos==0){
            PrevBitrateTime.start();
            PrevBitratePos=curPos;
            bitRate=0;
        }
        else{
            float tDelta=(float)PrevBitrateTime.elapsed()/1000;
            //32 = 2*8 (2 channes, 8 bit)
            if (curPos-PrevBitratePos!=0){
                bitRate=(int)(((float)curPos-PrevBitratePos)*8/(tDelta*1024));
                /*
                qDebug()<<"PrevBitratePos="<<PrevBitratePos<<" curPos="<<curPos;
                qDebug()<<"Position delta="<<curPos-PrevBitratePos;
                qDebug()<<"TimeDelta="<<tDelta;
                */
                PrevBitratePos=curPos;
                PrevBitrateTime.restart();

                if (bitRateCurAvg<=BITRATE_AVG_SAMPLES){
                    bitRateAvg=(bitRateAvg+bitRate)/(bitRateCurAvg);
                    bitRateCurAvg++;
                }
                else{
                    bitRateAvg=(bitRateAvg+bitRate)/(BITRATE_AVG_SAMPLES);
                    //qDebug()<<"Bitrate="<<bitRateAvg;
                    setBitrateDisplay(QString::number(bitRateAvg));
                }
            }
        }
    }

#ifdef VISUALS
    //BASS_SFX_PluginRender(visPlugin, chan, visHDC);

    if (isVisRendering)
        return;
    isVisRendering=true;
    if (bigVis && !bigVisRunning){
        qDebug()<<"Starting WinVisualization "<<visualPlugin;
        //visWin->setWindowFlags(Qt::Popup);
        //visWinPlugin=BASS_SFX_PluginCreate(visualPlugin.toLatin1().data(),(HWND)visWin->winId(), visWin->width(),visWin->height(), 0);
        visWinPlugin=BASS_SFX_PluginCreate(visualPlugin.toLatin1().data(),0,0,0, 0);


        //visWinPlugin=BASS_SFX_PluginCreate(visualPlugin.toLatin1().data(),(HWND)visWin->winId(), visWin->width(),visWin->height(), visWinHDC);
        BASS_SFX_PluginSetStream(visWinPlugin,chan);
        //BASS_SFX_PluginResize(visWinPlugin,visWin->width(),visWin->height());
        LastVisWidth=visWin->width();
        LastVisHeight=visWin->height();

        if (BASS_SFX_PluginStart(visWinPlugin))
            qDebug()<<"Visualization plugin started";
        else
            qDebug()<<"Visualization plugin FAILED to start";

        bigVisRunning=true;
    }

    if (bigVisRunning){
        //BASS_SFX_PluginRender(visWinPlugin, chan, visWinHDC);
        BASS_SFX_PluginRender(visWinPlugin, chan, 0);


        /*
        if (!isVisResized && (LastVisWidth!=visWin->width() || LastVisHeight!=visWin->height())){
            BASS_SFX_PluginResize(visWinPlugin,visWin->width(),visWin->height());
            BASS_SFX_PluginRender(visWinPlugin, chan, visWinHDC);
            LastVisWidth=visWin->width();
            LastVisHeight=visWin->height();
            isVisResized=true;
        }

        if (isVisResized)
            isVisResized=false;
        */

    }
    isVisRendering=false;

#endif

    slVolume->setValue(BASS_GetVolume()*slVolume->maximum());
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

            AddToHistory(currentRadio->GetTitle(),true);
            emit ConnectionEstablished();
            Spectrum_timer->start(SPECTRUM_TIME);


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
                qDebug()<<"No tags";
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
            else{
                bURL_Success = true;
/*
#ifdef VISUALS
                BASS_SFX_PluginSetStream(visPlugin,chan);
                BASS_SFX_PluginResize(visPlugin,lVis->width(),lVis->height());

                if (BASS_SFX_PluginStart(visPlugin)){
                    qDebug()<<"Visualization plugin started";
                }
                else{
                    qDebug()<<"Visualization plugin FAILED to start";
                }
#endif
*/
            }
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
                //mgh.m_tmr.start();        // start timing clock
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
    qDebug()<<"MainWindow::closeEvent";
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
    input=0;
    inNetRadio=0;
    source=0;

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
    //BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY, NULL);
    // BASS_SetConfig( BASS_CONFIG_SPLIT_BUFFER, 5000);
    BASS_SetConfig( BASS_CONFIG_FLOATDSP, TRUE );
    BASS_SetConfig( BASS_CONFIG_UPDATEPERIOD, UPDATE_TIME ); // the spec and update timers are 40 ms also.

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

    connect(&tURL,SIGNAL(ConnectionStartFailed()),this,SLOT(OnConnectionStartFailed()));

    qDebug()<<"BASS_RecordSetInput";
    BASS_RecordSetInput(0, BASS_INPUT_ON, -1); // enable the selected
    chan2 = BASS_RecordStart(44100, 2, MAKELONG(BASS_SAMPLE_FLOAT,25), &DuffRecording, NULL);
    qDebug()<<"BASS_RecordSetInput done";


#ifdef VISUALS
    //visHDC=GetDC((HWND)lVis->winId());
    visWinHDC=GetDC((HWND) visWin->winId());
    //visPlugin=BASS_SFX_PluginCreate(VIS_SMALL_PLUG,(HWND)lVis->winId(), lVis->width(),lVis->height(), 0);
    //lVis->setAttribute(Qt::WA_TranslucentBackground);
#endif


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
    inNetRadio =0;
    input=0;
}

void MainWindow::clearSpec(){
    int iTotal = SPECWIDTH * SPECHEIGHT;
    if (specbuf)
        memset(specbuf, 255, iTotal);
    specButton->repaint();
}


void MainWindow::specTimeout()
{
    static DWORD quietcount=0;
    int y=0, y1=0;
    int iTotal = SPECWIDTH * SPECHEIGHT;
    int iMax = iTotal -1;
    int iLLC = iTotal - SPECWIDTH ;

    //specButton->IsSpecBeingDrawn = true ;

    // update the Spectrum Display
    if( specmode != 2 )
        memset(specbuf, 255, iTotal);

    if( specmode == 3 ) { // waveform
        DWORD c, mixchans;
        float *buf;
        BYTE  PalChanColor;
#ifdef Q_OS_WIN
        if( bUseWASAPI_ )
        {
            if( input == inNetRadio )
            {
                BASS_CHANNELINFO ci;
                BASS_ChannelGetInfo(chan, &ci); // get number of channels
                mixchans = ci.chans ;
            } else {
                BASS_WASAPI_DEVICEINFO ci;
                BASS_WASAPI_GetDeviceInfo(source, &ci); // get number of channels
                mixchans = ci.mixchans ;
            }
        }
        else
#endif
        {
            BASS_CHANNELINFO ci;
            BASS_ChannelGetInfo(((input==inNetRadio)? chan : chan2), &ci); // get number of channels
            mixchans = ci.chans ;
        }
        buf = (float*)calloc(2*mixchans*SPECWIDTH, sizeof(float)); // allocate buffer for data

#ifdef Q_OS_WIN
        if( bUseWASAPI_ && input == inNetRadio )
        {
            BASS_Mixer_ChannelGetData(chan, buf, (mixchans*SPECWIDTH*sizeof(float))
                                          | BASS_DATA_FLOAT | /* BASS_DATA_FFT_REMOVEDC | */
                                          BASS_DATA_FFT_NOWINDOW | BASS_DATA_FFT_INDIVIDUAL ) ;
        } else
#endif
        {
            BASS_ChannelGetData(((input==inNetRadio)? chan : chan2), buf,(mixchans*SPECWIDTH*sizeof(float))
                                | BASS_DATA_FLOAT | BASS_DATA_FFT_REMOVEDC |
                                BASS_DATA_FFT_NOWINDOW | BASS_DATA_FFT_INDIVIDUAL );
        }

        for(c=0; c < mixchans; c++)
        {
            switch( c & 7 )
            {
            default:
            case 0: PalChanColor =  18;  break; // green, darker for contrast with yellow
            case 1: PalChanColor = 127;  break; // red
            case 2: PalChanColor = 220;  break; // yellow 224
            case 3: PalChanColor = 192;  break; // purple
            case 4: PalChanColor = 254;  break; // light blue
            case 5: PalChanColor = 253;  break; // pink
            case 6: PalChanColor = 160;  break; // blue
            case 7: PalChanColor = 204;  break; // light purple
            }
            for(int x=0; x < SPECWIDTH; x++)
            {
                int v= Round((1.0f - (1.0f/max(0.02f,inlevel))* buf[x*mixchans+c])*(SPECHEIGHT/2)) ; // invert and scale to fit display
                if (v<0) v=0;
                else if (v>=SPECHEIGHT) v=SPECHEIGHT-1;
                if (!x) y=v;
                do { // draw line from previous sample...
                    if (y<v) y++;
                    else if (y>v) y--;
                    specbuf[max(0,min(iMax, iLLC - y*SPECWIDTH+x))] = PalChanColor ; // ((c&1)? 127 : 1 ); // left=green, right=red (could add more colours to palette for more chans)
                } while (y!=v);
            }
        }
        free( buf );
    } else {
        float fft[1024];
        float csf = 127.0f / SPECHEIGHT ; // color scale factor so pallette is draw between 0 and 127
#ifdef Q_OS_WIN
        if( bUseWASAPI_ && input == inNetRadio )
        {
                BASS_Mixer_ChannelGetData(chan, fft, BASS_DATA_FFT2048);
        } else
#endif
            BASS_ChannelGetData(((input==inNetRadio)? chan : chan2), fft, BASS_DATA_FFT2048); // get the FFT data

        if( specmode == 0 ) { // "normal" FFT
            for(int x=0; x < SPECWIDTH/2 ; x++)
            {
#if 1
                y = Round( sqrt(fft[x+1])*3*SPECHEIGHT-4 ); // scale it (sqrt to make low values more visible)
#else
                y=fft[x+1]*10*SPECHEIGHT; // scale it (linearly)
#endif
                if (y>SPECHEIGHT) y=SPECHEIGHT; // cap it
                if (x && (y1=(y+y1)/2)) // interpolate from previous to make the display smoother
                    while (--y1>=0) specbuf[max(0, min(iMax, iLLC - y1*SPECWIDTH+x*2-1))] = int(float(y1 +1)*csf);
                y1=y;
                while (--y>=0) specbuf[max(0, min(iMax, iLLC - y*SPECWIDTH+x*2))] = int(float(y +1)*csf); // draw level
            }
        } else if( specmode == 1 ) { // logarithmic, acumulate & average bins
            int b0=0;

            int FullBandWidth = (SPECWIDTH / SPECBANDS) ;
            int DrawBandWidth = Round(0.9 * FullBandWidth);
            int iiMax = iMax - DrawBandWidth ;
            for(int x=0;x<SPECBANDS;x++) {
                float peak=0;
                int b1= Round(pow(2,x*10.0/(SPECBANDS-1)));
                if (b1>1023) b1=1023;
                if (b1<=b0) b1=b0+1; // make sure it uses at least 1 FFT bin
                for (;b0<b1;b0++)
                    if (peak<fft[1+b0]) peak=fft[1+b0];
                y= Round(sqrt(peak)*3*SPECHEIGHT-4); // scale it (sqrt to make low values more visible)
                if (y>SPECHEIGHT) y=SPECHEIGHT; // cap it
                while (--y>=0)
                    memset( specbuf + max(0,min(iiMax, iLLC - y*SPECWIDTH+x*FullBandWidth )),
                            int(float(y+1)*csf), DrawBandWidth); // draw bar
            }
        } else { // "3D" or specmode == 2
            for(int x=0; x < SPECHEIGHT ; x++)
            {
                y= Round(sqrt(fft[x+1])*3*127); // scale it (sqrt to make low values more visible)
                if (y>127) y=127; // cap it
                specbuf[max(0,min(iMax, iLLC - x*SPECWIDTH+specpos))] = 128+y; // plot it
            }
            // move marker onto next position
            specpos = (specpos+1) % SPECWIDTH;
            for(int x=0;x<SPECHEIGHT;x++)
                specbuf[max(0,min(iMax, iLLC - x*SPECWIDTH+specpos))] = 255;
        }
    }

update_display:
    // update the display
    // this->specButton uses pixmap named: Spec which it copies from this->pSpec
    // ... specButton's paintEvent uses a QPainter which replaces the BitBLT that was HERE.
    // ... so here, we just add in any needed text and call update() ...
    if( level < 0.01f || ((input != inNetRadio)? chan2 : chan) == 0 ){
        quietcount++;
        // it's been quiet for over 3 seconds
        if( quietcount>NOSOUND_SPEC_DELAY/SPECTRUM_TIME){
            this->specButton->MyText = "No sound" ;
        }
    }
    else{
        if (specButton->MyText!="")
            specButton->MyText="";
        quietcount = 0; // not quiet
    }
    //specButton->IsSpecBeingDrawn = false ;
    specButton->repaint();
    //specButton->update();
}


void MainWindow::on_specButton_clicked()
{
    specmode = (specmode+1)%4; // change spectrum mode
    memset(specbuf, 0, SPECWIDTH * SPECHEIGHT);	// clear display
}

void MainWindow::on_pbRecord_clicked()
{
    on_ClickRecord();
}


#ifdef Q_OS_WIN
bool MainWindow::DoACM_Dialog(bool Init)
{
    if( Init ) // initialize the acm as something in BASS library needs it, aka Mystery Error 37...
    {
        DWORD  dwFlags;
        if( bUseWASAPI_ ) dwFlags = BASS_STREAM_DECODE ;
        else dwFlags = BASS_STREAM_AUTOFREE ;
        HSTREAM dumChan = BASS_StreamCreate(44100,2, dwFlags, NULL, NULL);
        bool bRet = (BASS_Encode_GetACMFormat(dumChan, acmduncil, acmformlen, NULL,
                    MAKELONG(BASS_ACM_SUGGEST|BASS_ACM_RATE|BASS_ACM_CHANS, 0x0002)) != 0);
        BASS_StreamFree(dumChan);
        return bRet;
    }
    else
        return (BASS_Encode_GetACMFormat(((input != inNetRadio)? chan2 : chan), acmform, acmformlen, NULL, BASS_ACM_DEFAULT) != 0);
}
#endif

void MainWindow::on_ClickRecord(){

    qDebug()<<"MainWindow::on_ClickRecord";

    if( m_Encoder == 0 )  // ... if not null then stop recording ...
    {   // start recording ...
        if( ((input != inNetRadio)? chan2 : chan) != 0 )  // anything to record ?
        {
            QString TitleOut, filename ;
            // if filename has invalid characters, replace with under-score
#ifdef Q_OS_WIN
            QRegExp invalsh("[\\\\/|?\"<>:*]");
#else
            QRegExp invalsh("[\\\\/|?\"<>:*]");  // are these the same invalid filename characters in Linux + Mac ??
#endif
            Title.replace(invalsh, "_");
            TitleOut.sprintf(_T("%s_%s"), qPrintable(Title), exts[encoder]);

            TitleOut=recPath+TitleOut;
            //
            // if filename exists then add "(%d)", nCnt++ until not exists...
            if( QFile::exists(TitleOut) )
            {
                for(int nCnt=2; nCnt < 16777576; nCnt++)
                {
                    TitleOut.sprintf(_T("%s_(%d)%s"), qPrintable(Title), nCnt, exts[encoder]);
                    if( !QFile::exists(TitleOut) ) break;
                    if( nCnt == 16777575 )
                    {
                        // this is OVERKILL programming, as either audio stream buffer will overflow
                        // while this is looping, or a big chuck at the begining of the track will be lost.
                        qDebug()<<"Generating unique filename";
                        Title.append("_(16777576)");
                        nCnt = 1;
                    }
                }
            }
            qDebug()<<"CurrentEncoder="<<encoder;
            switch( encoder ){
            case 3:
#ifdef  Q_OS_MAC
                m_Encoder = BASS_Encode_StartCAFile(((input != inNetRadio)? chan2 : chan), 'mp4f', 'aac ',
                                                    BASS_ENCODE_AUTOFREE | BASS_ENCODE_FP_16BIT, 128000,
                                                    qPrintable(TitleOut));
                if( !m_Encoder )
                {   // couldn't start the CoreAudio encoder
                    Error(" Couldn't start CoreAudio encoding ");
                    bIsEncoding = false;
                    return;
                }
#else
                MsgBox(_T("\n Sorry, Mac OSX Only Format ... \n"));
                return;
#endif
                break;
            case 2:
#ifdef Q_OS_WIN
                if( acmform->nChannels != 0 || DoACM_Dialog() ){   // select the ACM codec
                    m_Encoder = BASS_Encode_StartACMFile( ((input != inNetRadio)? chan2 : chan) ,
                                                          acmform, BASS_ENCODE_AUTOFREE, qPrintable(TitleOut) );
                    if( !m_Encoder ){   // couldn't start the ACM encoder
                        Error(" Couldn't start ACM encoding ");
                        bIsEncoding = false;
                        return;
                    }
                }
#else
                MsgBox(_T("\n Sorry, Windows Only Format ... \n"));
                return;
#endif
                break;
            default:
                char cmd[1200];
                sprintf(cmd, /*254,*/ _T("%s//%s"), qPrintable(QApplication::applicationDirPath()), commands[encoder]);
                QString comment="Recorded from "+currentRadio->GetTitle()+" using Radiola player";
                filename.sprintf(cmd, qPrintable(Song), qPrintable(Artist), qPrintable(comment),qPrintable(TitleOut) );
                qDebug()<<"Recording file:"<<filename;

                m_Encoder = BASS_Encode_Start(
                            ((input != inNetRadio)? chan2 : chan /*((bUseWASAPI_)? outmixer : chan)*/ )
                            // chan      // this works on both Windows+Linux, but is it buffered ?
                            // outmixer  // this works only on Windows(WASAPI) with 2 channel stereo(no file made 5.1 systems), and is buffered. (on Linux, no file is made!!)
                            , qPrintable( filename ),
                            BASS_ENCODE_AUTOFREE | BASS_ENCODE_FP_16BIT,
                            NULL, 0);  // start the OGG/MP3 encoder
                if( !m_Encoder ){
                    Error("  Couldn't start encoding ... ");
                    bIsEncoding = false;
                    return;
                }
                break;
            }
            bIsEncoding = true ;
            qDebug()<<"Recording started m_Encoder="<<m_Encoder;
        } // end if chan != 0
        else
            qDebug()<<"nothing to record";
    }
    else{
        qDebug()<<"stopping the record... m_Encoder="<<m_Encoder;
        if( BASS_Encode_Stop( m_Encoder ) == FALSE )
            Error(_T("\n BASS_Encode_Stop Failed \n"));
        else
        {
            m_Encoder = 0;
            bIsEncoding = false;
        }
    }

    if (bIsEncoding)
        onRecordingStarts();
    else
        onRecordingStops();
}
