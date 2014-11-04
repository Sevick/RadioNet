#include <QDebug>
#include <QMessageBox>
#include <QDesktopServices>
#include <QtNetwork>
#include <QNetworkRequest>
#include <QTime>
#include <QMenu>
#include <QFile>
#include <QNetworkProxy>
#include <QStandardItemModel>
#include <QShortcut>
#include <QDesktopServices>
#include <QMovie>
#include <QSize>
#include <QGraphicsDropShadowEffect>
#include "mainwindow.h"
#include "myglobalhandler.h"
#include "radioedit.h"
#include "connectionprogress.h"
#include "showhistory.h"
#include "defs.h"
#include "playlistcl.h"

QString Proxy;
QString ProxyPort;
QString ProxyUser;
QString ProxyPass;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);

    Finished_Init = false;
    AllowInit = false;
    PreBuf_timer = new QTimer;
    Update_timer= new QTimer;
    Ping_timer= new QTimer;
    Spectrum_timer = new QTimer;
    BufferStatusUpdate_timer= new QTimer;
    singleClickTimer=new QTimer;
    m_msgbox = NULL;
    isPlaying=false;
    syncLost=false;
    currentRadio=NULL;
    currentTrack="";
    dConnectionProgress=NULL;
    dDelConfirm=NULL;
    dShowHistory=NULL;
    netManager=NULL;
    currentRadioURL=-1;
    streamsErrLoop=false;
    pSpec=NULL;
    mMoving=false;
    savedVolume=0.5;
    isMuted=false;
    treeSelectionChanged=false;
    isRecording=false;
    newVersionAnimation=NULL;
    recButtonAnimation=NULL;
    isRootOperation=false;
#ifdef VISUALS
    isVisResized=false;
    isVisRendering=false;
    visWin=new VisWinCL(this);
    bigVis=false;
    bigVisRunning=false;
#endif

    initBitRate();
    initOggQualityMap();

    qDebug()<<"Cheking BASS version";
    if (HIWORD(BASS_GetVersion())!=BASSVERSION) {
        qDebug()<<"An incorrect version of BASS.DLL was loaded";
        QMessageBox::warning(this,QString("warning"),QString("An incorrect version of BASS.DLL was loaded"));
    }

    BASS_SetConfigPtr(BASS_CONFIG_NET_AGENT, (void*) _T(PLAYER_HTTPAGENT));


#ifdef Q_OS_WIN
    if (BASS_PluginLoad("bass_aac.dll",0)==0){
        qDebug()<<"Unable to load bass_aac.ddl  BassError="<<BASS_ErrorGetCode();
    }
#elif Q_OS_LINUX
    if (BASS_PluginLoad("bass_aac.so",0)==0){
        qDebug()<<"Unable to load bass_aac.ddl  BassError="<<BASS_ErrorGetCode();
    }
#endif


    QSettings tSettings(AppPath+CONFIG_FILENAME,QSettings::IniFormat);
    tSettings.setIniCodec("ISO 8859-1");
    Proxy=tSettings.value("proxy","").toString();
    ProxyPort=tSettings.value("proxyport","").toString();
    ProxyUser=tSettings.value("proxyuser","").toString();
    ProxyPass=tSettings.value("proxypass","").toString();
    visualPlugin=tSettings.value("visual",VIS_WIN_PLUG).toString();
    qDebug()<<"visualPlugin="<<visualPlugin;
    recPath=tSettings.value("recpath","").toString();
    recPath=recPath.replace("\\","\\\\");
#ifdef Q_OS_WIN
    if (recPath!="" && recPath.at(recPath.length()-1)!='\\')
            recPath=recPath+"\\";
#else
    if (recPath!="" && recPath.at(recPath.length()-1)!='/')
            recPath=recPath+"/";
#endif

    if (recPath==""){
#ifdef Q_OS_WIN
        recPath=QDir::toNativeSeparators(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).at(0)+"/");
#endif

#ifdef Q_OS_MAC
        recPath=AppPath.fromLatin1(argv[0]);
        recPath+="/";
#endif
    }

    qDebug()<<"Recording path ="<<recPath;

    if (Proxy!=""){
        qDebug()<<"Proxy="<<Proxy;
        QString tBassProxyStr=ProxyUser;
        if (ProxyPass!="")
            tBassProxyStr+=":"+ProxyPass;
        if (ProxyUser!="")
            tBassProxyStr+="@";
        tBassProxyStr+=Proxy+":"+ProxyPort;
        qDebug()<<"BASSProxy="<<tBassProxyStr;
        qDebug()<<"BASSProxy="<<tBassProxyStr.toLatin1().data();
        //strcpy(proxyStrChar,Proxy.toLatin1().data());
        BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY,tBassProxyStr.toLatin1().data());
        //BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY,&proxyStrChar);
    }
    else{
        BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY, NULL);
    }

    //if( !InitVersion() ) ErrorMsgBox(_T("\n Error Setting up Version strings \n"));
    if( !InitBassErrorMap() ) ErrorMsgBox(_T("\n Error setting up Error Msgs \n"));
    if (!BASS_Init(-1,44100,0,NULL,NULL)) {
        ErrorMsgBox(_T("\nUnable to initialize BASS library\n"), 86, false);
        exit(86);
    }

    savedVolume=BASS_GetVolume();
    slVolume->setValue(savedVolume*slVolume->maximum());


