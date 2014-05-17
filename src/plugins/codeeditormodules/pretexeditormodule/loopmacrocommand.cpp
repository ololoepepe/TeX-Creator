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

#include "loopmacrocommand.h"
#include "global.h"
#include "macrocommand.h"
#include "macrocommandargument.h"
#include "executionstack.h"

#include <BeQtGlobal>

#include <QString>
#include <QList>

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool predLeqF(const double &t1, const double &t2)
{
    return t1 <= t2;
}

static bool predLeqI(const int &t1, const int &t2)
{
    return t1 <= t2;
}

static bool predGeqF(const double &t1, const double &t2)
{
    return t1 >= t2;
}

static bool predGeqI(const int &t1, const int &t2)
{
    return t1 >= t2;
}

/*============================================================================
================================ ForMacroCommand =============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ForMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 5) ? new ForMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ForMacroCommand::ForMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ForMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    typedef bool(*PredicateI)(const int &, const int &);
    typedef bool(*PredicateF)(const double &, const double &);
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString lb = margs.at(1).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString ub = margs.at(2).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString step = margs.at(3).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    int ilb = 0;
    bool nativelb;
    err = Global::toInt(lb, ilb, &nativelb);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    int iub = 0;
    bool nativeub;
    err = Global::toInt(ub, iub, &nativeub);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    int istep = 0;
    bool nativestep;
    err = Global::toInt(step, istep, &nativestep);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    if (nativelb && nativeub && nativestep)
    {
        PredicateI pred = (ilb < iub) ? &predLeqI : &predGeqI;
        //if (!stack->set(s, QString::number(ilb)))
        //    return bRet(error, QString("Failed to set variable"), QString("false"));
        for (int i = ilb; pred(i, iub); i += istep)
        {
            for (int j = 4; j < margs.size(); ++j)
            {
                margs.at(j).toText(doc, stack, &err);
                if (!err.isEmpty())
                    return bRet(error, err, QString("false"));
            }
            //if (!stack->set(s, QString::number(i + istep)))
            //    return bRet(error, QString("Failed to set variable"), QString("false"));
        }
    }
    else
    {
        double dlb = 0.0;
        Global::toDouble(lb, dlb);
        double dub = 0.0;
        Global::toDouble(ub, dub);
        double dstep = 0.0;
        Global::toDouble(step, dstep);
        PredicateF pred = (dlb < dub) ? &predLeqF : &predGeqF;
        //if (!stack->set(s, QString::number(dlb, 'g', 15)))
        //    return bRet(error, QString("Failed to set variable"), QString("false"));
        for (double d = dlb; pred(d, dub); d += dstep)
        {
            for (int j = 4; j < margs.size(); ++j)
            {
                margs.at(j).toText(doc, stack, &err);
                if (!err.isEmpty())
                    return bRet(error, err, QString("false"));
            }
            //if (!stack->set(s, QString::number(d + dstep, 'g', 15)))
            //    return bRet(error, QString("Failed to set variable"), QString("false"));
        }
    }
    return bRet(error, QString(), QString("true"));
}

QString ForMacroCommand::name() const
{
    return "for";
}

QString ForMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\for";
    for (int i = 0; i < 5; ++i)
        s += "{" + margs.at(i).toText() + "}";
    for (int i = 5; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ForMacroCommand::clone() const
{
    return new ForMacroCommand(margs);
}

/*============================================================================
================================ WhileMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *WhileMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new WhileMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

WhileMacroCommand::WhileMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString WhileMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString c = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    bool b = false;
    err = Global::toBool(c, b);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    while (b)
    {
        for (int i = 1; i < margs.size(); ++i)
        {
            margs.at(i).toText(doc, stack, &err);
            if (!err.isEmpty())
                return bRet(error, err, QString("false"));
        }
        c = margs.first().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
        err = Global::toBool(c, b);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
    }
    return bRet(error, QString(), QString("true"));
}

QString WhileMacroCommand::name() const
{
    return "while";
}

QString WhileMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\while";
    for (int i = 0; i < 2; ++i)
        s += "{" + margs.at(i).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *WhileMacroCommand::clone() const
{
    return new WhileMacroCommand(margs);
}

/*============================================================================
================================ DoWhileMacroCommand =========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *DoWhileMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new DoWhileMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

DoWhileMacroCommand::DoWhileMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString DoWhileMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString c;
    bool b = false;
    do
    {
        for (int i = 1; i < margs.size(); ++i)
        {
            margs.at(i).toText(doc, stack, &err);
            if (!err.isEmpty())
                return bRet(error, err, QString("false"));
        }
        c = margs.first().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
        err = Global::toBool(c, b);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
    }
    while (b);
    return bRet(error, QString(), QString("true"));
}

QString DoWhileMacroCommand::name() const
{
    return "doWhile";
}

QString DoWhileMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\doWhile";
    for (int i = 0; i < 2; ++i)
        s += "{" + margs.at(i).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *DoWhileMacroCommand::clone() const
{
    return new DoWhileMacroCommand(margs);
}

/*============================================================================
================================ UntilMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *UntilMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new UntilMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

UntilMacroCommand::UntilMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString UntilMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString c = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    bool b = false;
    err = Global::toBool(c, b);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    while (!b)
    {
        for (int i = 1; i < margs.size(); ++i)
        {
            margs.at(i).toText(doc, stack, &err);
            if (!err.isEmpty())
                return bRet(error, err, QString("false"));
        }
        c = margs.first().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
        err = Global::toBool(c, b);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
    }
    return bRet(error, QString(), QString("true"));
}

QString UntilMacroCommand::name() const
{
    return "until";
}

QString UntilMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\until";
    for (int i = 0; i < 2; ++i)
        s += "{" + margs.at(i).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *UntilMacroCommand::clone() const
{
    return new UntilMacroCommand(margs);
}

/*============================================================================
================================ DoUntilMacroCommand =========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *DoUntilMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new DoUntilMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

DoUntilMacroCommand::DoUntilMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString DoUntilMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString c;
    bool b = false;
    do
    {
        for (int i = 1; i < margs.size(); ++i)
        {
            margs.at(i).toText(doc, stack, &err);
            if (!err.isEmpty())
                return bRet(error, err, QString("false"));
        }
        c = margs.first().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
        err = Global::toBool(c, b);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
    }
    while (!b);
    return bRet(error, QString(), QString("true"));
}

QString DoUntilMacroCommand::name() const
{
    return "doUntil";
}

QString DoUntilMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\doUntil";
    for (int i = 0; i < 2; ++i)
        s += "{" + margs.at(i).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *DoUntilMacroCommand::clone() const
{
    return new DoUntilMacroCommand(margs);
}
