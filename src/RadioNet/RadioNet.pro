#-------------------------------------------------
#
# Project created by QtCreator 2014-08-27T05:53:53
#
#-------------------------------------------------

QT       += core gui network widgets


include(C:/Work/Qt/qtsingleapplication/src/qtsingleapplication.pri)

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
    winsystemcommand.cpp \
    playlistcl.cpp


RC_ICONS += radiola.ico

FORMS   = mainwindow.ui \
    radioedit.ui \
    connectionprogress.ui \
    test.ui \
    showhistory.ui

HEADERS  += \
    bass/bass.h \
    bass/bassenc.h \
    bass/bassmix.h \
    bass/basswasapi.h \
    targetver.h \
    mainwindow.h \
    mythread.h \
    myglobalhandler.h \
    climitsingleinstance.h \
              treeitem.h \
              treemodel.h \
    defs.h \
    radioedit.h \
    rtreeview.h \
    radiocl.h \
    connectionprogress.h \
    showhistory.h \
    treesortfilterproxymodel.h \
    winsystemcommand.h \
    playlistcl.h


win32:HEADERS += MF_nR_Bridge.h

LIBS += bass.lib \
    bassenc.lib \
    basswasapi.lib \
    bassmix.lib



RESOURCES += \
    Radiola.qrc

OTHER_FILES += \
    Radiola_resource.rc