#ifdef Q_OS_WIN
    // allocate ACM format buffer, using suggested buffer size
    acmformlen = BASS_Encode_GetACMFormat(0,NULL,0,NULL,0);
    acmform = (WAVEFORMATEX*)malloc(acmformlen);
    memset(acmform,0,acmformlen);
    acmduncil = (WAVEFORMATEX*)malloc(acmformlen);
    memset(acmduncil, 0, acmformlen);
    //
#endif

    connect(radioTree,SIGNAL(AddStationSig(QModelIndex,RadioCL*)),this,SLOT(AddStation(QModelIndex,RadioCL*)));
    connect(radioTree, SIGNAL(treeSelectionChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(radioTreeSelectionChanges(const QModelIndex &, const QModelIndex &)));

    qDebug()<<"Connecting timers signals ";
    connect(PreBuf_timer, SIGNAL(timeout()), this, SLOT(prebufTimeout()));
    connect(Update_timer, SIGNAL(timeout()), this, SLOT(updateTimeout()));
    connect(Ping_timer, SIGNAL(timeout()),this, SLOT(pingRadio()));
    connect(Spectrum_timer, SIGNAL(timeout()), this, SLOT(specTimeout()));
    connect(BufferStatusUpdate_timer, SIGNAL(timeout()),this, SLOT(updateBufferStatus()));
    connect(singleClickTimer, SIGNAL(timeout()),this, SLOT(singleClickTimeout()));

    qDebug()<<"Connecting mgh signals";
    connect( &mgh, SIGNAL(SendUpdTrackInfo(QString)), this, SLOT(on_UpdTrackInfo(QString)) );
    connect( &mgh, SIGNAL(SendUpdRadioInfo(QString,QString,QString,QString)), this, SLOT(on_UpdRadioInfo(QString,QString,QString,QString)));
    connect( &mgh, SIGNAL(SendSyncLost()), this, SLOT(on_SyncLost()) );
    connect( &mgh, SIGNAL(SendPlaybackStarts()), this, SLOT(on_PlaybackStarts()) );
    connect( &mgh, SIGNAL(SendClickRecord()), this, SLOT(on_ClickRecord()) );

    qDebug()<<"Creating actions";
    createActions();
    qDebug()<<"Creating tray icon";
    createTrayIcon();

    qDebug()<<"Setting tray icon";
    setIcon(PLAYER_STATUS_INACTIVE);
    trayIcon->setVisible(true);
    trayIcon->show();
    setWindowTitle(QString(PLAYER_NAME)+" v"+RADIOLA_VERSION);

    qDebug()<<"Initializing spectrum image";
    specbuf = NULL;
    specmode = DEFAULT_SPEC_MODE; // spectrum mode
    specpos = 0; // spectrum mode (and marker pos for 2nd mode)
    pSpec = new QImage(SPECWIDTH, SPECHEIGHT, QImage::Format_Indexed8);
    // setup palette
    pSpec->setColor(0, qRgb(0,0,0));
    for(int a=1; a < 128; a++) {
        pSpec->setColor(a, qRgb(2*a, 256-2*a, 0));
    }
    for(int a=0; a < 32; a++) {
        pSpec->setColor(128+a, qRgb(0, 0, 8*a));
        pSpec->setColor(128+32+a, qRgb(8*a, 0, 255));
        pSpec->setColor(128+64+a, qRgb(255, 8*a, 8*(31-a)));
        //pSpec->setColor(128+64+a, qRgb(8*(31-a), 8*a, 8*a));
        pSpec->setColor(128+96+a, qRgb(255, 255, 8*a));
        //pSpec->setColor(128+96+a, qRgb(255, 255, 8*a));
    }
    pSpec->setColor(254, qRgb(112, 112, 255));
    pSpec->setColor(253, qRgb(255, 128, 128));
    pSpec->setColor(255, qRgb(212,208,200));    // background color
    // create the bitmap
    specbuf = (BYTE*)pSpec->bits();
    pSpec->fill(255);
    specButton->setSpec(pSpec);

    readSettings();

    qDebug()<<"Connecting tray signals ";
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    setupRadioTree();

    historyFile.setFileName(GetHistoryDir());

    if (!historyFile.open(QIODevice::ReadWrite | QIODevice::Append)){
        qDebug()<<"Unable to open history file for write"<<historyFile.fileName();
        return;
    }

    StopPlayback();

    QIcon icon = QIcon(MAINWIN_ICON);
    setWindowIcon(icon);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(showSettings()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), this, SLOT(showRecPath()));
    new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_H), this, SLOT(showHistory()));
