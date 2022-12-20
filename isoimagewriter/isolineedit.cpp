/* 
 * SPDX-FileCopyrightText: 2022 Enes Albay <albayenes@gmail.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "isolineedit.h"

IsoLineEdit::IsoLineEdit(QWidget* parent)
: QLineEdit(parent)
{
    installEventFilter(this);
}
