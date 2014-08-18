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

#include "remoteterminaldriver.h"

#include "client.h"
#include "mainwindow.h"
#include "settings.h"
#include "texsample/texsamplecore.h"

#include <TBinaryFile>
#include <TBinaryFileList>
#include <TCompileTexProjectReplyData>
#include <TCompileTexProjectRequestData>
#include <TOperation>
#include <TReply>
#include <TTexCompiler>
#include <TTexProject>

#include <BAbstractTerminalDriver>
#include <BeQt>

#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QVariant>
#include <QVariantMap>

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

void RemoteTerminalDriver::close()
{
    //
}

bool RemoteTerminalDriver::isActive() const
{
    return mactive;
}

bool RemoteTerminalDriver::processCommand(const QString &, const QStringList &, QString &, QTextCodec *)
{
    return false;
}

QString RemoteTerminalDriver::read(QTextCodec *)
{
    QString s = mbuffer;
    mbuffer.clear();
    return s;
}

bool RemoteTerminalDriver::terminalCommand(const QVariant &data, QString &error, QTextCodec *)
{
    mactive = true;
    emitBlockTerminal();
    QVariantMap m = data.toMap();
    QTextCodec *codec = BeQt::codec(m.value("codec_name").toString());
    QString fileName = m.value("file_name").toString();
    TTexProject project;
    if (!project.load(fileName, codec)) {
        error = tr("Failed to load project", "error");
        emitUnblockTerminal();
        mactive = false;
        emitFinished(-1);
        return false;
    }
    TCompileTexProjectRequestData request;
    request.setCodec(codec);
    request.setCommands(Settings::Compiler::compilerCommands());
    request.setCompiler(Settings::Compiler::compiler());
    request.setDvipsEnabled(Settings::Compiler::dvipsEnabled());
    request.setMakeindexEnabled(Settings::Compiler::makeindexEnabled());
    request.setOptions(Settings::Compiler::compilerOptions());
    request.setProject(project);
    TReply r = tSmp->client()->performOperation(TOperation::CompileTexProject, request, bApp->mostSuitableWindow());
    if (!r.success()) {
        error = r.message();
        emitUnblockTerminal();
        mactive = false;
        emitFinished(-1);
        return false;
    }
    TCompileTexProjectReplyData replyData = r.data().value<TCompileTexProjectReplyData>();
    mbuffer = replyData.output();
    QString path = QFileInfo(fileName).path();
    foreach (const TBinaryFile &file, replyData.files()) {
        if (!file.save(path)) {
            error = tr("Failed to save file:", "error") + " " + file.fileName();
            emitUnblockTerminal();
            mactive = false;
            emitFinished(replyData.exitCode());
            return false;
        }
    }
    emitReadyRead();
    emitUnblockTerminal();
    mactive = false;
    emitFinished(replyData.exitCode());
    return true;
}