#ifdef Q_OS_WIN
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this, SLOT(showVisualization()));
#endif
    new QShortcut(QKeySequence(Qt::Key_Space), this, SLOT(on_pbPlay_clicked()));
    new QShortcut(QKeySequence(Qt::Key_MediaPlay), this, SLOT(on_pbPlay_clicked()));
    new QShortcut(QKeySequence(Qt::Key_Delete), this, SLOT(removeRow()));
    new QShortcut(QKeySequence(Qt::Key_Insert), this, SLOT(insertStation()));
    new QShortcut(QKeySequence(Qt::Key_Plus), this, SLOT(insertSubfolder()));

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(1); //Adjust accordingly
    effect->setOffset(3,3); //Adjust accordingly
    setShadow(bnShowHistory);
    setShadow(pbPlay);
    setShadow(pbRecord);
    setShadow(pbQuit);

    recButtonAnimation=new QMovie(":/images/rec_animation.gif");
    connect(recButtonAnimation,SIGNAL(frameChanged(int)),this,SLOT(setRecButtonIcon(int)));

    singleClickTimer->setInterval(QApplication::doubleClickInterval());
    singleClickTimer->setSingleShot(true);

    CheckNewVersion();

    netManager = new QNetworkAccessManager(this);
    if (Proxy!=""){
        netManager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,Proxy,ProxyPort.toInt(),ProxyUser,ProxyPass));
    }
    connect(netManager, SIGNAL(finished(QNetworkReply*)),this, SLOT(pingReply(QNetworkReply*)));
}

void MainWindow::setRecButtonIcon(int pFrame){
    pbRecord->setIcon(QIcon(recButtonAnimation->currentPixmap()));
}

void MainWindow::onRecordingStarts(){
    recButtonAnimation->start();
    isRecording=true;
}

void MainWindow::onRecordingStops(){
    recButtonAnimation->stop();
    pbRecord->setIcon(QIcon());
    isRecording=false;
}


void MainWindow::setShadow(QPushButton* pButton){
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(2); //Adjust accordingly
    effect->setOffset(3,3); //Adjust accordingly
    pButton->setGraphicsEffect(effect);
}

QString MainWindow::GetHistoryDir(){
    return(AppPath+"history.txt");
}

void MainWindow::initBitRate(){
    PrevBitratePos=0;
    bitRateAvg=0;
    bitRateCurAvg=1;

    setBitrateDisplay("-");
    setAudioQuaDisplay("-");
}

void MainWindow::contextMenuRequest(QPoint pReqPos){

    //QModelIndex index=radioTree->indexAt(pReqPos);
    QModelIndex index=proxyModel->mapToSource(radioTree->indexAt(pReqPos));
    QMenu *menu=new QMenu(this);

    if (!index.isValid()){
        // context menu on empty space
        isRootOperation=true;
        menu->addAction(insertChildAction);        
    }
    else{
        isRootOperation=false;
        QString tClickedTitle=((TreeModel*) radioTree->model())->getItem(index)->data(0).toString();
        qDebug()<<"ContextMenuRequested@"<<pReqPos.x()<<","<<pReqPos.y()<<"  Title@pos:"<<tClickedTitle;

        if (model->getItem(index)->getIsFolder()){
            menu->addAction(insertChildAction);
        }
        else{
            menu->addAction(editRadioAction);
        }
        menu->addAction(insertRowAction);
        menu->addAction(renameRowAction);
        menu->addAction(removeRowAction);
    }
    menu->setStyleSheet(MENU_STYLE);

    menu->popup(radioTree->viewport()->mapToGlobal(pReqPos));
}


void MainWindow::setupRadioTree(){

    qDebug()<<"Initializing radiolist";

    model = new TreeModel();
    radioTree->setModel(model);

    proxyModel = new TreeSortFilterProxyModel;
    proxyModel->setSourceModel(model);
    proxyView = new QTreeView;
    proxyView->setRootIsDecorated(false);
    proxyView->setAlternatingRowColors(true);
    proxyView->setModel(proxyModel);
    proxyView->setSortingEnabled(true);
    proxyModel->sort(0, Qt::AscendingOrder);
    connect(filterPatternLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(filterRegExpChanged()));

    //radioTree->setModel(proxyModel);
    radioTree->SetProxyModel(proxyModel);

    for (int column = 0; column < model->columnCount(); ++column)
        radioTree->resizeColumnToContents(column);

    radioTree->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(radioTree->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &,
                                    const QItemSelection &)),
            this, SLOT(updateActions()));
    connect(radioTree,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(radioDoubleClicked(QModelIndex)));
    connect(radioTree,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(contextMenuRequest(QPoint)));

    connect(insertRowAction, SIGNAL(triggered()), this, SLOT(insertStation()));
    connect(removeRowAction, SIGNAL(triggered()), this, SLOT(removeRow()));
    connect(insertChildAction, SIGNAL(triggered()), this, SLOT(insertSubfolder()));
    connect(editRadioAction,SIGNAL(triggered()),this,SLOT(editRadio()));

    updateActions();
}

void MainWindow::filterRegExpChanged()
{
    QRegExp::PatternSyntax syntax =
            QRegExp::PatternSyntax(QRegExp::RegExp);
    Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
    QRegExp regExp(filterPatternLineEdit->text(), caseSensitivity, syntax);
    proxyModel->setFilterRegExp(regExp);
}


