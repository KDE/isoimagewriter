#include <QApplication>
#include <QTranslator>

#include "common.h"
#include "maindialog.h"

#if !defined(Q_OS_WIN32) && !defined(Q_OS_LINUX)
#error Unsupported platrofm!
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator appTranslator;
    appTranslator.load(QLocale::system().name(), "lang");
    a.installTranslator(&appTranslator);

#if defined(Q_OS_WIN32)
    // CoInitialize() seems to be called by Qt automatically, so only set security attributes
    HRESULT res = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
    if (res != S_OK)
    {
        printf("CoInitializeSecurity failed! (Code: 0x%08lx)\n", res);
        return res;
    }
#endif

    MainDialog w;
    // MainDialog implements QAbstractNativeEventFilter interface and processes WM_DEVICECHANGE,
    // register it for filtering native Windows events
    a.installNativeEventFilter(&w);
    w.show();
    
    return a.exec();
}
