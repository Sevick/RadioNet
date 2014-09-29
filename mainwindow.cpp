#include <QDebug>
#include <QMessageBox>
#include <QDesktopServices>
#include <QtNetwork>
#include <QTime>
#include <QMenu>
#include <QFile>
#include <QNetworkProxy>
#include <QStandardItemModel>
#include <QShortcut>
#include <QDesktopServices>
#include "mainwindow.h"
//#include "ui_mainwindow.h"
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
    //,ui(new Ui::MainWindow)
{
    setupUi(this);

    Finished_Init = false;
    AllowInit = false;
    PreBuf_timer = new QTimer;
    Update_timer= new QTimer;
    Ping_timer= new QTimer;
    BufferStatusUpdate_timer= new QTimer;
    m_msgbox = NULL;
    isPlaying=false;
    syncLost=false;
    currentRadio=NULL;
    //currentRadioURL="";
    currentTrack="";
    //currentRadioTitle="";
    dConnectionProgress=NULL;
    currentRadioURL=-1;

    initBitRate();
    initOggQualityMap();

    qDebug()<<"Cheking BASS version";
    if (HIWORD(BASS_GetVersion())!=BASSVERSION) {
        qDebug()<<"An incorrect version of BASS.DLL was loaded";
        QMessageBox::warning(this,QString("warning"),QString("An incorrect version of BASS.DLL was loaded"));
    }

    BASS_SetConfigPtr(BASS_CONFIG_NET_AGENT, (void*) _T(PLAYER_HTTPAGENT));

    QSettings tSettings(AppPath+CONFIG_FILENAME,QSettings::IniFormat);
    Proxy=tSettings.value("proxy","").toString();
    ProxyPort=tSettings.value("proxyport","").toString();
    ProxyUser=tSettings.value("proxyuser","").toString();
    ProxyPass=tSettings.value("proxypass","").toString();

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

    if( !InitVersion() ) ErrorMsgBox(_T("\n Error Setting up Version strings \n"));
    if( !InitBassErrorMap() ) ErrorMsgBox(_T("\n Error setting up Error Msgs \n"));
    if (!BASS_Init(-1,44100,0,NULL,NULL)) {
        ErrorMsgBox(_T("\nUnable to initialize BASS library\n"), 86, false);
        exit(86);
    }

    connect(radioTree,SIGNAL(AddStationSig(QModelIndex,RadioCL*)),this,SLOT(AddStation(QModelIndex,RadioCL*)));

    qDebug()<<"Connecting timers signals ";
    connect(PreBuf_timer, SIGNAL(timeout()), this, SLOT(prebufTimeout()));
    connect(Update_timer, SIGNAL(timeout()), this, SLOT(updateTimeout()));
    connect(Ping_timer, SIGNAL(timeout()),this, SLOT(pingRadio()));
    connect(BufferStatusUpdate_timer, SIGNAL(timeout()),this, SLOT(updateBufferStatus()));

    qDebug()<<"Connecting mgh signals";
    connect( &mgh, SIGNAL(SendUpdTrackInfo(QString)), this, SLOT(on_UpdTrackInfo(QString)) );
    connect( &mgh, SIGNAL(SendUpdRadioInfo(QString,QString,QString,QString)), this, SLOT(on_UpdRadioInfo(QString,QString,QString,QString)));
    connect( &mgh, SIGNAL(SendSyncLost()), this, SLOT(on_SyncLost()) );
    connect( &mgh, SIGNAL(SendPlaybackStarts()), this, SLOT(on_PlaybackStarts()) );

    qDebug()<<"Creating actions";
    createActions();
    qDebug()<<"Creating tray icon";
    createTrayIcon();

    qDebug()<<"Setting tray icon";
    setIcon(PLAYER_STATUS_INACTIVE);
    trayIcon->setVisible(true);
    trayIcon->show();
    setWindowTitle(tr(PLAYER_NAME));


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

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), this, SLOT(showSettings()));
    new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(showHelp()));
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
    if (!index.isValid())
        return;
    QString tClickedTitle=((TreeModel*) radioTree->model())->getItem(index)->data(0).toString();
    qDebug()<<"ContextMenuRequested@"<<pReqPos.x()<<","<<pReqPos.y()<<"  Title@pos:"<<tClickedTitle;

    QMenu *menu=new QMenu(this);
    if (model->getItem(index)->getIsFolder()){
       menu->addAction(insertChildAction);
    }
    else{
        menu->addAction(editRadioAction);
    }
    menu->addAction(insertRowAction);
    menu->addAction(removeRowAction);

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

