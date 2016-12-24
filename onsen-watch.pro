#-------------------------------------------------
#
# Project created by QtCreator 2016-09-26T23:55:15
#
#-------------------------------------------------

VERSION = 0.1.2

QT       += core gui network

packagesExist(QtMultimedia) {
    QT += multimedia
    DEFINES += USE_QT_MULTIMEDIA
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = onsen-watch
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    apionsen.cpp \
    datastore.cpp \
    show.cpp \
    showlistmodel.cpp \
    showviewdelegate.cpp \
    showlistproxymodel.cpp \
    downloadmanager.cpp \
    settingsdialog.cpp \
    jumpstyle.cpp

HEADERS  += mainwindow.h \
    apionsen.h \
    globs.h \
    datastore.h \
    show.h \
    showlistmodel.h \
    showviewdelegate.h \
    showlistproxymodel.h \
    downloadmanager.h \
    settingsdialog.h \
    jumpstyle.h

FORMS    += mainwindow.ui \
    settingsdialog.ui

TRANSLATIONS += onsen_en.ts
TRANSLATIONS += onsen_ja.ts

win32 {
    INCLUDEPATH += C:/Libs/include
    DEPENDPATH += C:/Libs/include

    LIBS += -LC:/Libs/lib_492/
    LIBS += -llibxml2

    RC_ICONS += main.ico
} else:unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libxml-2.0
}

RESOURCES += res.qrc

target.path = /usr/local/bin
INSTALLS += target

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
