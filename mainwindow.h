#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QCloseEvent>
#include <QDir>
#include <QStatusBar>
#include <QVector>
#include <QPoint>
#include <QTime>
#include <QTreeWidgetItem>
#include <QSortFilterProxyModel>
#include <QPoint>
#include <QNetworkReply>
#include <QMovie>

#include "ui_mainwindow.h"

#include "defs.h"
#include "targetver.h"
#include "treemodel.h"
#include "treeitem.h"
#include "connectionprogress.h"
#include "treesortfilterproxymodel.h"
#include "viswincl.h"
#include "delconfirmdialogcl.h"
#include "showhistory.h"



#include "bass/bass.h"			// add bass.lib to linker
#include "bass/bassmix.h"		// add bassmix.lib to linker
#ifdef Q_OS_WIN
#include "bass/basswasapi.h"		// add basswasapi.lib to linker
#endif
#include "bass/bassenc.h"        // add bassenc.lib to linker input
#ifdef VISUALS
  #include "bass/bass_sfx.h"
#endif



//typedef  QVector<QVector<QString> >* RADIOLISTARRAY;

#define _T(x)     qPrintable(QObject::tr(x))
#ifndef min
#define min(a,b)  (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b)  (((a)<(b))?(b):(a))
#endif
#ifndef MAX_PATH
#define MAX_PATH  1024
#endif

#ifndef UINT
#define UINT   unsigned int
#endif
#ifndef BYTE
#define BYTE   unsigned char
#endif
#ifndef WORD
#define WORD   unsigned short
#endif
//#ifndef DWORD
//#define DWORD  unsigned long
//#endif
#ifndef LPSTR
#define LPSTR  char *
#endif
#ifndef LPCSTR
#define LPCSTR  const char *
#endif
#ifndef COLORREF
#define COLORREF  DWORD
#endif

#ifndef Q_OS_WIN
#define _strnicmp  strncasecmp
#endif

#ifndef Q_OS_WIN
#define _strdup  strdup
#endif

template<class num> int Round(const num& v)
{
    if( v >= 0.0 )
        return int(v + 0.5);
    if( v - int(v) == -0.5 )
        return int(v);
    return int(v - 0.5);
}


class MyThread;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void on_Error(const char* es, int ec, bool bModal);
    UINT DoEvents();
    UINT MsgSleep(const int& milsec);
    void  MsgBox(QString _msg, QString _title=QString(), bool bModal=true, int xPos=-1, int yPos=-1, bool bParent=true);
    DWORD ErrorMsgBox(LPCSTR lpszFunction, DWORD err_code=0, bool GetError=true, bool ErrCode_is_HRESULT=false);
    void Error(const char *es, int ec=-1234, bool bModal=true);
    void StartURL(QString pRadioURL);
    void INITDIALOG();

    QTimer *Spectrum_timer;
    QImage *pSpec;

    bool isPlaying;
    bool syncLost;

    bool   Finished_Init, AllowInit;
    QMessageBox  *m_msgbox;
    //QString	  m_CurDir_App;
    QString RadioUID;

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    //void mouseReleaseEvent(QMouseEvent* event);

#ifdef Q_OS_WIN
    WAVEFORMATEX     *acmform, *acmduncil ;			// ACM codec format
    DWORD	      acmformlen ;			// ACM codec format size
    bool  DoACM_Dialog(bool Init=false);
#endif


public slots:

    void on_UpdTrackInfo(QString);
    void on_UpdRadioInfo(QString,QString, QString, QString);
    void on_PlaybackStarts();
    void on_SyncLost();

    void updateActions();
    void AddStation(QModelIndex pParent,RadioCL* pRadio);

    void exitPlayer();
    void showAbout();
    void pingReply(QNetworkReply* pNetRepl);

    void contextMenuRequest(QPoint pReqPos);
    void activateMainWin(QString pMsg);