void MainWindow::radioDoubleClicked(QModelIndex pModelIndex){

    if (pModelIndex.isValid()){
        QModelIndex origIndex=proxyModel->mapToSource(pModelIndex);
        TreeItem *item = model->getItem(origIndex);
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
                qDebug()<<"Radio doubleclicked:"<<currentRadio->GetURL(0);
                StartPlayback();
            }
        }
    }
}

MainWindow::~MainWindow()
{
    historyFile.close();
    delete PreBuf_timer;
    delete Update_timer;
    delete Ping_timer;
    if( m_msgbox != NULL ) delete m_msgbox;
}

void MainWindow::on_MainWindow_close()
{
    //this->on_MainWindow_clickOff();
    writeSettings();
    //this->Cleanup();
    this->close();
}

void MainWindow::on_UpdRadioInfo(QString pRadioTitle,QString pRadioWeb, QString pGenre, QString pBPS){
    lStationNameTxt->setText(pRadioTitle);
    //lStationURL->setText(pRadioWeb);
    lStationURL->setText("<html><head><style type=text/css>"
                         "a:link {color:green; text-decoration:underline;}"
                         "a:hover {color:green; text-decoration:underline}"
                         "</style></head><body>"+pRadioWeb+"</a></body></html>");
    lGenreTxt->setText(pGenre);
    lAudioQuality->setText(pBPS+" kb/s");
}


void MainWindow::on_UpdTrackInfo(QString pTrackInfo){
    if (currentTrack!=pTrackInfo){
        qDebug()<<"Updating track info: "<<pTrackInfo;
        lTrackNameTxt->setText(pTrackInfo);
        AddToHistory(HISTORY_TAB+pTrackInfo);
        trayIcon->showMessage(currentRadio->GetTitle(),"Next track: "+pTrackInfo);
        trayIcon->setToolTip(pTrackInfo.replace("&","&&&"));
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
    isPlaying=true;
    pingRadio();
    initBitRate();
    StartTimers();
}


void MainWindow::StartPlayback(){
/*
    ConnectionProgress* dConnectionProgress=new ConnectionProgress;
    dConnectionProgress->setWindowModality(Qt::WindowModal);
    dConnectionProgress->show();
*/
    if (isPlaying){
        StopPlayback();
    }

    qDebug()<<"Starting playback";
    if (currentRadio){
        pbPlay->setText("Stop");

        dConnectionProgress=new ConnectionProgress(currentRadio->GetTitle());
        connect(this,SIGNAL(ConnectionProgressUpdate(int)),dConnectionProgress,SLOT(OnConnectionProgressUpdate(int)));
        connect(this,SIGNAL(ConnectionEstablished()),dConnectionProgress,SLOT(OnConnectionEstablished()));
        connect(this,SIGNAL(ConnectionFailed()),dConnectionProgress,SLOT(OnConnectionFailed()));

        currentRadioURL=0;
        dConnectionProgress->ChangeURL(currentRadio->GetURL(0));
        StartURL(currentRadio->GetURL(0));

        int tResult=dConnectionProgress->exec();
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


    //historyStream<<pText;
}


void MainWindow::StopPlayback(){
    qDebug()<<"Stopping playback";
    cleanRadioInfo();
    setIcon(PLAYER_STATUS_INACTIVE);
    pbPlay->setText("Play");
    trayIcon->setToolTip(PLAYER_NAME);
    on_MainWindow_clickOff();
    isPlaying=false;
    syncLost=false;
    StopTimers();
    initBitRate();
    updateActions();
}

void MainWindow::StopTimers(){
    Ping_timer->stop();
    PreBuf_timer->stop();
    BufferStatusUpdate_timer->stop();
    Update_timer->stop();
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

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        if (Proxy!=""){
            manager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,Proxy,ProxyPort.toInt(),ProxyUser,ProxyPass));
        }
        connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(replyFinished(QNetworkReply*)));


        QNetworkRequest req;
        req.setUrl(QUrl(pingString));
        req.setRawHeader("User-Agent",PLAYER_PING_HTTPAGENT);

        manager->get(req);
    }
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
    lAudioQuality->setText(pBitRate+" kb/s");
}

