/*
    SPDX-FileCopyrightText: 2016 ROSA, 2022 Jonathan Esk-Riddell <jr@jriddell.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "mainapplication.h"

#include <KAboutData>
#include <KLocalizedString>
#include <QIcon>
#include <QStandardPaths>
#include <QUrl>

#include "common.h"
#include "isoimagewriter_version.h"

// in one source file
Q_LOGGING_CATEGORY(IMAGEWRITER, "org.kde.isoimagewriter");

MainApplication::MainApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setWindowIcon(QIcon::fromTheme("org.kde.isoimagewriter"));
    KLocalizedString::setApplicationDomain("isoimagewriter");
    KAboutData aboutData(QStringLiteral("isoimagewriter"),
                         i18n("ISO Image Writer"),
                         QStringLiteral(ISOIMAGEWRITER_VERSION_STRING),
                         i18n("Write an ISO Image to a USB Disk"),
                         KAboutLicense::GPL,
                         i18n("Copyright (c) 2016 ROSA, Copyright (c) 2022 Jonathan Esk-Riddell <jr@jriddell.org>"));

    aboutData.addAuthor(i18n("Konstantin Vlasov"), i18n("Author"), QStringLiteral("konstantin.vlasov@rosalab.ru"));
    aboutData.addAuthor(i18n("Jonathan Riddell"), i18n("Author"), QStringLiteral("jr@jriddell.org"));
    m_Options.addVersionOption();
    m_Options.addOption(QCommandLineOption("lang", "", "language"));
    m_Options.addOption(QCommandLineOption("dir", "", "path"));
    m_Options.addPositionalArgument("[File]", i18n("ISO file to open"));
    aboutData.setupCommandLine(&m_Options);
    m_Options.process(arguments());
    aboutData.processCommandLine(&m_Options);
    KAboutData::setApplicationData(aboutData);

    QLoggingCategory::setFilterRules(QStringLiteral("org.kde.isoimagewriter = true"));
}

// Returns the language id to be used by the application (specified by --lang, or system locale otherwise)
QString MainApplication::getLocale()
{
    return (m_Options.isSet("lang") ? m_Options.value("lang") : QLocale::system().name());
}

// Returns the start-up directory that will be shown by default in the Open File dialog
QString MainApplication::getInitialDir()
{
    // TODO: Check for elevation
    // win:restricted
    // win:admin
    // mac:restricted
    // linux:restricted
    // linux:root
    // linux: translated dir names
    // win: redefined paths
    if (m_Options.isSet("dir"))
        return m_Options.value("dir");

    // Otherwise get the standard system Downloads location
    QStringList downloadDirs = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
    if (downloadDirs.size() > 0)
        return downloadDirs.at(0);
    else
        return "";
}

// Returns the fila path passed to the application as command-line parameter
QUrl MainApplication::getInitialImage()
{
    const QStringList args = m_Options.positionalArguments();
    if (!args.isEmpty())
        return QUrl::fromUserInput(args.first(), QString(), QUrl::AssumeLocalFile);
    else
        return {};
}
