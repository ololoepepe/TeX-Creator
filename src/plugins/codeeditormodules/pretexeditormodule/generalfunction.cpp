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

#include "generalfunction.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "executionstack.h"
#include "global.h"
#include "executionmodule.h"
#include "tokendata.h"
#include "token.h"

#include <BeQtGlobal>
#include <BeQt>

#include <QList>
#include <QString>
#include <QRegExp>

#include <QDebug>

B_DECLARE_TRANSLATE_FUNCTION

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool predLeqD(const double &t1, const double &t2)
{
    return t1 <= t2;
}

static bool predLeqI(const int &t1, const int &t2)
{
    return t1 <= t2;
}

static bool predGeqD(const double &t1, const double &t2)
{
    return t1 >= t2;
}

static bool predGeqI(const int &t1, const int &t2)
{
    return t1 >= t2;
}

static bool isEmpty(ExecutionStack *stack, QString *err = 0)
{
    stack->setReturnValue(stack->obligArg().isNull() ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool toInteger(ExecutionStack *stack, QString *err = 0)
{
    stack->setReturnValue(stack->obligArg().toInt());
    return bRet(err, QString(), true);
}

static bool toReal(ExecutionStack *stack, QString *err = 0)
{
    stack->setReturnValue(stack->obligArg().toReal());
    return bRet(err, QString(), true);
}

static bool toString(ExecutionStack *stack, QString *err = 0)
{
    stack->setReturnValue(stack->obligArg().toString());
    return bRet(err, QString(), true);
}

static bool format(ExecutionStack *stack, QString *err = 0)
{
    if (stack->obligArg(1).type() != PretexVariant::String)
        return bRet(err, translate("format", "Expected a string", "error"), false);
    if (stack->obligArg(0).type() == PretexVariant::String)
    {
        stack->setReturnValue(stack->obligArg(0));
        return bRet(err, QString(), true);
    }
    QString text = stack->obligArg(0).toString();
    QString format = stack->obligArg(1).toString();
    if (QRegExp("(([1-9][0-9]*)\\.)?i(\\.([1-9]|1[0-5]))?").exactMatch(format))
    {
        QStringList sl = format.split('.');
        int prep = 0;
        int prec = 0;
        if (sl.size() == 3)
        {
            prep = sl.first().toInt();
            prec = sl.last().toInt();
        }
        else if (sl.size() == 2)
        {
            if (sl.first() == "i")
                prec = sl.last().toInt();
            else
                prep = sl.first().toInt();
        }
        int i = (text.contains('.') || "inf" == text) ? (int) text.toDouble() : text.toInt();
        text = QString::number(i);
        if (prec)
            text += "." + QString().fill('0', prec);
        if (prep)
            text.prepend(QString().fill('0', prep - text.length()));
    }
    else if ("b" == format.toLower())
    {
        text = text.toInt() ? "true" : "false";
    }
    else if (QRegExp("(([1-9][0-9]*)\\.)?(f|e|E|g|G)(\\.([1-9]|1[0-5]))?").exactMatch(format))
    {
        QStringList sl = format.split('.');
        int prep = 0;
        char f = '\0';
        int prec = 6;
        if (sl.size() == 3)
        {
            prep = sl.first().toInt();
            f = sl.at(1).at(0).toLatin1();
            prec = sl.last().toInt();
        }
        else if (sl.size() == 2)
        {
            if (QRegExp("f|e|E|g|G").exactMatch(sl.first()))
            {
                f = sl.first().at(0).toLatin1();
                prec = sl.last().toInt();
            }
            else
            {
                prep = sl.first().toInt();
                f = sl.last().at(0).toLatin1();
            }
        }
        double d = text.toDouble();
        text = QString::number(d, f, prec);
        if (prep)
            text.prepend(QString().fill('0', prep - text.length()));
    }
    else
    {
        return bRet(err, translate("format", "Invalid argument", "error"), false);
    }
    if (text.contains('-'))
    {
        text.remove('-');
        text.prepend('-');
    }
    stack->setReturnValue(text);
    return bRet(err, QString(), true);
}

static bool ifCondition(ExecutionStack *stack, QString *err = 0)
{
    Token t;
    if (stack->obligArg().toInt())
        t = stack->specialArg(0);
    else if (stack->specialArgCount() > 1)
        t = stack->specialArg(1);
    else
        return bRet(err, QString(), true);
    bool b = false;
    PretexVariant v = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &t), "if", &b, err);
    if (!b)
        return false;
    stack->setReturnValue(v);
    return bRet(err, QString(), true);
}