void MainWindow::setBitrateDisplay(QString pBitRate){
    lNetUsageTxt->setText(pBitRate+" kb/s");
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
        // build up display string from resource file information ... version, author, credits, legal question, etc.
        //     QString myAbout(_T("\n\n  netRadio QT Built Version 1.0.1.8      .\n\n  Copyright 2012 Zac-Labs  \n  Author: F.G.Zacharias   \n\n"));

        QString myAbout;
        myAbout.sprintf("\n %s by %s \n version %d.%d.%d.%d %s \n Copyright © %s \n Author: %s \n %s \n %s is Freeware, Please Give it Freely to others! \n ",
                        qPrintable(m_nR_version._ProductName) , qPrintable(m_nR_version._CompanyName) ,
                        m_nR_version._wVerMajor , m_nR_version._wVerMinor ,
                        m_nR_version._wVerBuild , m_nR_version._wVerRevision ,
                        ((m_nR_version._bVerAlpha)? _T("Alpha") : ((m_nR_version._bVerBeta)? _T("Beta") : _T(""))),
                        qPrintable(m_nR_version._LegalCopyright), qPrintable(m_nR_version._Author),
                        qPrintable(m_nR_version._Comments) ,
                        qPrintable(m_nR_version._ProductName) );
        myAbout.sprintf("Radiola v.%d.%d%d\n(c) 2014 FBytes.com\n\nBuilt with QT SDK and Bass audio library\nspecial thanks to F.G.Zacharias",
                        m_nR_version._wVerMajor , m_nR_version._wVerMinor , m_nR_version._wVerBuild);
        QMessageBox::about(this, _T(PLAYER_NAME), myAbout);
}


void MainWindow::goPartURL(){
        qDebug()<<"goPartnerURL";
        QString link = "";
        QDesktopServices::openUrl(QUrl(link));
}


void MainWindow::goSponsorURL(){
        qDebug()<<"goSponsorURL";
        QString link = "";
        QDesktopServices::openUrl(QUrl(link));
}


void MainWindow::goRadioURL(){
        qDebug()<<"goRadioURL";
        QString link = "";
        QDesktopServices::openUrl(QUrl(link));
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
/*
    partnerAction = new QAction(tr("Sochi.Camera"), this);
    connect(partnerAction, SIGNAL(triggered()), this, SLOT(goPartURL()));

    radioAction = new QAction(tr("ElectroWave.FM"), this);
    connect(radioAction, SIGNAL(triggered()), this, SLOT(goRadioURL()));

    sponsorAction = new QAction(tr("Business-Svyaz"), this);
    connect(sponsorAction, SIGNAL(triggered()), this, SLOT(goSponsorURL()));
*/
}

void MainWindow::createTrayIcon(){
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(aboutAction);
    /*
    trayIconMenu->addAction(radioAction);
    trayIconMenu->addAction(sponsorAction);
    trayIconMenu->addAction(partnerAction);
    */
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

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
}

void MainWindow::writeSettings()
{

}


void MainWindow::ResetSettings(){
    QSettings settings("FBytes.com", PLAYER_NAME);
    settings.remove("UID");
}


