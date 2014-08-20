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

#ifndef APPLICATIONSERVER_H
#define APPLICATIONSERVER_H

class QStringList;

#include <BApplicationServer>
#include <BeQt>

#include <QObject>

/*============================================================================
================================ ApplicationServer ===========================
============================================================================*/

class ApplicationServer : public BApplicationServer
{
public:
    explicit ApplicationServer(quint16 port, const QString &serverName, int operationTimeout = 5 * BeQt::Second);
protected:
    void handleMessage(const QStringList &arguments);
private:
    Q_DISABLE_COPY(ApplicationServer)
};

#endif // APPLICATIONSERVER_H