static bool waitFunction(ExecutionStack *stack, QString *err = 0)
{
    if (stack->obligArg().type() != PretexVariant::Int)
        return bRet(err, translate("waitCondition", "Expected an integer", "error"), false);
    int n = stack->obligArg().toInt();
    if (n < 0)
        return bRet(err, translate("waitCondition", "Invalid argument", "error"), false);
    int k = BeQt::Second;
    if (!stack->optArg().isNull())
    {
        if (stack->optArg().type() != PretexVariant::String)
            return bRet(err, translate("waitCondition", "Expected a string", "error"), false);
        QString s = stack->optArg().toString();
        if (!s.isEmpty())
        {
            if (!QString::compare(s, "ms") || !QString::compare(s, "msec") || !QString::compare(s, "millisec")
                    || !QString::compare(s, "millisecond") || !QString::compare(s, "milliseconds"))
                k = 1;
            else if (!QString::compare(s, "s") || !QString::compare(s, "sec") || !QString::compare(s, "second")
                     || !QString::compare(s, "seconds"))
                k = BeQt::Second;
            else if (!QString::compare(s, "m") || !QString::compare(s, "min") || !QString::compare(s, "minute")
                     || !QString::compare(s, "minutes"))
                k = BeQt::Minute;
            else
                return bRet(err, translate("waitCondition", "Invalid argument", "error"), false);
        }
    }
    stack->setReturnValue(1);
    BeQt::waitNonBlocking(n * k);
    return bRet(err, QString(), true);
}

static bool forLoop(ExecutionStack *stack, QString *err = 0)
{
    if (!stack->isNameOccupied(stack->obligArg(0).toString(), false))
    {
        QString e;
        if (!stack->declareVar(false, stack->obligArg(0).toString(), stack->obligArg(1), &e))
            return bRet(err, e, false);
    }
    PretexVariant v;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs().mid(1, 3)))
    {
    case PretexVariant::Int:
    {
        typedef bool(*PredicateI)(const int &, const int &);
        int lb = stack->obligArg(1).toInt();
        int ub = stack->obligArg(2).toInt();
        int step = stack->obligArg(3).toInt();
        PredicateI pred = (lb < ub) ? &predLeqI : &predGeqI;
        for (int i = lb; pred(i, ub); i += step)
        {
            if (!stack->isNameOccupied(stack->obligArg(0).toString(), false))
                return bRet(err, translate("forLoop", "Unexpected internal error", "error"), false);
            if (!stack->setVar(stack->obligArg(0).toString(), PretexVariant(i), err))
                return false;
            Token t = stack->specialArg();
            bool b = false;
            PretexVariant vv = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &t), "for", &b, err);
            if (!b)
                return false;
            if (PretexBuiltinFunction::ReturnFlag == stack->flag())
            {
                stack->setReturnValue(vv);
                return bRet(err, QString(), true);
            }
            else if (PretexBuiltinFunction::BreakFlag == stack->flag())
            {
                break;
            }
            if (PretexBuiltinFunction::ContinueFlag != stack->flag())
                v = vv;
        }
        break;
    }
    case PretexVariant::Real:
    {
        typedef bool(*PredicateD)(const double &, const double &);
        double lb = stack->obligArg(1).toReal();
        double ub = stack->obligArg(2).toReal();
        double step = stack->obligArg(3).toReal();
        PredicateD pred = (lb < ub) ? &predLeqD : &predGeqD;
        for (double d = lb; pred(d, ub); d += step)
        {
            if (!stack->isNameOccupied(stack->obligArg(0).toString(), false))
                return bRet(err, translate("forLoop", "Unexpected internal error", "error"), false);
            if (!stack->setVar(stack->obligArg(0).toString(), PretexVariant(d), err))
                return false;
            Token t = stack->specialArg();
            bool b = false;
            PretexVariant vv = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &t), "for", &b, err);
            if (!b)
                return false;
            if (PretexBuiltinFunction::ReturnFlag == stack->flag())
            {
                stack->setReturnValue(vv);
                return bRet(err, QString(), true);
            }
            else if (PretexBuiltinFunction::BreakFlag == stack->flag())
            {
                break;
            }
            if (PretexBuiltinFunction::ContinueFlag != stack->flag())
                v = vv;
        }
        break;
    }
    default:
        return bRet(err, translate("forLoop", "Expected integer or real", "error"), false);
    }
    stack->setReturnValue(v);
    return bRet(err, QString(), true);
}

