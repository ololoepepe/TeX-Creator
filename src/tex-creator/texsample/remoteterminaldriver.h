/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
**
** This file is part of TeX Creator.
**
** TeX Creator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** TeX Creator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with TeX Creator.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef REMOTETERMINALDRIVER_H
#define REMOTETERMINALDRIVER_H

class QStringList;
class QTextCodec;
class QVariant;

#include <BAbstractTerminalDriver>

#include <QString>

/*============================================================================
================================ RemoteTerminalDriver ========================
============================================================================*/

class RemoteTerminalDriver : public BAbstractTerminalDriver
{
    Q_OBJECT
private:
    bool mactive;
    QString mbuffer;
public:
    explicit RemoteTerminalDriver(QObject *parent = 0);
public:
    void close();
    bool isActive() const;
    bool processCommand(const QString &command, const QStringList &arguments, QString &error, QTextCodec *codec);
    QString read(QTextCodec *codec);
    bool terminalCommand(const QVariant &data, QString &error, QTextCodec *codec);
};

#endif // REMOTETERMINALDRIVER_H
