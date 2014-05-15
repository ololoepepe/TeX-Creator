/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
**
** This file is part of the MacrosEditorModule plugin of TeX Creator.
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

#include "generalmacrocommands.h"
#include "macrocommand.h"
#include "macrocommandargument.h"
#include "macroexecutionstack.h"
#include "global.h"

#include <BeQtGlobal>
#include <BAbstractCodeEditorDocument>
#include <BeQt>

#include <QList>
#include <QMap>
#include <QString>
#include <QThread>

/*============================================================================
================================ ThreadHack ==================================
============================================================================*/

class ThreadHack : public QThread
{
public:
    static void waitBlocking(int msecs);
};

/*============================================================================
================================ ThreadHack ==================================
============================================================================*/

/*============================== Static public methods =====================*/

void ThreadHack::waitBlocking(int msecs)
{
    currentThread()->msleep(msecs);
}

/*============================================================================
================================ FormatMacroCommand ==========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *FormatMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 2) ? new FormatMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

FormatMacroCommand::FormatMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString FormatMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString f = margs.last().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    err = Global::formatText(s, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), s);
}

QString FormatMacroCommand::name() const
{
    return "format";
}

QString FormatMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\fromat{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    return s;
}

AbstractMacroCommand *FormatMacroCommand::clone() const
{
    return new FormatMacroCommand(margs);
}

/*============================================================================
================================ MultiMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *MultiMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1) ? new MultiMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

MultiMacroCommand::MultiMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString MultiMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    foreach (const MacroCommandArgument &a, margs)
    {
        QString err;
        a.toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    return bRet(error, QString(), QString());
}

QString MultiMacroCommand::name() const
{
    return "multi";
}

QString MultiMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\multi{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *MultiMacroCommand::clone() const
{
    return new MultiMacroCommand(margs);
}

/*============================================================================
================================ IfMacroCommand ==============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *IfMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new IfMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

IfMacroCommand::IfMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString IfMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString c = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    bool b = false;
    if ("true" == c.toLower())
    {
        b = true;
    }
    else if ("false" != c.toLower())
    {
        bool ok = false;
        b = c.contains('.') ? (bool) c.toDouble(&ok) : (bool) c.toInt(&ok);
        if (!ok)
            return bRet(error, QString("Failed to convert"), QString());
    }
    if (!b && margs.size() < 3)
        return bRet(error, QString(), QString());
    QString s = margs.at(1 + (b ? 0 : 1)).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), s);
}

QString IfMacroCommand::name() const
{
    return "if";
}

QString IfMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\if{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *IfMacroCommand::clone() const
{
    return new IfMacroCommand(margs);
}

/*============================================================================
================================ WaitMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *WaitMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 3) ? new WaitMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

WaitMacroCommand::WaitMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString WaitMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString ns = margs.first().toText(doc, stack, &err);
    if (ns.isEmpty() || !err.isEmpty())
        return bRet(error, !err.isEmpty() ? err : QString("Invalid parameter"), QString("false"));
    int n = 0;
    err = Global::toInt(ns, n);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    int k = BeQt::Second;
    bool b = false;
    bool kb = false;
    bool bb = false;
    for (int i = 1; i < margs.size(); ++i)
    {
        QString as = margs.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
        if (!as.compare("ms", Qt::CaseInsensitive) || !as.compare("milliseconds", Qt::CaseInsensitive))
        {
            if (kb)
                return bRet(error, QString("Duplicate parameter"), QString("false"));
            k = 1;
            kb = true;
        }
        else if (!as.compare("s", Qt::CaseInsensitive) || !as.compare("seconds", Qt::CaseInsensitive))
        {
            if (kb)
                return "false";
            k = BeQt::Second;
            kb = true;
        }
        else if (!as.compare("m", Qt::CaseInsensitive) || !as.compare("minutes", Qt::CaseInsensitive))
        {
            if (kb)
                return "false";
            k = BeQt::Minute;
            kb = true;
        }
        else if (!as.compare("b", Qt::CaseInsensitive) || !as.compare("blocking", Qt::CaseInsensitive))
        {
            if (bb)
                return "false";
            b = true;
            bb = true;
        }
        else if (!as.compare("nb", Qt::CaseInsensitive) || !as.compare("non-blocking", Qt::CaseInsensitive))
        {
            if (bb)
                return "false";
            b = false;
            bb = true;
        }
        else
        {
            return bRet(error, QString("Unknown parameter"), QString("false"));
        }
    }
    if (b)
        ThreadHack::waitBlocking(k * n);
    else
        BeQt::waitNonBlocking(k * n);
    return bRet(error, QString(), QString("true"));
}

QString WaitMacroCommand::name() const
{
    return "wait";
}

QString WaitMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\wait{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *WaitMacroCommand::clone() const
{
    return new WaitMacroCommand(margs);
}