static bool whileLoop(ExecutionStack *stack, QString *err = 0)
{
    Token condt = stack->specialArg(0);
    Token bodyt = stack->specialArg(1);
    bool b = false;
    PretexVariant cond = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &condt), "", &b, err);
    if (!b)
        return false;
    if (cond.type() != PretexVariant::Int)
        return bRet(err, translate("whileLoop", "Conditional argument must be an integer", "error"), false);
    PretexVariant v;
    while (cond.toInt())
    {
        b = false;
        PretexVariant vv = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &bodyt), "while", &b, err);
        if (!b)
            return false;
        if (PretexBuiltinFunction::ReturnFlag == stack->flag())
        {
            stack->setReturnValue(vv);
            return bRet(err, QString(), true);
        }
        else if (PretexBuiltinFunction::BreakFlag == stack->flag())
        {
            break;
        }
        if (PretexBuiltinFunction::ContinueFlag != stack->flag())
            v = vv;
        b = false;
        cond = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &condt), "", &b, err);
        if (!b)
            return false;
        if (cond.type() != PretexVariant::Int)
            return bRet(err, translate("whileLoop", "Conditional argument must be an integer", "error"), false);
    }
    stack->setReturnValue(v);
    return bRet(err, QString(), true);
}

static bool doWhileLoop(ExecutionStack *stack, QString *err = 0)
{
    PretexVariant v;
    PretexVariant cond;
    Token bodyt = stack->specialArg(0);
    Token condt = stack->specialArg(1);
    bool b = false;
    do
    {
        b = false;
        PretexVariant vv = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &bodyt), "doWhile", &b, err);
        if (!b)
            return false;
        if (PretexBuiltinFunction::ReturnFlag == stack->flag())
        {
            stack->setReturnValue(vv);
            return bRet(err, QString(), true);
        }
        else if (PretexBuiltinFunction::BreakFlag == stack->flag())
        {
            break;
        }
        if (PretexBuiltinFunction::ContinueFlag != stack->flag())
            v = vv;
        b = false;
        cond = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &condt), "", &b, err);
        if (!b)
            return false;
        if (cond.type() != PretexVariant::Int)
            return bRet(err, translate("doWhileLoop", "Conditional argument must be an integer", "error"), false);
    }
    while (cond.toInt());
    stack->setReturnValue(v);
    return bRet(err, QString(), true);
}

static bool untilLoop(ExecutionStack *stack, QString *err = 0)
{
    Token condt = stack->specialArg(0);
    Token bodyt = stack->specialArg(1);
    bool b = false;
    PretexVariant cond = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &condt), "", &b, err);
    if (!b)
        return false;
    if (cond.type() != PretexVariant::Int)
        return bRet(err, translate("whileLoop", "Conditional argument must be an integer", "error"), false);
    PretexVariant v;
    while (!cond.toInt())
    {
        b = false;
        PretexVariant vv = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &bodyt), "until", &b, err);
        if (!b)
            return false;
        if (PretexBuiltinFunction::ReturnFlag == stack->flag())
        {
            stack->setReturnValue(vv);
            return bRet(err, QString(), true);
        }
        else if (PretexBuiltinFunction::BreakFlag == stack->flag())
        {
            break;
        }
        if (PretexBuiltinFunction::ContinueFlag != stack->flag())
            v = vv;
        b = false;
        cond = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &condt), "", &b, err);
        if (!b)
            return false;
        if (cond.type() != PretexVariant::Int)
            return bRet(err, translate("whileLoop", "Conditional argument must be an integer", "error"), false);
    }
    stack->setReturnValue(v);
    return bRet(err, QString(), true);
}

