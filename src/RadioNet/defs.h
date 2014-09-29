#ifndef DEFS_H
#define DEFS_H

#include <QString>

#define PING_TIME 60000
#define RETRY_TIME 500
#define STREAM_CONNECT_TIMEOUT 30000

#define PLAYER_HTTPAGENT "Mozilla/5.0"
#define PLAYER_PING_HTTPAGENT "RadioPing"

#define BITRATE_AVG_SAMPLES 5

#define PLAYER "Radiola"
#define PING_URL "http://s.fbytes.com/ping/smth.txt"
#define PLAYER_NAME "Radiola"
#define PLAYER_STATUS_INACTIVE 0
#define PLAYER_STATUS_ACTIVE   1
#define PLAYER_STATUS_ERROR    2

#define RADIOLIST_URL_COLUMN   2


#define MAINWIN_ICON ":/images/radiola.png"

#ifdef Q_OS_WIN
    #define TRAY_ICON_ACTIVE ":/images/win/radiola_tray_active.png"
    #define TRAY_ICON_INACTIVE ":/images/win/radiola_tray_inactive.png"
    #define TRAY_ICON_ERROR ":/images/win/radiola_tray_error.png"
#endif

#ifdef Q_OS_DARWIN
    #define TRAY_ICON_ACTIVE ":/images/mac/radiola_tray_active.png"
    #define TRAY_ICON_INACTIVE ":/images/mac/radiola_tray_inactive.png"
    #define TRAY_ICON_ERROR ":/images/mac/radiola_tray_error.png"
#endif


/*
#define LINK_PARTNER "http://s.fbytes.com"
#define LINK_SPONSOR "http://fbytes.com"
#define LINK_RADIO   "http://di.fm"
*/

#define CONNECTION_STATUS_FAILED 3

#define HISTORY_TAB "     "
#define PLAYLIST_TEMP   "~playlist.tmp"
#define CONFIG_FILENAME  "radiola.cfg"

extern QString AppPath;
extern QString Proxy;
extern QString ProxyPort;
extern QString ProxyUser;
extern QString ProxyPass;

#endif // DEFS_H
