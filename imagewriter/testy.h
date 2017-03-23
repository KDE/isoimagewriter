/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  KDE neon <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TESTY_H
#define TESTY_H

#include <KAuthAction>

#include <QObject>
#include <QPushButton>
#include <KJob>

/**
 * @todo write docs
 */
class Testy : public QObject
{
    Q_OBJECT
public:
    Testy(int argc, char *argv[]);
public slots:
    void run();
    void runAsync();
    void statusChanged(KAuth::Action::AuthStatus status);
    void progressStep(KJob*, unsigned long step);
    void progressStep(const QVariantMap &);
    void finished(KJob* job);
private:
    QPushButton *m_button;
};

#endif // TESTY_H
