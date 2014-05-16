/****************************************************************************
**
** Copyright (C) 2014 TeXSample Team
**
** This file is part of the PreTeX Editor Module plugin of TeX Creator.
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

#include "execmacrocommands.h"
#include "macrocommand.h"
#include "macrocommandargument.h"
#include "executionstack.h"
#include "pretexeditormoduleplugin.h"
#include "global.h"

#include <BeQtGlobal>
#include <BAbstractCodeEditorDocument>
#include <BeQt>
#include <BTextTools>
#include <BApplication>

#include <QList>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QFileInfo>
#include <QMap>
#include <QTextCodec>
#include <QTextStream>
#include <QByteArray>

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static QString getWorkingDir(BAbstractCodeEditorDocument *doc, QString &dir)
{
    QFileInfo fi(doc->fileName());
    if (!fi.exists() || !fi.isFile())
        return "Document does not exist on disk";
    dir = fi.path();
    return "";
}

static QString getCommand(const MacroCommandArgument &arg, BAbstractCodeEditorDocument *doc, ExecutionStack *stack,
                          QString &command)
{
    QString err;
    QString cmd = arg.toText(doc, stack, &err);
    if (!err.isEmpty())
        return err;
    QString cmd2 = PretexEditorModulePlugin::externalTools().value(cmd);
    if (!cmd2.isEmpty())
        cmd = cmd2;
    if (cmd.isEmpty())
        return "Invalid parameter";
    command = cmd;
    return "";
}

static QString getFileName(const MacroCommandArgument &arg, BAbstractCodeEditorDocument *doc, ExecutionStack *stack,
                           QString &fileName, QString *workingDir = 0)
{
    QString path;
    QString err = getWorkingDir(doc, path);
    if (!err.isEmpty())
        return err;
    QString fn = path + "/" + arg.toText(doc, stack, &err);
    if (!err.isEmpty())
        return err;
    if (fn.isEmpty())
        return "Invalid parameter";
    QFileInfo fi(fn);
    if (!fi.exists() || !fi.isFile())
        return "File does not exist";
    fileName = fn;
    bSet(workingDir, path);
    return "";
}

static QString getCommandArgs(const QList<MacroCommandArgument> &args, int from, BAbstractCodeEditorDocument *doc,
                              ExecutionStack *stack, QStringList &arguments)
{
    QStringList sl;
    for (int i = from; i < args.size(); ++i)
    {
        QString err;
        QString s = args.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return err;
        sl << s;
    }
    arguments = sl;
    return "";
}

static QString processOutputToText(const QByteArray &output)
{
    QTextCodec *codec = BTextTools::guessTextCodec(output, BApplication::locale());
    if (!codec)
        return Global::toVisibleText(QString(output));
    QTextStream in(output);
    in.setCodec(codec);
    return Global::toVisibleText(in.readAll());
}

/*============================================================================
================================ ExecMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1) ? new ExecMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecMacroCommand::ExecMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QStringList args;
    err = getCommandArgs(margs, 1, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString dir;
    getWorkingDir(doc, dir);
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    if (!dir.isEmpty())
        proc.setWorkingDirectory(dir);
    BeQt::startProcess(&proc, cmd, args);
    BeQt::waitNonBlocking(&proc, SIGNAL(started()), 30 * BeQt::Second);
    if (proc.state() != QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process start timeout"), QString());
    }
    BeQt::waitNonBlocking(&proc, SIGNAL(finished(int)), 5 * BeQt::Minute);
    if (proc.state() == QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process execution timeout"), QString());
    }
    return bRet(error, QString(), processOutputToText(proc.readAll()));
}

QString ExecMacroCommand::name() const
{
    return "exec";
}

QString ExecMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\exec{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecMacroCommand::clone() const
{
    return new ExecMacroCommand(margs);
}

/*============================================================================
================================ ExecFMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecFMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new ExecFMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecFMacroCommand::ExecFMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecFMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString fn;
    QString dir;
    err = getFileName(margs.at(1), doc, stack, fn, &dir);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QStringList args;
    err = getCommandArgs(margs, 2, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    args.prepend(fn);
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.setWorkingDirectory(dir);
    BeQt::startProcess(&proc, cmd, args);
    BeQt::waitNonBlocking(&proc, SIGNAL(started()), 30 * BeQt::Second);
    if (proc.state() != QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process start timeout"), QString());
    }
    BeQt::waitNonBlocking(&proc, SIGNAL(finished(int)), 5 * BeQt::Minute);
    if (proc.state() == QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process execution timeout"), QString());
    }
    return bRet(error, QString(), processOutputToText(proc.readAll()));
}

QString ExecFMacroCommand::name() const
{
    return "execF";
}

QString ExecFMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\execF{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecFMacroCommand::clone() const
{
    return new ExecFMacroCommand(margs);
}

/*============================================================================
================================ ExecDMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecDMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1) ? new ExecDMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecDMacroCommand::ExecDMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecDMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QStringList args;
    err = getCommandArgs(margs, 1, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString dir;
    getWorkingDir(doc, dir);
    bool b = !dir.isEmpty() ? QProcess::startDetached(cmd, args, dir) : QProcess::startDetached(cmd, args);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString ExecDMacroCommand::name() const
{
    return "execD";
}

QString ExecDMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\execD{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecDMacroCommand::clone() const
{
    return new ExecDMacroCommand(margs);
}

/*============================================================================
================================ ExecFDMacroCommand ==========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecFDMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new ExecFDMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecFDMacroCommand::ExecFDMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecFDMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString fn;
    QString dir;
    err = getFileName(margs.at(1), doc, stack, fn, &dir);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QStringList args;
    err = getCommandArgs(margs, 2, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    args.prepend(fn);
    bool b = !dir.isEmpty() ? QProcess::startDetached(cmd, args, dir) : QProcess::startDetached(cmd, args);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString ExecFDMacroCommand::name() const
{
    return "execFD";
}

QString ExecFDMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\execFD{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecFDMacroCommand::clone() const
{
    return new ExecFDMacroCommand(margs);
}
