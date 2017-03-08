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

#include <QStandardPaths>
#include <QIcon>
#include <KAboutData>
#include <KLocalizedString>

#include "common.h"
#include "mainapplication.h"

// in one source file
Q_LOGGING_CATEGORY(IMAGEWRITER, "org.kde.imagewriter");

MainApplication::MainApplication(int& argc, char** argv) :
    QApplication(argc, argv)
{

    //FIXME why is this needed? because it's root?
    QIcon::setThemeName("breeze");
    setWindowIcon(QIcon::fromTheme("drive-removable-media"));
    KLocalizedString::setApplicationDomain("imagewriter");
    KAboutData aboutData( QStringLiteral("imagewriter"),
                          i18n("Image Writer"),
                          QStringLiteral("PROJECT_VERSION"),
                          i18n("Write an ISO Image to a USB Disk"),
                          KAboutLicense::GPL,
                          i18n("Copyright (c) 2016 ROSA"));

    aboutData.addAuthor(i18n("Konstantin Vlasov"), i18n("Author"), QStringLiteral("konstantin.vlasov@rosalab.ru"));
    aboutData.addAuthor(i18n("Jonathan Riddell"), i18n("Author"), QStringLiteral("jr@jriddell.org"));
    m_Options.addOption(QCommandLineOption("lang", "", "language"));
    m_Options.addOption(QCommandLineOption("dir", "", "path"));
    m_Options.addHelpOption();
    m_Options.addVersionOption();
    aboutData.setupCommandLine(&m_Options);
    m_Options.process(arguments());
    aboutData.processCommandLine(&m_Options);
    KAboutData::setApplicationData(aboutData);
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
QString MainApplication::getInitialImage()
{
    QStringList args = m_Options.positionalArguments();
    if (args.size() > 0)
        return args.at(0);
    else
        return "";
}