MainWindow::~MainWindow(){
    this->Cleanup();
    historyFile.close();
    delete PreBuf_timer;
    delete Update_timer;
    delete Ping_timer;
    delete Spectrum_timer;
    delete singleClickTimer;
    if( m_msgbox != NULL ) delete m_msgbox;
    if (dConnectionProgress) delete dConnectionProgress;
    if (dDelConfirm) delete dDelConfirm;
    if (dShowHistory) delete dShowHistory;
    if (newVersionAnimation) delete newVersionAnimation;
    if (recButtonAnimation) delete recButtonAnimation;
    if (netManager) delete netManager;
}


void MainWindow::on_MainWindow_close(){
    qDebug()<<"MainWindow::on_MainWindow_close";
    //this->on_MainWindow_clickOff();
    writeSettings();
    //this->close();
}


void MainWindow::on_UpdRadioInfo(QString pRadioTitle,QString pRadioWeb, QString pGenre, QString pBPS){
    lStationNameTxt->setText(pRadioTitle.trimmed());
    lStationURL->setText("<html><head><style type=text/css>"
                         "a:link {color:green; text-decoration:underline;}"
                         "a:hover {color:green; text-decoration:underline}"
                         "</style></head><body>"+pRadioWeb+"</a></body></html>");
    lGenreTxt->setText(pGenre);
    lAudioQuality->setText(pBPS);
}


void MainWindow::on_UpdTrackInfo(QString pTrackInfo){
    if (currentTrack!=pTrackInfo){
        qDebug()<<"Updating track info: "<<pTrackInfo;
        lTrackNameTxt->setText(pTrackInfo);
        AddToHistory(HISTORY_TAB+pTrackInfo);
        trayIcon->showMessage(currentRadio->GetTitle(),"Next track: "+pTrackInfo);
        QString tTrayToolTip;
#ifdef Q_OS_WIN
        tTrayToolTip=pTrackInfo.replace("&","&&&");
#else
        tTrayToolTip=pTrackInfo;
#endif
        trayIcon->setToolTip(tTrayToolTip);
    }
}


void MainWindow::on_SyncLost(){
    qDebug()<<"on_SyncLost";
    if (!syncLost){
        syncLost=true;
        qDebug()<<"Sync lost. We need to reconnect";
        cleanRadioInfo();
        trayIcon->setToolTip("Connection lost: Reconnecting...");
        setIcon(PLAYER_STATUS_ERROR);
        initBitRate();
        StopTimers();
        Ping_timer->start(RETRY_TIME);
    }
}


void MainWindow::on_PlaybackStarts(){
    qDebug()<<"on_PlaybackStarts";
    syncLost=false;
    setIcon(PLAYER_STATUS_ACTIVE);
    pbRecord->setEnabled(true);
    isPlaying=true;
    //pingRadio();
    initBitRate();
    StartTimers();
    if (streamsErrLoop)
        streamsErrLoop=false;
}


void MainWindow::StartPlayback(){
    if (isPlaying){
        StopPlayback();
    }

    qDebug()<<"Starting playback";
    if (currentRadio){
        pbPlay->setText("Stop");

        if (!dConnectionProgress){
            dConnectionProgress=new ConnectionProgress(currentRadio->GetTitle(),this);
            connect(this,SIGNAL(ConnectionProgressUpdate(int)),dConnectionProgress,SLOT(OnConnectionProgressUpdate(int)));
            connect(this,SIGNAL(ConnectionEstablished()),dConnectionProgress,SLOT(OnConnectionEstablished()));
            connect(this,SIGNAL(ConnectionFailed()),dConnectionProgress,SLOT(OnConnectionFailed()));
        }

        currentRadioURL=0;
        dConnectionProgress->ChangeURL(currentRadio->GetURL(0));
        dConnectionProgress->setTitle(currentRadio->GetTitle());
        StartURL(currentRadio->GetURL(0));

        QApplication::setOverrideCursor(Qt::WaitCursor);
        int tResult=dConnectionProgress->exec();
        QApplication::restoreOverrideCursor();
        delete dConnectionProgress;
        dConnectionProgress=NULL;

        if (tResult==QDialog::Rejected){
            qDebug()<<"Connection aborted";
        }
    }
}


void MainWindow::AddToHistory(QString pText, bool pSeparate){
    QDate curDate=QDate::currentDate();
    QTime curTime=QTime::currentTime();

    QString logStr;
    if (pSeparate)
        logStr="\n";
    logStr=logStr+curDate.toString("dd.MM.yy")+" "+curTime.toString("hh:mm")+HISTORY_TAB+pText;

    qDebug()<<"Adding to history:"<<logStr;
    historyFile.write((logStr+"\n").toUtf8());
    historyFile.flush();

    currentSessionHistory.insert(0,logStr);
    emit HistoryChanged(logStr);
}


void MainWindow::StopPlayback(){
    qDebug()<<"Stopping playback";
    cleanRadioInfo();
    setIcon(PLAYER_STATUS_INACTIVE);
    pbPlay->setText("Play");
    trayIcon->setToolTip(PLAYER_NAME);
    pbRecord->setEnabled(false);
    on_MainWindow_clickOff();
    isPlaying=false;
    syncLost=false;
    StopTimers();
    initBitRate();
    clearSpec();
    updateActions();
}