bool MainWindow::InitVersion()
{
    // this data is also/originally held in netradio.rc ... how to access it under Linux/Mac ?
    m_nR_version._wVerMajor = 1 ;
    m_nR_version._wVerMinor = 0 ;
    m_nR_version._wVerBuild = 3 ;
    m_nR_version._wVerRevision = 0;
    m_nR_version._bVerBeta = true;
    m_nR_version._bVerAlpha = false;
    m_nR_version._Comments = _T("Built using Qt SDK and Bass audio library");
    m_nR_version._CompanyName = _T("FBytes.com");
    m_nR_version._Author = _T("V.Vasilyev");
    m_nR_version._LegalCopyright = _T("09.2014");
    m_nR_version._LegalTrademarks = _T("(c) 2014 Fbytes.com");
    m_nR_version._ProductName = _T(PLAYER_NAME);

    QString setver;
    setver.sprintf("%d.%d.%d.%d", m_nR_version._wVerMajor, m_nR_version._wVerMinor,
                   m_nR_version._wVerBuild, (int)m_nR_version._wVerRevision);
    QApplication::setApplicationVersion(setver);
    return true;
}

UINT MainWindow::DoEvents()
{
    QCoreApplication::sendPostedEvents();
    //NOTE: Signals emitted from the GUI are processed independantly from the event (msg?) loop
    return 0;
}


void MainWindow::updateActions()
{
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


void MainWindow::insertSubfolder()
{
    qDebug()<<"MainWindow::insertSubfolder";
    QModelIndex index = proxyModel->mapToSource(radioTree->selectionModel()->currentIndex());

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index)){
        qDebug()<<"Unable to insert row";
        return;
    }

    QModelIndex child = model->index(0, 0, index);
    model->setData(child, QVariant("New folder"), Qt::EditRole);
    radioTree->selectionModel()->setCurrentIndex(proxyModel->mapFromSource(model->index(0, 0, index)),
                                            QItemSelectionModel::ClearAndSelect);
    radioTree->edit(child);

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
        qDebug()<<"Unable to insert row";
        return;
    }

    updateActions();

    if (pRadio!=NULL){
        QModelIndex child = model->index(0, 0, indexToIns);
        model->setData(child, pRadio->GetTitle(), Qt::EditRole);
        model->getItem(child)->setRadio(pRadio);
    }

    model->saveToDisk();
}


void MainWindow::removeRow()
{
    qDebug()<<"removeRow";
    QModelIndex index = radioTree->selectionModel()->currentIndex();

    //TreeModel *model = (TreeModel*) radioTree->model();
    if (radioTree->model()->removeRow(index.row(), index.parent()))
        updateActions();

    model->saveToDisk();
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
        StartPlayback();
    }
}


void MainWindow::on_bnShowHistory_clicked()
{
    qDebug()<<"Show history";
    ShowHistory* wShowHistory=new ShowHistory(GetHistoryDir(),this);
    connect(this,SIGNAL(HistoryChanged(QString)),wShowHistory,SLOT(OnHistoryChanged(QString)));
    wShowHistory->show();

}


void MainWindow::on_pbQuit_clicked(){
    QApplication::exit();
}

void MainWindow::showSettings(){
    qDebug()<<"showSettings";
    QDesktopServices::openUrl("file:///"+AppPath);
}

void MainWindow::showHelp(){
    qDebug()<<"showHelp";
    QString tHelpPath="file:///"+QDir::toNativeSeparators(QCoreApplication::applicationDirPath()+"/readme_eng.txt");
    qDebug()<<tHelpPath;
    QDesktopServices::openUrl(tHelpPath);
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

        //!!! repull screams URLs from playlist
        if (currentRadio->GetPlaylist()!=""){
            dConnectionProgress->ChangeURL("repulling streams from playlist");
            PlaylistCL* tPlaylist=new PlaylistCL;
            RadioCL* newRadio=tPlaylist->Parse(QUrl(currentRadio->GetPlaylist()));
            currentRadio->SetURLs(newRadio->GetURLs());
            model->saveToDisk();
            delete tPlaylist;
            delete newRadio;
            qDebug()<<"trying "<<currentRadio->GetURL(currentRadioURL);
            StartURL(currentRadio->GetURL(currentRadioURL));
        }
        else{
            if (dConnectionProgress!=NULL){
                dConnectionProgress->close();
                delete dConnectionProgress;
                dConnectionProgress=NULL;
            }
        }
    }
}
