#-------------------------------------------------
#
# Project created by QtCreator 2013-03-12T16:10:18
#
#-------------------------------------------------

QT       += core gui

QTPLUGIN += qico qwindows

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RosaImageWriter
TEMPLATE = app


SOURCES += main.cpp\
	maindialog.cpp \
    imagewriter.cpp \
    common.cpp

HEADERS  += maindialog.h \
    imagewriter.h \
    common.h

FORMS    += maindialog.ui

RESOURCES += \
    RosaImageWriter.qrc

# The following variables can be used for automatic VERSIONINFO generating,
# but unfortunately it's impossible to use them together with RC_FILE or RES_FILE
# which is needed for specifying the executable file icon.
#VERSION = 2.0.0.0
#QMAKE_TARGET_COMPANY = ROSA
#QMAKE_TARGET_PRODUCT = "ROSA Image Writer"
#QMAKE_TARGET_DESCRIPTION = "Tool for creating bootable ROSA installation USB flash drives"
#QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2013 ROSA"

RC_FILE += RosaImageWriter.rc

LIBS += Ole32.lib OleAut32.lib Wbemuuid.lib

QMAKE_LFLAGS_RELEASE += "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