private slots:    
    void CheckNewVersionReply(QNetworkReply* pRepl);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void prebufTimeout();
    void updateTimeout();
    void updateBufferStatus();
    void singleClickTimeout();
    void pingRadio();
    void specTimeout();
    void insertSubfolder();
    void insertStation();
    void removeRow();
    void editRadio();
    void showSettings();
    void showRecPath();
    void showHelp();
    void showVisualization();
    void OnConnectionStartFailed();
    void on_pbPlay_clicked();
    void on_bnShowHistory_clicked();
    void filterRegExpChanged();
    void on_pbQuit_clicked();
    void on_specButton_clicked();
    void on_pbRecord_clicked();
    void on_ClickRecord();
    void on_slVolume_valueChanged(int value);
    void on_muteAction_triggered();
    void showHistory();
    void on_pbMute_clicked();
    void radioTreeSelectionChanges(const QModelIndex & pCurrent, const QModelIndex & pPrevious);
    void radioDoubleClicked(QModelIndex);

    void on_radioTree_clicked(const QModelIndex &index);

    void on_radioTree_activated(const QModelIndex &index);
    void on_renameRowAction_triggered();

    void on_pbLogo_clicked();
    void setRecButtonIcon(int pFrame);
    void setLogoButtonIcon(int pFrame);

protected:
    bool isRootOperation;
    QMovie* recButtonAnimation;
    QMovie* newVersionAnimation;
    bool treeSelectionChanged;
    bool isRecording;
    QString visualPlugin;
    QString recPath;


    void onRecordingStarts();
    void onRecordingStops();
    void closeEvent(QCloseEvent *event);


signals:
    void ConnectionProgressUpdate(int pProgress);
    void ConnectionEstablished();
    void ConnectionFailed();
    void HistoryChanged(QString pText);


private:
    //Ui::MainWindow *ui;

    TreeModel *model;
#ifdef VISUALS
    //HSFX visPlugin;
    HSFX visWinPlugin;
    VisWinCL* visWin;
    bool bigVis;
    bool bigVisRunning;
    int LastVisWidth;
    int LastVisHeight;
    bool isVisResized;
    bool isVisRendering;
    HDC visWinHDC;
#endif

    // the following are for the Spectrum Display in the Audio Settings Dialog
    BYTE* specbuf;
    int specmode;
    int	specpos; // spectrum mode (and marker pos for 2nd mode)

    char proxyStrChar[256];

    bool mMoving;
    QPoint mLastMousePosition;


    float savedVolume;
    bool isMuted;

    void createActions();
    void createTrayIcon();
    void setIcon(int index);

    int GetBufferStatus();
    void setupModelData();
    void setupRadioTree();
    void clearSpec();

    void initBitRate();
    void initOggQualityMap();
    void setBitrateDisplay(QString pBitRate);
    void setAudioQuaDisplay(QString pBitRate);

    void playTreeItem(QModelIndex pIndex);
    void setShadow(QPushButton* pButton);

    QTimer *Update_timer ;
    QTimer *PreBuf_timer ;
    QTimer *Ping_timer ;
    QTimer *BufferStatusUpdate_timer;
    QTimer *singleClickTimer;

    QVector<QVector<QString> > radioList;

    QAction *aboutAction;
    QAction *radioAction;
    QAction *sponsorAction;
    QAction *partnerAction;
    QAction *quitAction;
    QAction *autorunOptionsAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QMenu *optionsMenu;

    QString pingString;
    RadioCL* currentRadio;
    int currentRadioURL;
    bool streamsErrLoop;
    QString currentTrack;

    QStringList currentSessionHistory;
    QFile historyFile;

    QTime PrevBitrateTime;
    QWORD PrevBitratePos;
    int bitRateAvg;
    int bitRateCurAvg;

    ConnectionProgress* dConnectionProgress;
    DelConfirmDialogCL* dDelConfirm;
    ShowHistory* dShowHistory;
    QNetworkAccessManager *netManager;

    TreeSortFilterProxyModel *proxyModel;
    QTreeView *proxyView;

    void CheckNewVersion();

    void StopTimers();
    void StartTimers();

    void Cleanup();
    bool InitBassErrorMap();
    void on_MainWindow_close();
    void readSettings();
    void writeSettings();
    void ResetSettings();
    void on_MainWindow_clickOff();
    QString GetHistoryDir();

    void AddToHistory(QString pText, bool pSeparate = false);

    void StartPlayback();
    void StopPlayback();
    void StopRecording();
    void cleanRadioInfo();
    RadioCL* getSelectedRadio();


    friend class MyThread;
    friend class QThread;

};

#endif // MAINWINDOW_H