void MainWindow::StopTimers(){
    Ping_timer->stop();
    PreBuf_timer->stop();
    BufferStatusUpdate_timer->stop();
    Update_timer->stop();
    Spectrum_timer->stop();
}


void MainWindow::StartTimers(){
    Ping_timer->start(PING_TIME);
    BufferStatusUpdate_timer->start(UPDATE_TIME);
    Update_timer->start(UPDATE_TIME);
}


void MainWindow::pingRadio(){

    if (syncLost){
        qDebug()<<"Retry ping";
        StartPlayback();
    }
    else{
        qDebug()<<"Pinging radio: "<<pingString;

        QNetworkRequest req;
        req.setUrl(QUrl(pingString));
        req.setRawHeader("User-Agent",PLAYER_PING_HTTPAGENT);

        netManager->get(req);
    }
}

void MainWindow::pingReply(QNetworkReply* pNetRepl){

    qDebug()<<"pingRepl length="<<pNetRepl->size();
}


void MainWindow::setIcon(int index){
    QIcon icon;
    switch (index){
    case PLAYER_STATUS_ERROR:
        icon = QIcon(TRAY_ICON_ERROR);
        trayIcon->setIcon(icon);
        break;
    case PLAYER_STATUS_ACTIVE:
        icon = QIcon(TRAY_ICON_ACTIVE);
        trayIcon->setIcon(icon);
        break;
    case PLAYER_STATUS_INACTIVE:
        icon = QIcon(TRAY_ICON_INACTIVE);
        trayIcon->setIcon(icon);
        break;
    default:
        break;
    }
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason){

    switch (reason) {
    case QSystemTrayIcon::Trigger:
        qDebug()<<"Tray icon leftclicked";
        show();
        raise();
        activateWindow();
        break;
    default:
        qDebug()<<"iconActivated with strange reason:"<<reason;
        break;
    }
}

void MainWindow::setAudioQuaDisplay(QString pBitRate){
    lAudioQuality->setText(pBitRate);
}

void MainWindow::setBitrateDisplay(QString pBitRate){
    lNetUsageTxt->setText(pBitRate);
}

void MainWindow::cleanRadioInfo(){
    lNetUsageTxt->setText("");
    lTrackNameTxt->setText("");
    lStationNameTxt->setText("");
    lStationURL->setText("");
    lGenreTxt->setText("");
}


void MainWindow::showAbout(){
        qDebug()<<"showAbout";
        QString myAbout;
        myAbout.sprintf("Radiola v.%s\n(c) 2014 FBytes.com\n\nBuilt with QT SDK and Bass audio library\nspecial thanks to F.G.Zacharias\n\nMac build by Zuker",
                        RADIOLA_VERSION);
        QMessageBox::about(this, _T(PLAYER_NAME), myAbout);
}


void MainWindow::exitPlayer(){
        qDebug()<<"quitApp";
        QApplication::quit();
}


void MainWindow::createActions(){
    quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(exitPlayer()));

    aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
}

void MainWindow::createTrayIcon(){
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(aboutAction);

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIconMenu->setStyleSheet(MENU_STYLE);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}


void MainWindow::readSettings(){
    qDebug()<<"readSettings";
    QSettings settings("FBytes.com", PLAYER_NAME);

    RadioUID=settings.value("UID",QString("")).toString();
    if (RadioUID==NULL){
    //if (1==1){

        trayIcon->showMessage(PLAYER_NAME,"Click this icon to start listening!");

        qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
        int randInt;
        for(int i=0; i<20; i++)
        {
        randInt = qrand()%('Z'-'A'+1)+'A';
        RadioUID.append(randInt);
        }
    }
    qDebug()<<"RadioUID="<<RadioUID;
    pingString=PING_URL;
    pingString.append("?ID=");
    pingString.append(RadioUID);
    pingString.append("&PLATFORM=WIN");
    settings.setValue("UID", RadioUID);

#ifdef Q_OS_WIN
    // ACM Settings ... this creates:
    // BASS_ERROR_NOTAVAIL == There are no codecs available that will accept the channel's format ,
    // which can only be fixed by user dialog ... see DoACM_Dialog(true) uses: BASS_ACM_SUGGEST. why error??
    settings.beginGroup("ACM");
    acmform->cbSize = (WORD)settings.value("cbSize", 0).toUInt();
    acmform->nAvgBytesPerSec = (DWORD)settings.value("nAvgBytesPerSec", 0).toUInt();
    acmform->nBlockAlign = (WORD)settings.value("nBlockAlign", 0).toUInt();
    acmform->nChannels = (WORD)settings.value("nChannels", 0).toUInt();
    acmform->nSamplesPerSec = (DWORD)settings.value("nSamplesPerSec", 0).toUInt();
    acmform->wBitsPerSample = (WORD)settings.value("wBitsPerSample", 0).toUInt() ;
    acmform->wFormatTag = (WORD)settings.value("wFormatTag", 0).toUInt() ;
    settings.endGroup();
#endif

    int tWidth=settings.value("WinWidth",-1).toInt();
    int tHeight=settings.value("WinHeight",-1).toInt();

    if (tWidth!=-1 && tHeight!=-1)
        this->resize(QSize(tWidth,tHeight));

}

