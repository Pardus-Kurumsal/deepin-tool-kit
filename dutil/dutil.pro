DCOMMON_DIR = $$PWD/../common
include($$DCOMMON_DIR/lib.pri)
include(dlog/dlog.pri)
include(private/private.pri)

QT -= gui
QT += multimedia

TARGET = dtkutil

DEFINES += LIBDTKUTIL_LIBRARY

HEADERS += \
    dutil_global.h \
    dpathbuf.h \
    dutil.h \
    ddesktopservices.h \
    dtrashmanager.h \
    dfilesystemwatcher.h \
    dfilewatcher.h \
    dfilewatchermanager.h \
    dbasefilewatcher.h

includes.path = $${DTK_INCLUDEPATH}/DUtil
includes.files += \
    $$PWD/DUtil \
    $$PWD/DPathBuf \
    $$PWD/*.h \
    $$PWD/dlog/DLog \
    $$PWD/dlog/CuteLogger_global.h \
    $$PWD/dlog/RollingFileAppender.h \
    $$PWD/dlog/Logger.h \
    $$PWD/dlog/LogManager.h \
    $$PWD/dlog/FileAppender.h \
    $$PWD/dlog/ConsoleAppender.h \
    $$PWD/dlog/AbstractStringAppender.h \
    $$PWD/dlog/AbstractAppender.h \
    $$PWD/DDesktopServices \
    $$PWD/DTrashManager \
    $$PWD/DBaseFileWatcher \
    $$PWD/DFileSystemWatcher \
    $$PWD/DFileWatcher \
    $$PWD/DFileWatcherManager

QMAKE_PKGCONFIG_NAME = DTK_UTIL
QMAKE_PKGCONFIG_DESCRIPTION = Deepin Tool Kit Util Module
QMAKE_PKGCONFIG_INCDIR = $$includes.path
QMAKE_PKGCONFIG_REQUIRES += dtkbase

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dbase/release/ -ldtkbase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dbase/debug/ -ldtkbase
else:unix: LIBS += -L$$OUT_PWD/../dbase/ -ldtkbase

INCLUDEPATH += $$PWD/../dbase
DEPENDPATH += $$PWD/../dbase

linux {
    qtHaveModule(dbus) {
        QT += dbus
        SOURCES += ddesktopservices_linux.cpp
    }

    CONFIG += link_pkgconfig
    PKGCONFIG += gsettings-qt

    SOURCES += \
        dtrashmanager_linux.cpp \
        dfilesystemwatcher_linux.cpp
} else:win* {
    SOURCES += \
        ddesktopservices_win.cpp \
        dfilesystemwatcher_win.cpp \
        dtrashmanager_win.cpp
} else:mac* {
    SOURCES += \
        ddesktopservices_win.cpp \
        dfilesystemwatcher_win.cpp \
        dtrashmanager_win.cpp
}

DISTFILES += \
    DUtil

SOURCES += \
    dfilewatcher.cpp \
    dfilewatchermanager.cpp \
    dbasefilewatcher.cpp
