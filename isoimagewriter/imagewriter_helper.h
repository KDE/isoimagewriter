/*
    SPDX-FileCopyrightText: 2017 Jonathan Riddell <jr@jriddell.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <QtGlobal>

#include <kauth_version.h>
#if KAUTH_VERSION >= QT_VERSION_CHECK(5, 92, 0)
#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>
#else
#include <kauth.h>
#endif

using namespace KAuth;

class ImageWriterHelper : public QObject
{
    Q_OBJECT

public:
    ImageWriterHelper();

public Q_SLOTS:
    ActionReply write(const QVariantMap &args);
};