void MainWindow::writeSettings()
{
    QSettings settings("FBytes.com", PLAYER_NAME);
#ifdef Q_OS_WIN
//    // ACM Settings ... is there a shorter way to read/write a structure ?
    settings.beginGroup("ACM");
    settings.setValue("cbSize", acmform->cbSize) ;
    settings.setValue("nAvgBytesPerSec", (uint)acmform->nAvgBytesPerSec) ;
    settings.setValue("nBlockAlign", acmform->nBlockAlign);
    settings.setValue("nChannels", acmform->nChannels) ;
    settings.setValue("nSamplesPerSec", (uint)acmform->nSamplesPerSec) ;
    settings.setValue("wBitsPerSample", acmform->wBitsPerSample) ;
    settings.setValue("wFormatTag", acmform->wFormatTag) ;
    settings.endGroup();
#endif

    settings.setValue("WinWidth",this->width());
    settings.setValue("WinHeight",this->height());
}


void MainWindow::ResetSettings(){
    QSettings settings("FBytes.com", PLAYER_NAME);
    settings.remove("UID");
}


UINT MainWindow::DoEvents(){
    QCoreApplication::sendPostedEvents();
    //NOTE: Signals emitted from the GUI are processed independantly from the event (msg?) loop
    return 0;
}


void MainWindow::updateActions(){
    //qDebug()<<"updateActions";
    bool hasSelection = !radioTree->selectionModel()->selection().isEmpty();
    removeRowAction->setEnabled(hasSelection);

    bool hasCurrent = radioTree->selectionModel()->currentIndex().isValid();
    insertRowAction->setEnabled(hasCurrent);

    if (!isPlaying){
        bool tCanPlay=false;
        if (hasCurrent){
            TreeItem* curItem=model->getItem(proxyModel->mapToSource(radioTree->selectionModel()->currentIndex()));
            if (!curItem->getIsFolder())
                tCanPlay=true;
        }
        pbPlay->setEnabled(tCanPlay);
    }
    else{
        pbPlay->setEnabled(true);
    }
}


void MainWindow::editRadio(){
    qDebug()<<"MainWindow::editRadio";
    //QModelIndex index = radioTree->selectionModel()->currentIndex();
    QModelIndex index = proxyModel->mapToSource(radioTree->selectionModel()->currentIndex());


    //TreeItem* curItem=model->getItem(radioTree->selectionModel()->currentIndex());
    TreeItem* curItem=model->getItem(index);

    RadioCL* pCurRadioRec=curItem->getRadio();

    RadioEdit* RadioEditForm=new RadioEdit(pCurRadioRec,this);
    if (RadioEditForm->exec()==QDialog::Accepted){
        curItem->setData(0,pCurRadioRec->GetTitle());
        model->saveToDisk();
    }

}


void MainWindow::insertSubfolder(){
    qDebug()<<"MainWindow::insertSubfolder";
    QModelIndex index;

    if (isRootOperation){
        qDebug()<<"Parent should be root";
        //model->index=NULL;
    }
    else{
        index = proxyModel->mapToSource(radioTree->selectionModel()->currentIndex());
    }

    QModelIndex indexToIns;
    if (!model->getItem(index)->getIsFolder()){
        qDebug()<<"Adding to parent";
        indexToIns=index.parent();
    }
    else{
        qDebug()<<"Adding subfolder to this position";
        indexToIns=index;
    }


    if (model->columnCount(indexToIns) == 0) {
        if (!model->insertColumn(0, indexToIns))
            return;
    }

    if (!model->insertRow(0, indexToIns)){
        qDebug()<<"Unable to insert row";
        return;
    }

    QModelIndex child = model->index(0, 0, indexToIns);
    model->setData(child, QVariant("New folder"), Qt::EditRole);
    radioTree->selectionModel()->setCurrentIndex(proxyModel->mapFromSource(model->index(0, 0, indexToIns)),
                                            QItemSelectionModel::ClearAndSelect);
    radioTree->edit(proxyModel->mapFromSource(model->index(0, 0, indexToIns)));

    updateActions();

    model->saveToDisk();
}


void MainWindow::insertStation()
{
    qDebug()<<"insertStation";

    RadioCL* newRadio=new RadioCL("",new QStringList());

    RadioEdit* RadioEditForm=new RadioEdit(newRadio,this);
    int tResult=RadioEditForm->exec();
    if (tResult==QDialog::Rejected){
        delete newRadio;
        return;
    }

    AddStation(radioTree->selectionModel()->currentIndex(),newRadio);
}


