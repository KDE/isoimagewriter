/*
    SPDX-FileCopyrightText: 2017 Jonathan Riddell <jr@jriddell.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef IMAGEWRITER_HELPER_H
#define IMAGEWRITER_HELPER_H

#include <kauth.h>

using namespace KAuth;

class ImageWriterHelper : public QObject
{
    Q_OBJECT

public:
    ImageWriterHelper();

public Q_SLOTS:
    ActionReply write(const QVariantMap &args);
};

#endif // IMAGEWRITER_HELPER_H
