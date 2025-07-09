        /*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QIcon>
#include <QLoggingCategory>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQml>
#include <QUrl>


#include <KAboutData>
#include <KLocalizedString>
#include <KCrash>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "common.h"
#include "mainapplication.h"
#include "mainwindow.h"
#include "usbdevicemonitor.h"

#if !defined(Q_OS_WIN32) && !defined(Q_OS_LINUX) && !defined(Q_OS_MAC) && !defined(Q_OS_FREEBSD)
#error Unsupported platform!
#endif


int main(int argc, char *argv[])
{
#if defined(Q_OS_MAC)
    // On Mac OS X elevated launch is treated as setuid which is forbidden by default -> enable it
    // TODO: Try to find a more "kosher" way, as well as get rid of deprecated AuthorizationExecuteWithPrivileges()
    QCoreApplication::setSetuidAllowed(true);
#endif

    // MainApplication a(argc, argv);
    QGuiApplication app(argc, argv);
    KCrash::initialize();
    // KCrash::initialize();

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

    // MainWindow w;
    // w.show();

    // UsbDeviceMonitor deviceMonitor;
    // deviceMonitor.startMonitoring();

    // // When device changing event comes, refresh the list of USB flash disks
    // // Using QueuedConnection to avoid delays in processing the message
    // QObject::connect(&deviceMonitor, &UsbDeviceMonitor::deviceChanged, &w,
    //                  &MainWindow::scheduleEnumFlashDevices, Qt::QueuedConnection);

    // return a.exec();

    KLocalizedString::setApplicationDomain("helloworld");
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationName(QStringLiteral("Hello World"));

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }


    KAboutData aboutData(
        QStringLiteral("IsoImage Writer"),
        i18nc("@title", "IsoImage Writer"),
        QStringLiteral("1.0"),
        i18n("Write an ISO Image to a USB Disk"),
        KAboutLicense::GPL,
        i18n("(c) 2021"));


    aboutData.addAuthor(
        i18nc("@info:credit", "@Holychicken"),
        i18nc("@info:credit", "Author Role"),

        //TODO : update the school address
        QStringLiteral("assa297@sfu.ca"),
        QStringLiteral("https://tcombinator.dev"));

    KAboutData::setApplicationData(aboutData);

    qmlRegisterSingletonType(
        "org.kde.isoimagewriter.about", // How the import statement should look like
        1, 0, // Major and minor versions of the import
        "About", // The name of the QML object
        [](QQmlEngine* engine, QJSEngine *) -> QJSValue {
            return engine->toScriptValue(KAboutData::applicationData());
        }
    );

    QQmlApplicationEngine engine;

    engine.addImportPath("qrc:/");
    engine.addImportPath("qrc:/qml");

    engine.rootContext()->setContextProperty("mainApp", &app);
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qWarning() << "Failed to create QML object for" << objUrl;
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    engine.load(url);

    // if (engine.rootObjects().isEmpty()) {
    //     qWarning() << "Failed to load QML file:" << url;
    //     // const auto errors = engine.errors();
    //     // for (const auto &error : errors) {
    //     //     qWarning() << "QML Error:" << error.toString();
    //     // }
    //     return -1;
    // }
    return app.exec();  // Start the Qt event loop
}
