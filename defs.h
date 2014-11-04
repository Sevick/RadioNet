#ifndef DEFS_H
#define DEFS_H

#include <QString>

//#define VISUALS
#ifdef Q_OS_MAC
    #undef VISUALS
#endif
#ifdef Q_OS_LINUX
    #undef VISUALS
#endif

#define UPDATE_TIME 100     // was 40
#define SPECTRUM_TIME 50
#define PING_TIME 60000
#define RETRY_TIME 500
#define STREAM_CONNECT_TIMEOUT 3000
#define NOSOUND_SPEC_DELAY 1000

#define VIS_WIN_PLUG "plugins\\vis_geis.dll"
//"plugins\\vis_milk2.dll"
//"wmpvis\\vis_geis.dll"

#define RADIOLA_VERSION "2.0.1"

//#define PLAYER_HTTPAGENT "Mozilla/5.0"
#define PLAYER_HTTPAGENT "Radiola player 2.0.1"
#ifdef Q_OS_WIN
    #define PLAYER_PING_HTTPAGENT "RadiolaPingWin"
#elif Q_OS_MAC
    #define PLAYER_PING_HTTPAGENT "RadiolaPingMac"
#elif Q_OS_LINUX
    #define PLAYER_PING_HTTPAGENT "RadiolaPingLinux"
#endif

#ifdef Q_OS_WIN
    #define PLAYER_VERCHECK_HTTPAGENT "RadiolaWinVerCheck "
#elif Q_OS_MAC
    #define PLAYER_VERCHECK_HTTPAGENT "RadiolaMacVerCheck "
#elif Q_OS_LINUX
    #define PLAYER_VERCHECK_HTTPAGENT "RadiolaLinuxVerCheck "
#endif


#define PLAYER_WEBSITE "http://fbytes.com"

#define BITRATE_AVG_SAMPLES (2*100/UPDATE_TIME)

#define PLAYER "Radiola"
#define PING_URL "http://s.fbytes.com/ping/ping.php"
#ifdef Q_OS_WIN
    #define VERSION_CHECK_URL "http://fbytes.com/radiola/win_lastversion_debug.txt"
#endif
#ifdef Q_OS_MAC
    #define VERSION_CHECK_URL "http://fbytes.com/radiola/mac_lastversion.txt"
#endif
#ifdef Q_OS_LINUX
    #define VERSION_CHECK_URL "http://fbytes.com/radiola/linux_lastversion.txt"
#endif

#define PLAYER_NAME "Radiola"
#define PLAYER_STATUS_INACTIVE 0
#define PLAYER_STATUS_ACTIVE   1
#define PLAYER_STATUS_ERROR    2

#define RADIOLIST_URL_COLUMN   2

#ifdef Q_OS_MAC
   #define MAINWIN_ICON "radiola.icns"
#else
   #define MAINWIN_ICON ":/images/radiola.png"
#endif

#ifdef Q_OS_OSX
    #define TRAY_ICON_ACTIVE ":/images/mac/radiola_tray_active.png"
    #define TRAY_ICON_INACTIVE ":/images/mac/radiola_tray_inactive.png"
    #define TRAY_ICON_ERROR ":/images/mac/radiola_tray_error.png"
#else
    #define TRAY_ICON_ACTIVE ":/images/win/radiola_tray_active.png"
    #define TRAY_ICON_INACTIVE ":/images/win/radiola_tray_inactive.png"
    #define TRAY_ICON_ERROR ":/images/win/radiola_tray_error.png"
#endif



#define SPK_ICON_NOTMUTED ":/images/green_speaker.png"
#define SPK_ICON_MUTED ":/images/green_speaker_muted.png"

#define CONNECTION_STATUS_FAILED 3

#define HISTORY_TAB "     "
#define PLAYLIST_TEMP   "~playlist.tmp"
#define CONFIG_FILENAME  "radiola.cfg"

#define SPECWIDTH 180 //368
// height (changing requires palette adjustments too)
#define SPECHEIGHT 127
#define DEFAULT_SPEC_MODE 1
#define SPECBANDS 20 //28

#define MENU_STYLE "background-color: rgb(212, 208, 200);\nborder: 2px solid #555;selection-color: green;selection-background-color: white; "

#define HISTORY_READBLOCK_SIZE 10240
#define HISTORY_AULOAD_SCROLL  20

extern QString AppPath;
extern QString Proxy;
extern QString ProxyPort;
extern QString ProxyUser;
extern QString ProxyPass;

#endif // DEFS_H
