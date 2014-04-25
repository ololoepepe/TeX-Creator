/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
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

#include "applicationserver.h"
#include "application.h"

#include <BApplicationServer>

#include <QObject>
#include <QStringList>

#include <QDebug>

/*============================================================================
================================ ApplicationServer ===========================
============================================================================*/

/*============================== Public constructors =======================*/

ApplicationServer::ApplicationServer(quint16 port, const QString &serverName, int operationTimeout) :
    BApplicationServer(port, serverName, operationTimeout)
{
    //
}

/*============================== Protected methods =========================*/

void ApplicationServer::handleMessage(const QStringList &arguments)
{
    bApp->handleExternalRequest(arguments);
}