void MainWindow::AddStation(QModelIndex pParent,RadioCL* pRadio){

    qDebug()<<"MainWindow::AddStation  "<<pRadio->GetTitle();

    QModelIndex origIndex=proxyModel->mapToSource(pParent);
    QModelIndex indexToIns;
    if (model->getItem(origIndex)->getIsFolder()){
        qDebug()<<"Adding to this position";
        indexToIns=origIndex;
    }
    else{
        qDebug()<<"Adding to parent";
        indexToIns=origIndex.parent();
    }

    if (!model->insertRow(0, indexToIns)){
        qDebug()<<"Unable to add station";
        return;
    }

    updateActions();

    if (pRadio!=NULL){
        QModelIndex child = model->index(0, 0, indexToIns);
        model->setData(child, pRadio->GetTitle(), Qt::EditRole);
        model->getItem(child)->setRadio(pRadio);
        radioTree->selectionModel()->setCurrentIndex(proxyModel->mapFromSource(child),QItemSelectionModel::ClearAndSelect);
    }

    model->saveToDisk();
}


void MainWindow::removeRow(){
    qDebug()<<"removeRow";

    QModelIndex index = radioTree->selectionModel()->currentIndex();
    QModelIndex srcIdx= proxyModel->mapToSource(index);
    QString pTitleToDelete=model->getItem(srcIdx)->data(0).toString();

    if (!dDelConfirm){
        dDelConfirm=new DelConfirmDialogCL(this);
    }
    dDelConfirm->setRemovingItemStr(pTitleToDelete);
    if (dDelConfirm->exec()==QDialog::Accepted){
        if (radioTree->model()->removeRow(index.row(), index.parent()))
            updateActions();
        model->saveToDisk();
    }
    delete dDelConfirm;
    dDelConfirm=NULL;
}


RadioCL* MainWindow::getSelectedRadio(){
    QModelIndex tModelIndex = radioTree->selectionModel()->currentIndex();
    if (tModelIndex.isValid()){
        //TreeItem *item = ((TreeModel*)radioTree->model())->getItem(pModelIndex);
        QModelIndex origIndex=proxyModel->mapToSource(tModelIndex);
        TreeItem *item = model->getItem(origIndex);
        if (!item){
            qDebug()<<"Item is NULL";
            return(NULL);
        }
        else{
            qDebug()<<"Selected row="<<item->data(0);
            return(item->getRadio());
        }
    }
    else
        return(NULL);
}


void MainWindow::on_pbPlay_clicked()
{
    if (isPlaying){
        StopPlayback();
    }
    else{
        QModelIndex origIndex = proxyModel->mapToSource(radioTree->selectionModel()->currentIndex());
        playTreeItem(origIndex);
    }
}

void MainWindow::playTreeItem(QModelIndex pIndex){
    TreeItem *item = model->getItem(pIndex);
    if (!item)
        qDebug()<<"Item is NULL";
    else{
        if (item->getIsFolder())
            return;
        qDebug()<<"Row="<<item->data(0);
        currentRadio=item->getRadio();
        if (!currentRadio)
            qDebug()<<"Radio is NULL!";
        else{
            qDebug()<<"playTreeItem->Starting playback:"<<currentRadio->GetURL(0);
            StartPlayback();
        }
    }
}


void MainWindow::on_bnShowHistory_clicked()
{
    showHistory();
}


void MainWindow::on_pbQuit_clicked(){
    writeSettings();
    QApplication::exit();
}

void MainWindow::showSettings(){
    qDebug()<<"showSettings";
    QDesktopServices::openUrl("file:///"+AppPath);
}

void MainWindow::showRecPath(){
    qDebug()<<"showRecPath";
    QDesktopServices::openUrl("file:///"+recPath);
}


void MainWindow::showHelp(){
    qDebug()<<"showHelp";
    QString tHelpPath="file:///"+QDir::toNativeSeparators(QCoreApplication::applicationDirPath()+"/readme_eng.txt");
    qDebug()<<tHelpPath;
    QDesktopServices::openUrl(tHelpPath);
}

void MainWindow::showVisualization(){
#ifdef VISUALS
    qDebug()<<"showVisualization";

    //visWin->show();
    if (bigVisRunning){
        //visWinPlugin->
        bigVisRunning=false;

    }
    bigVis=true;
#endif
}


void MainWindow::activateMainWin(QString pMsg){
    qDebug()<<"activateMainWin  message received:"<<pMsg;

    if (pMsg.compare("Wakeup!")==0){
        show();
        raise();
    }
}

void MainWindow::OnConnectionStartFailed(){
    qDebug()<<"OnConnectionStartFailed";
    currentRadioURL++;
    if (currentRadioURL<currentRadio->GetURLs()->count()){
        if (dConnectionProgress)
            dConnectionProgress->ChangeURL(currentRadio->GetURL(currentRadioURL));
        qDebug()<<"trying "<<currentRadio->GetURL(currentRadioURL);
        StartURL(currentRadio->GetURL(currentRadioURL));
    }
    else{
        currentRadioURL=0;
        if (!streamsErrLoop){
            streamsErrLoop=true;

            //repull screams URLs from playlist
            if (currentRadio->GetPlaylist()!=""){
                dConnectionProgress->ChangeURL("repulling streams from playlist");
                PlaylistCL* tPlaylist=new PlaylistCL;
                RadioCL* newRadio=tPlaylist->Parse(QUrl(currentRadio->GetPlaylist()));
                if (newRadio){
                    if (newRadio->GetURLs()->count()>0){
                        qDebug()<<"URLs count="<<newRadio->GetURLs()->count();
                        currentRadio->SetURLs(newRadio->GetURLs());
                        model->saveToDisk();
                    }
                }
                delete tPlaylist;
                delete newRadio;
                qDebug()<<"trying "<<currentRadio->GetURL(currentRadioURL);
                StartURL(currentRadio->GetURL(currentRadioURL));
                return;
            }
        }
        if (dConnectionProgress!=NULL){
            dConnectionProgress->close();
            delete dConnectionProgress;
            dConnectionProgress=NULL;
        }
    }
}


