#-------------------------------------------------
#
# Project created by QtCreator 2015-12-10T15:00:29
#
#-------------------------------------------------

QT       += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StudentHelperClient
TEMPLATE = app
CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp\
        ../StudentHelperServer/shquery.cpp \
    ../StudentHelperServer/filetreeitem.cpp \
    ../StudentHelperServer/studenthelpercontent.cpp \
    filedialog.cpp \
    searcherwidget.cpp \
    filebrowserwidget.cpp \
    printcompositor.cpp \
    printerwidget.cpp \
    shclientwidget.cpp \
    shclientsettings.cpp \
    cachingbase.cpp

HEADERS  += mainwindow.h\
        ../StudentHelperServer/shquery.h \
    ../StudentHelperServer/studenthelpercommon.h \
    ../StudentHelperServer/studenthelpercontent.h \
    ../StudentHelperServer/filetreeitem.h \
    filedialog.h \
    filebrowserwidget.h \
    searcherwidget.h \
    printerwidget.h \
    printcompositor.h \
    shclientwidget.h \
    shclientsettings.h \
    cachingbase.h

FORMS    += mainwindow.ui \
    filedialog.ui \
    searcherwidget.ui \
    printerwidget.ui \
    filewidget.ui \
    filebrowserwidget.ui \
    folderwidget.ui \
    shclientwidget.ui

RESOURCES += \
    icons.qrc

DISTFILES += \
    StudentHelperClient.pro.user
