#-------------------------------------------------
#
# Project created by QtCreator 2014-08-27T05:53:53
#
#-------------------------------------------------

QT       += core gui network widgets
win32:  QT+=winextras

macx:ICONS=radiola.icns
win32:RC_ICONS = radiola.ico
win32:RC_FILE = Radiola_resource.rc


TARGET = Radiola
TEMPLATE = app

DEFINES += IN_NETRADIO

#CONFIG += console

SOURCES += main.cpp\
        mainwindow.cpp \
        targetver.cpp \
        netradio.cpp \
    myglobalhandler.cpp \
              treeitem.cpp \
              treemodel.cpp \
    radioedit.cpp \
    rtreeview.cpp \
    radiocl.cpp \
    connectionprogress.cpp \
    showhistory.cpp \
    treesortfilterproxymodel.cpp \
    playlistcl.cpp \  
    graphicsbutton.cpp \
    viswincl.cpp \
    delconfirmdialogcl.cpp

win32:SOURCES += winsystemcommand.cpp


FORMS   = mainwindow.ui \
    radioedit.ui \
    connectionprogress.ui \
    showhistory.ui \
    viswincl.ui \
    mainwindow1.ui \
    delconfirmdialogcl.ui

HEADERS  += \
    bass/bass.h \
    bass/bassenc.h \
    bass/bassmix.h \
    bass/bass_aac.h \
    targetver.h \
    mainwindow.h \
    mythread.h \
    myglobalhandler.h \
              treeitem.h \
              treemodel.h \
    defs.h \
    radioedit.h \
    rtreeview.h \
    radiocl.h \
    connectionprogress.h \
    showhistory.h \
    treesortfilterproxymodel.h \
    playlistcl.h \
    graphicsbutton.h \
    viswincl.h \
    delconfirmdialogcl.h

win32:HEADERS += \
    winsystemcommand.h \
    bass/basswasapi.h \
    bass/bass_sfx.h \
    bass/Plugin.h

win32:LIBS += bass.lib \
    bassenc.lib \
    basswasapi.lib \
    bassmix.lib \
    BASS_SFX.lib \
    bass_aac.lib

macx: LIBS += -L$$PWD/bass/ -lbass
macx: LIBS += -L$$PWD/bass/ -lbassenc
macx: LIBS += -L$$PWD/bass/ -lbassmix

RESOURCES += \
    Radiola.qrc

OTHER_FILES += \
    Radiola_resource.rc

INCLUDEPATH += $$PWD/bass
DEPENDPATH += $$PWD/bass

include(../qtsingleapplication/src/qtsingleapplication.pri)

