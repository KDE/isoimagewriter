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

#ifndef SUPROGRAM_H
#define SUPROGRAM_H

#include <QString>
#include <QStringList>

// Auxiliary set of classes for handling different kinds of su-applications

// Base class
class SuProgram
{
protected:
    // Name of the su-application
    const QString m_binaryName;
    // Full path to the executable (if present)
    QString m_binaryPath;
    // Additional arguments
    const QStringList m_suArgs;
    // Whether it accepts target command line as separate arguments or single argument
    const bool m_splitArgs;

    // Constructor
    SuProgram(QString binaryName, QStringList suArgs, bool splitArgs);

public:
    // Destructor
    virtual ~SuProgram() {}
    // Check if the program is present in the system; by default searching in PATH is used
    virtual bool isPresent() const;
    // Check whether the su-application is native to the current desktop environment
    virtual bool isNative() const = 0;
    // Restarts the current application with the specified arguments using the GUI su program
    // Returns only if error occurred (execv() is used)
    virtual void restartAsRoot(const QStringList& args);
};


// Derivative classes for specific su-applications

class XdgSu : public SuProgram
{
public:
    XdgSu();
    virtual ~XdgSu() {}
    virtual bool isNative() const;
};

class KdeSu : public SuProgram
{
public:
    KdeSu();
    virtual ~KdeSu() {}
    virtual bool isNative() const;
};

class GkSu : public SuProgram
{
public:
    GkSu();
    virtual ~GkSu() {}
    virtual bool isNative() const;
};

class BeeSu : public SuProgram
{
public:
    BeeSu();
    virtual ~BeeSu() {}
    virtual bool isNative() const;
};


#endif // SUPROGRAM_H
