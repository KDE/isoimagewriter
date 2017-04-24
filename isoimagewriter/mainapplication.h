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