static bool doUntilLoop(ExecutionStack *stack, QString *err = 0)
{
    PretexVariant v;
    PretexVariant cond;
    bool b = false;
    Token bodyt = stack->specialArg(0);
    Token condt = stack->specialArg(1);
    do
    {
        b = false;
        PretexVariant vv = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &bodyt), "doUntil", &b, err);
        if (!b)
            return false;
        if (PretexBuiltinFunction::ReturnFlag == stack->flag())
        {
            stack->setReturnValue(vv);
            return bRet(err, QString(), true);
        }
        else if (PretexBuiltinFunction::BreakFlag == stack->flag())
        {
            break;
        }
        if (PretexBuiltinFunction::ContinueFlag != stack->flag())
            v = vv;
        b = false;
        cond = ExecutionModule::executeSubprogram(stack, DATA_CAST(Subprogram, &condt), "", &b, err);
        if (!b)
            return false;
        if (cond.type() != PretexVariant::Int)
            return bRet(err, translate("doWhileLoop", "Conditional argument must be an integer", "error"), false);
    }
    while (!cond.toInt());
    stack->setReturnValue(v);
    return bRet(err, QString(), true);
}

static bool flagFunction(ExecutionStack *stack, PretexBuiltinFunction::SpecialFlag flag, QString *err = 0)
{
    if (PretexBuiltinFunction::ReturnFlag != flag && !stack->obligArg().isNull())
        return bRet(err, translate("flagFunction", "Argument given to a function which does not accept arguments",
                                   "error"), false);
    if (!stack->parent()->setFlag(flag, err))
        return false;
    if (PretexBuiltinFunction::ReturnFlag == flag)
        stack->setReturnValue(stack->obligArg());
    return bRet(err, QString(), true);
}

/*============================================================================
================================ GeneralFunction =============================
============================================================================*/

/*============================== Public constructors =======================*/

GeneralFunction::GeneralFunction(Type t)
{
    mtype = t;
}

/*============================== Public methods ============================*/

QString GeneralFunction::name() const
{
    switch (mtype)
    {
    case IsEmptyType:
        return "isEmpty";
    case ToIntegerType:
        return "toInteger";
    case ToRealType:
        return "toReal";
    case ToStringType:
        return "toString";
    case FormatType:
        return "format";
    case IfType:
        return "if";
    case WaitType:
        return "wait";
    case ForType:
        return "for";
    case WhileType:
        return "while";
    case DoWhileType:
        return "doWhile";
    case UntilType:
        return "until";
    case DoUntilType:
        return "doUntil";
    case ReturnType:
        return "return";
    case BreakType:
        return "break";
    case ContinueType:
        return "continue";
    default:
        break;
    }
    return QString();
}

int GeneralFunction::obligatoryArgumentCount() const
{
    switch (mtype)
    {
    case IsEmptyType:
    case ToIntegerType:
    case ToRealType:
    case ToStringType:
    case WaitType:
    case ReturnType:
    case BreakType:
    case ContinueType:
        return 1;
    case FormatType:
    case IfType:
    case WhileType:
    case DoWhileType:
    case UntilType:
    case DoUntilType:
        return 2;
    case ForType:
        return 5;
    default:
        break;
    }
    return 0;
}

int GeneralFunction::optionalArgumentCount() const
{
    switch (mtype)
    {
    case IsEmptyType:
    case ToIntegerType:
    case ToRealType:
    case ToStringType:
    case FormatType:
    case ForType:
    case WhileType:
    case DoWhileType:
    case UntilType:
    case DoUntilType:
    case ReturnType:
    case BreakType:
    case ContinueType:
        return 0;
    case IfType:
    case WaitType:
        return 1;
    default:
        break;
    }
    return 0;
}