void MainWindow::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        mLastMousePosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}


void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - mLastMousePosition);
        event->accept();
    }
}


void MainWindow::on_slVolume_valueChanged(int value){
    BASS_SetVolume((float)value/slVolume->maximum());    
}


void MainWindow::on_muteAction_triggered(){
    qDebug()<<"MainWindow::on_muteAction_triggered";
    if (isMuted){
        isMuted=false;
        BASS_SetVolume(savedVolume);
        pbMute->setIcon(QIcon(SPK_ICON_NOTMUTED));
    }
    else{
        isMuted=true;
        savedVolume=BASS_GetVolume();
        BASS_SetVolume(0);
        pbMute->setIcon(QIcon(SPK_ICON_MUTED));
    }
}


void MainWindow::showHistory(){
    qDebug()<<"MainWindow::on_showHistoryAction_triggered";
    if (!dShowHistory){
        dShowHistory=new ShowHistory(GetHistoryDir(),this);
        connect(this,SIGNAL(HistoryChanged(QString)),dShowHistory,SLOT(OnHistoryChanged(QString)));
    }
    dShowHistory->show();
}


void MainWindow::on_pbMute_clicked(){
    on_muteAction_triggered();
}


void MainWindow::CheckNewVersion(){
    qDebug()<<"Checking for new version";

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    if (Proxy!=""){
        manager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,Proxy,ProxyPort.toInt(),ProxyUser,ProxyPass));
    }
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(CheckNewVersionReply(QNetworkReply*)));

    QNetworkRequest req;
    req.setUrl(QUrl(VERSION_CHECK_URL));
    QString tAgent=PLAYER_VERCHECK_HTTPAGENT;
    tAgent+=RADIOLA_VERSION;
    req.setRawHeader("User-Agent",tAgent.toLocal8Bit());

    manager->get(req);
}


void MainWindow::CheckNewVersionReply(QNetworkReply* pRepl){
    qDebug()<<"MainWindow::CheckNewVersionReply";

    QString pVersionReply=pRepl->readAll();
    qDebug()<<"Version reply="<<pVersionReply;
    int tLastVer=pVersionReply.replace(".","").toInt();
    int tThisVer=QString(RADIOLA_VERSION).replace(".","").toInt();
    if (tLastVer>tThisVer){
        if (!newVersionAnimation){
            newVersionAnimation=new QMovie(":/images/radio_supernova_anim.gif");
            connect(newVersionAnimation,SIGNAL(frameChanged(int)),this,SLOT(setLogoButtonIcon(int)));
        }
        newVersionAnimation->start();
        setWindowTitle(QString(PLAYER_NAME)+" new version is available for download!");
        /*
        QMovie *movie = new QMovie(":/images/radio_supernova_anim.gif");
        movie->setScaledSize(QSize(50, 50));
        pbLogo->setMovie(movie);
        movie->start();
        */
    }
}

void MainWindow::setLogoButtonIcon(int pFrame){
    pbLogo->setIcon(QIcon(newVersionAnimation->currentPixmap()));
}


void MainWindow::on_radioTree_clicked(const QModelIndex &index){
    qDebug()<<"MainWindow::on_radioTree_clicked";
    singleClickTimer->start();
}


void MainWindow::singleClickTimeout(){
    if (!treeSelectionChanged){
        // click on the item already selected
        radioTree->edit(radioTree->selectionModel()->currentIndex());
    }
    else{
        treeSelectionChanged=false;
    }
}

void MainWindow::on_radioTree_activated(const QModelIndex &pModelIndex){
    qDebug()<<"MainWindow::on_radioTree_activated";
    if (pModelIndex.isValid()){
        QModelIndex origIndex=proxyModel->mapToSource(pModelIndex);
        if (!model->getItem(origIndex)->getIsFolder()){
            playTreeItem(origIndex);
        }
    }
}


void MainWindow::radioTreeSelectionChanges(const QModelIndex & pCurrent, const QModelIndex & pPrevious){
    treeSelectionChanged=true;
}


void MainWindow::radioDoubleClicked(QModelIndex pIndex){
    qDebug()<<"MainWindow::radioDoubleClicked";
    singleClickTimer->stop();
}


void MainWindow::on_renameRowAction_triggered(){
    qDebug()<<"MainWindow::renameRow";
    radioTree->edit(radioTree->selectionModel()->currentIndex());
}


void MainWindow::on_pbLogo_clicked(){
    QDesktopServices::openUrl(QUrl(QString(PLAYER_WEBSITE)));
}
