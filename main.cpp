#include <comutil.h>

#include <QApplication>

#include "maindialog.h"

#ifndef Q_OS_WIN32
#error Only Win32 is supported!
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HRESULT res = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
    if (res != S_OK)
    {
        printf("CoInitializeSecurity failed! (Code: 0x%08x)\n", res);
        return res;
    }
    MainDialog w;
    w.show();
    
    return a.exec();
}
