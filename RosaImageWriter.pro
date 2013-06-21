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

win32 {
	SOURCES += platform_win.cpp
}
linux {
	SOURCES += platform_lin.cpp
}
macx {
    OBJECTIVE_SOURCES += platform_mac.mm
    ICON = res/icon-rosa.icns
}

HEADERS  += maindialog.h \
    imagewriter.h \
    common.h \
    platform.h \
    externalprogressbar.h \
    physicaldevice.h \
    usbdevicemonitor.h \
    usbdevice.h

FORMS    += maindialog.ui

RESOURCES += \
    RosaImageWriter.qrc

# The following variables can be used for automatic VERSIONINFO generating,
# but unfortunately it is impossible to use them together with RC_FILE or RES_FILE
# which is needed for specifying the executable file icon in Windows.
#VERSION = 2.3.0.0
#QMAKE_TARGET_COMPANY = ROSA
#QMAKE_TARGET_PRODUCT = "ROSA Image Writer"
#QMAKE_TARGET_DESCRIPTION = "Tool for creating bootable ROSA installation USB flash drives"
#QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2013 ROSA"

win32 {
	RC_FILE += RosaImageWriter.rc
	msvc {
		LIBS += Ole32.lib OleAut32.lib
	}
	mingw {
		QMAKE_CXXFLAGS += -std=gnu++11
		LIBS += -lole32 -loleaut32 -luuid
	}
}
linux:gcc {
	LIBS += -ludev
	GCCSTRVER = $$system(g++ -dumpversion)
	GCCVERSION = $$split(GCCSTRVER, .)
	GCCV_MJ = $$member(GCCVERSION, 0)
	GCCV_MN = $$member(GCCVERSION, 1)
	greaterThan(GCCV_MJ, 3) {
		lessThan(GCCV_MN, 7) {
			QMAKE_CXXFLAGS += -std=gnu++0x
		}
		greaterThan(GCCV_MN, 6) {
			QMAKE_CXXFLAGS += -std=gnu++11
		}
	}
}
macx {
    QMAKE_CFLAGS = $$replace(QMAKE_CFLAGS, '-mmacosx-version-min=10.6', '-mmacosx-version-min=10.7')
    QMAKE_CXXFLAGS = $$replace(QMAKE_CXXFLAGS, '-mmacosx-version-min=10.6', '-mmacosx-version-min=10.7')
    QMAKE_LFLAGS = $$replace(QMAKE_LFLAGS, '-mmacosx-version-min=10.6', '-mmacosx-version-min=10.7')
    QMAKE_OBJECTIVE_CFLAGS = $$replace(QMAKE_OBJECTIVE_CFLAGS, '-mmacosx-version-min=10.6', '-mmacosx-version-min=10.7')

    QMAKE_CXXFLAGS += -std=c++0x -stdlib=libc++
    QMAKE_OBJECTIVE_CFLAGS += -std=c++0x -stdlib=libc++
    QMAKE_INCDIR += /System/Library/Frameworks/AppKit.framework/Headers /System/Library/Frameworks/Security.framework/Headers /System/Library/Frameworks/ServiceManagement.framework/Headers
    QMAKE_LFLAGS += -framework IOKit -framework Cocoa -framework Security
}

TRANSLATIONS = lang/ru_RU.ts
