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

#include "remoteterminaldriver.h"

#include "client.h"
#include "application.h"
#include "global.h"

#include <BAbstractTerminalDriver>

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QTextCodec>

#include <QDebug>

/*============================================================================
================================ RemoteTerminalDriver ========================
============================================================================*/

/*============================== Public constructors =======================*/

RemoteTerminalDriver::RemoteTerminalDriver(QObject *parent) :
    BAbstractTerminalDriver(parent)
{
    mactive = false;
}

/*============================== Public methods ============================*/

bool RemoteTerminalDriver::processCommand(const QString &, const QStringList &, QString &)
{
    return false;
}

bool RemoteTerminalDriver::isActive() const
{
    return mactive;
}

QString RemoteTerminalDriver::read(QTextCodec *codec)
{
    QString s = mbuffer;
    mbuffer.clear();
    return s;
}

void RemoteTerminalDriver::close()
{
    //
}

bool RemoteTerminalDriver::terminalCommand(const QVariant &data, QString &error)
{
    mactive = true;
    emitBlockTerminal();
    /*TCompilerParameters param = Global::compilerParameters();
    QVariantMap m = data.toMap();
    QString fn = m.value("file_name").toString();
    QTextCodec *codec = QTextCodec::codecForName(m.value("codec_name").toString().toLatin1());
    TCompilationResult mr;
    TCompilationResult dr;
    TCompilationResult r = sClient->compile(fn, codec, param, mr, dr);
    mbuffer = r.log();
    if (!mr.log().isEmpty())
        mbuffer += "\n\n" + mr.log();
    if (!dr.log().isEmpty())
        mbuffer += "\n\n" + dr.log();
    error = r.messageString();*/
    emitReadyRead();
    emitUnblockTerminal();
    mactive = false;
    //emitFinished(r.exitCode());
    //return r.success();
}