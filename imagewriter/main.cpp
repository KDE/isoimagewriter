/*
 * Copyright 2016 ROSA
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QIcon>

#include <KAboutData>
#include <KLocalizedString>

#include "common.h"
#include "mainapplication.h"
#include "maindialog.h"
#include "usbdevicemonitor.h"

#if !defined(Q_OS_WIN32) && !defined(Q_OS_LINUX) && !defined(Q_OS_MAC)
#error Unsupported platform!
#endif

int main(int argc, char *argv[])
{
#if defined(Q_OS_MAC)
    // On Mac OS X elevated launch is treated as setuid which is forbidden by default -> enable it
    // TODO: Try to find a more "kosher" way, as well as get rid of deprecated AuthorizationExecuteWithPrivileges()
    QCoreApplication::setSetuidAllowed(true);
#endif

    MainApplication a(argc, argv);

    KLocalizedString::setApplicationDomain("imagewriter");
    KAboutData aboutData( QStringLiteral("imagewriter"),
                          i18n("Image Writer"),
                          "1.0", //QStringLiteral("PROJECT_VERSION 1.0"),
                          i18n("Write an ISO Image to a USB Disk"),
                          KAboutLicense::GPL,
                          i18n("Copyright (c) 2016 ROSA"));

    aboutData.addAuthor(i18n("Konstantin Vlasov"), i18n("Author"), QStringLiteral("konstantin.vlasov@rosalab.ru"));
    aboutData.addAuthor(i18n("Jonathan Riddell"), i18n("Author"), QStringLiteral("jr@jriddell.org"));
    QCommandLineParser m_Options;
    m_Options.addOption(QCommandLineOption("lang", "", "language"));
    m_Options.addOption(QCommandLineOption("dir", "", "path"));
    m_Options.addHelpOption();
    m_Options.addVersionOption();
    aboutData.setupCommandLine(&m_Options);
    m_Options.process(a);
    aboutData.processCommandLine(&m_Options);
    KAboutData::setApplicationData(aboutData);

    QString langName = a.getLocale();

    // Load main Qt translation for those languages that do not split into modules
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + langName, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    // For those languages that come splitted, load only the base Qt module translation
    QTranslator qtBaseTranslator;
    qtBaseTranslator.load("qtbase_" + langName, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtBaseTranslator);

    // Finally, load the translation of the application itself
    QTranslator appTranslator;
    appTranslator.load(langName, QCoreApplication::applicationDirPath() + "/lang");
    a.installTranslator(&appTranslator);

    if (!ensureElevated())
        return 1;

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
    w.show();

    UsbDeviceMonitor deviceMonitor;
    deviceMonitor.startMonitoring();

    // When device changing event comes, refresh the list of USB flash disks
    // Using QueuedConnection to avoid delays in processing the message
    QObject::connect(&deviceMonitor, &UsbDeviceMonitor::deviceChanged, &w, &MainDialog::scheduleEnumFlashDevices, Qt::QueuedConnection);

    return a.exec();
}