bool GeneralFunction::execute(ExecutionStack *stack, Function_TokenData *f, QString *err)
{
    switch (mtype)
    {
    case IfType:
    {
        if (!standardCheck(f, err))
            return false;
        bool b = false;
        PretexVariant a = ExecutionModule::executeSubprogram(stack, f->obligatoryArgument(0), "", &b, err);
        if (!b)
            return false;
        if (a.type() != PretexVariant::Int)
            return bRet(err, tr("Conditional function argument must be an integer", "error") + " " + name(), false);
        QList<Token> specArgs;
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(1));
        specArgs << t;
        if (f->optionalArgumentCount())
        {
            DATA_CAST(Subprogram, &t)->copyStatements(f->optionalArgument(0));
            specArgs << t;
        }
        ExecutionStack s(QList<PretexVariant>() << a, QList<PretexVariant>(), specArgs, name(), stack);
        if (!execute(&s, err))
            return false;
        stack->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    case ForType:
    {
        if (!standardCheck(f, err))
            return false;
        QList<PretexVariant> oblArgs;
        foreach (int i, bRangeD(0, 3))
        {
            bool b = false;
            PretexVariant a = ExecutionModule::executeSubprogram(stack, f->obligatoryArgument(i), "", &b, err);
            if (!b)
                return false;
            oblArgs << a;
        }
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(4));
        ExecutionStack s(oblArgs, QList<PretexVariant>(), QList<Token>() << t, name(), stack);
        if (!execute(&s, err))
            return false;
        stack->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    case WhileType:
    case UntilType:
    case DoWhileType:
    case DoUntilType:
    {
        if (!standardCheck(f, err))
            return false;
        QList<Token> specArgs;
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(0));
        specArgs << t;
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(1));
        specArgs << t;
        ExecutionStack s(QList<PretexVariant>(), QList<PretexVariant>(), specArgs, name(), stack);
        if (!execute(&s, err))
            return false;
        stack->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    default:
        return PretexBuiltinFunction::execute(stack, f, err);
    }
}

GeneralFunction::SpecialFlags GeneralFunction::acceptedFlags() const
{
    switch (mtype)
    {
    case ForType:
    case WhileType:
    case DoWhileType:
    case UntilType:
    case DoUntilType:
        return SpecialFlags(BreakFlag | ContinueFlag);
    case IsEmptyType:
    case ToIntegerType:
    case ToRealType:
    case ToStringType:
    case FormatType:
    case WaitType:
    case IfType:
    case ReturnType:
    case BreakType:
    case ContinueType:
    default:
        return NoFlag;
    }
}

GeneralFunction::SpecialFlags GeneralFunction::flagsPropagateMask() const
{
    switch (mtype)
    {
    case IfType:
        return SpecialFlags(ReturnFlag | BreakFlag | ContinueFlag);
    case ForType:
    case WhileType:
    case DoWhileType:
    case UntilType:
    case DoUntilType:
        return ReturnFlag;
    case IsEmptyType:
    case ToIntegerType:
    case ToRealType:
    case ToStringType:
    case FormatType:
    case WaitType:
    case ReturnType:
    case BreakType:
    case ContinueType:
    default:
        return NoFlag;
    }
}

/*============================== Protected methods =========================*/

bool GeneralFunction::execute(ExecutionStack *stack, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case IsEmptyType:
        return isEmpty(stack, err);
    case ToIntegerType:
        return toInteger(stack, err);
    case ToRealType:
        return toReal(stack, err);
    case ToStringType:
        return toString(stack, err);
    case FormatType:
        return format(stack, err);
    case IfType:
        return ifCondition(stack, err);
    case WaitType:
        return waitFunction(stack, err);
    case ForType:
        return forLoop(stack, err);
    case WhileType:
        return whileLoop(stack, err);
    case DoWhileType:
        return doWhileLoop(stack, err);
    case UntilType:
        return untilLoop(stack, err);
    case DoUntilType:
        return doUntilLoop(stack, err);
    case ReturnType:
        return flagFunction(stack, ReturnFlag, err);
    case BreakType:
        return flagFunction(stack, BreakFlag, err);
    case ContinueType:
        return flagFunction(stack, ContinueFlag, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
