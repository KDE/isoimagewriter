#-------------------------------------------------
#
# Project created by QtCreator 2013-03-12T16:10:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RosaImageWriter
TEMPLATE = app


SOURCES += main.cpp\
        maindialog.cpp \
    progressdialog.cpp \
    imagewriter.cpp

HEADERS  += maindialog.h \
    progressdialog.h \
    imagewriter.h

FORMS    += maindialog.ui \
    progressdialog.ui

RESOURCES += \
    RosaImageWriter.qrc

LIBS += Ole32.lib OleAut32.lib Wbemuuid.lib
