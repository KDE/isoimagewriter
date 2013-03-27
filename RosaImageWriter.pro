#-------------------------------------------------
#
# Project created by QtCreator 2013-03-12T16:10:18
#
#-------------------------------------------------

QT       += core gui

QTPLUGIN += qico

win32 {
	QTPLUGIN += qwindows
}
linux {
	QTPLUGIN += qxcb
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RosaImageWriter
TEMPLATE = app


SOURCES += main.cpp\
	maindialog.cpp \
    imagewriter.cpp \
    common.cpp \
    externalprogressbar.cpp \
    physicaldevice.cpp \
    usbdevicemonitor.cpp

HEADERS  += maindialog.h \
    imagewriter.h \
    common.h \
    platform.h \
    externalprogressbar.h \
    physicaldevice.h \
    usbdevicemonitor.h

FORMS    += maindialog.ui

RESOURCES += \
    RosaImageWriter.qrc

# The following variables can be used for automatic VERSIONINFO generating,
# but unfortunately it's impossible to use them together with RC_FILE or RES_FILE
# which is needed for specifying the executable file icon.
#VERSION = 2.1.0.0
#QMAKE_TARGET_COMPANY = ROSA
#QMAKE_TARGET_PRODUCT = "ROSA Image Writer"
#QMAKE_TARGET_DESCRIPTION = "Tool for creating bootable ROSA installation USB flash drives"
#QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2013 ROSA"

win32 {
	RC_FILE += RosaImageWriter.rc
	QMAKE_LFLAGS_RELEASE += "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
}

win32:msvc {
	LIBS += Ole32.lib OleAut32.lib
}
win32:mingw {
	QMAKE_CXXFLAGS += -std=gnu++11
	LIBS += -lole32 -loleaut32 -luuid
}
linux:gcc {
	QMAKE_CXXFLAGS += -std=gnu++11
	LIBS += -ludev
}

TRANSLATIONS = lang/ru_RU.ts
