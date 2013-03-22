#include <comutil.h>

#include <QApplication>
#include <QTranslator>

#include "maindialog.h"

// For now, only Windows platform is supported
// Non-cross-platform code:
//  1. USB devices enumeration;
//  2. reading/writig the image file;
//  3. unmounting of USB device volumes before writing;
//  4. CoInitializeSecurity in main();
//  5. WinAPI-specific headers, libraries, resources (icon, manifest), drag&drop MIME types (?).
//  6. ITaskbarList3 COM interface.
//  7. WM_DEVICECHANGE processing.
#ifndef Q_OS_WIN32
#error Only Win32 is supported!
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator appTranslator;
    appTranslator.load(QLocale::system().name(), "lang");
    a.installTranslator(&appTranslator);

    // CoInitialize() seems to be called by Qt automatically, so only set security attributes
    HRESULT res = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
    if (res != S_OK)
    {
        printf("CoInitializeSecurity failed! (Code: 0x%08lx)\n", res);
        return res;
    }

    MainDialog w;
    // MainDialog implements QAbstractNativeEventFilter interface and processes WM_DEVICECHANGE,
    // register it for filtering native Windows events
    a.installNativeEventFilter(&w);
    w.show();
    
    return a.exec();
}
