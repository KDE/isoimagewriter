/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QApplication>
#include <QCommandLineParser>

class MainApplication : public QApplication
{
public:
    MainApplication(int& argc, char** argv);
    // Returns the language id to be used by the application (specified by --lang, or system locale otherwise)
    QString getLocale();
    // Returns the start-up directory that will be shown by default in the Open File dialog
    QString getInitialDir();
    // Returns the fila path passed to the application as command-line parameter
    QString getInitialImage();

protected:
    QCommandLineParser m_Options;
};

#endif // MAINAPPLICATION_H
