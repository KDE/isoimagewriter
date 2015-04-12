#include <QStandardPaths>

#include "mainapplication.h"

MainApplication::MainApplication(int& argc, char** argv) :
    QApplication(argc, argv)
{
    m_Options.addOption(QCommandLineOption("lang", "", "language"));
    m_Options.addOption(QCommandLineOption("dir", "", "path"));
    // Command line interface is internal, so using parse() instead of process() to ignore unknown options
    m_Options.parse(arguments());
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
