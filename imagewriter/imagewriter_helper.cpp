/*
 * Copyright 2017 Jonathan Riddell <jr@jriddell.org>
 * 
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

#include "imagewriter_helper.h"
#include "imagewriter_debug.h"

#include <KLocalizedString>
#include <KAuthActionReply>

#include <QProcess>
#include <QDateTime>
#include <QDebug>
#include <QFile>

#include <stdio.h>
#include <iostream>
#include <time.h>

ImageWriterHelper::ImageWriterHelper()
{
    KLocalizedString::setApplicationDomain("imagewriter");
}

ActionReply ImageWriterHelper::writefile(const QVariantMap &args)
{
    ActionReply reply;
    QString filename = args["filename"].toString();
    QFile file("/root/" + filename);

    if (!file.open(QIODevice::ReadWrite)) {
       reply = ActionReply::HelperErrorReply();
       reply.setError(file.error());

       return reply;
    }

    QTextStream stream(&file);
    stream << "something" << endl;

    QVariantMap retdata;
    retdata["contents"] = "something";

    reply.setData(retdata);
    qDebug() << "I'm in the helper";
    for (int i = 0; i < 5; i++) {
        qDebug() << "helper: " << i;
        QFile file("/tmp/foo");

        if (!file.open(QIODevice::ReadWrite)) {
            reply = ActionReply::HelperErrorReply();
            reply.setError(file.error());
            return reply;
        }
        QTextStream stream(&file);
        stream << i << endl;
        file.close();
        struct timespec ts = { 1000 / 1000, (1000 % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
    }
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.imagewriter", ImageWriterHelper)
