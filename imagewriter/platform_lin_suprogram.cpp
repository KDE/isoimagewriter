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

#include <QProcess>
#include <QStandardPaths>
#include <QFile>

#include "common.h"
#include "platform_lin_suprogram.h"

// Copied from private Qt implementation at qtbase/src/platformsupport/services/genericunix/qgenericunixservices.cpp
static QString detectDesktopEnvironment()
{
    const QString xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");
    if (!xdgCurrentDesktop.isEmpty())
        return xdgCurrentDesktop.toUpper(); // KDE, GNOME, UNITY, LXDE, MATE, XFCE...

    // Classic fallbacks
    if (!qEnvironmentVariableIsEmpty("KDE_FULL_SESSION"))
        return "KDE";
    if (!qEnvironmentVariableIsEmpty("GNOME_DESKTOP_SESSION_ID"))
        return "GNOME";

    // Fallback to checking $DESKTOP_SESSION (unreliable)
    const QString desktopSession = qgetenv("DESKTOP_SESSION");
    if (desktopSession == "gnome")
        return "GNOME";
    if (desktopSession == "xfce")
        return "XFCE";

    return "";
}

SuProgram::SuProgram(QString binaryName, QStringList suArgs, bool splitArgs)
    : m_binaryName(binaryName)
    , m_suArgs(suArgs)
    , m_splitArgs(splitArgs)
{
    m_binaryPath = QStandardPaths::findExecutable(m_binaryName);
}

XdgSu::XdgSu()
    : SuProgram("xdg-su", {"-c"}, false)
{
}

KdeSu::KdeSu()
    : SuProgram("kdesu", {"-c"}, false)
{
    // Extract from man:
    //   Since <kdesu> is no longer installed in <$(kde4-config --prefix)/bin> but in <kde4-config --path libexec>
    //   and therefore not in your <Path>, you have to use <$(kde4-config --path libexec)kdesu> to launch <kdesu>.
    // Now, what we do: first, simple PATH check is performed in SuProgram constructor. Then we try to find kdesu
    // according to the KDE documentation and, if found, overwrite the path with the new one.
    QProcess kdeConfig;
    kdeConfig.start("kde4-config", {"--path", "libexec"}, QIODevice::ReadOnly);
    kdeConfig.waitForFinished(3000);
    QString kdesuPath = kdeConfig.readAllStandardOutput().trimmed();
    if (!kdesuPath.isEmpty())
    {
        kdesuPath += (kdesuPath.endsWith('/') ? "" : "/");
        kdesuPath += "kdesu";
        if (QFile::exists(kdesuPath) && QFile::permissions(kdesuPath).testFlag(QFileDevice::ExeUser))
            m_binaryPath = kdesuPath;
    }
}

GkSu::GkSu()
    : SuProgram("gksu", {}, false)
{
}

BeeSu::BeeSu()
    : SuProgram("beesu", {}, true)
{
}


bool SuProgram::isPresent() const
{
    return (!m_binaryPath.isEmpty());
}


bool XdgSu::isNative() const
{
    // xdg-su is considered a universal tool
    return true;
}

bool KdeSu::isNative() const
{
    // kdesu is native only in KDE
    const QString desktopEnvironment = detectDesktopEnvironment();
    return (desktopEnvironment == "KDE");
}

bool GkSu::isNative() const
{
    // gksu is native for GTK-based DEs; however, for Qt-based DEs (like LXQt) there is no alternative either
    const QString desktopEnvironment = detectDesktopEnvironment();
    return (!desktopEnvironment.isEmpty() && (desktopEnvironment != "KDE"));
}

bool BeeSu::isNative() const
{
    // beesu is developed for Fedora/RedHat, let's consider it native there
    QFile redhatRelease("/etc/redhat-release");
    if (!redhatRelease.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QString redhatInfo = redhatRelease.readAll();
    return redhatInfo.contains("Red Hat", Qt::CaseInsensitive) || redhatInfo.contains("Fedora", Qt::CaseInsensitive);
}


void SuProgram::restartAsRoot(const QStringList& args)
{
    if (m_binaryPath.isEmpty())
        return;

    int i;

    // For execv() we need the list of char* arguments; using QByteArrays as temporary storage
    // Store QByteArray objects explicitly to make sure they live long enough, so that their data()'s were valid until execv() call
    QList<QByteArray> argsBA;

    // First comes the application being started (su-application) with all its arguments (if any)
    argsBA << m_binaryPath.toUtf8();
    for (i = 0; i < m_suArgs.size(); ++i)
        argsBA << m_suArgs[i].toUtf8();
    // Now append the passed arguments
    // Depending on the su-application, we may need to either pas them separately, or space-join them into one single argument word
    if (m_splitArgs)
    {
        for (i = 0; i < args.size(); ++i)
            argsBA << args[i].toUtf8();
    }
    else
    {
        QString joined = '\'' + args[0] + '\'';
        for (i = 1; i < args.size(); ++i)
            joined += " '" + args[i] + "'";
        argsBA << joined.toUtf8();
    }

    // Convert arguments into char*'s and append NULL element
    int argsNum = argsBA.size();
    char** argsBin = new char*[argsNum + 1];
    for (i = 0; i < argsNum; ++i)
        argsBin[i] = argsBA[i].data();
    argsBin[argsNum] = NULL;

    // Replace ourselves with su-application
    execv(argsBin[0], argsBin);

    // Something went wrong, we should have never returned! Cleaning up
    delete[] argsBin;
}
