/****************************************************************************
**
** Copyright (C) 2014 Andrey Bogdanov
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

#include "executionmodule.h"
#include "executioncontext.h"
#include "global.h"
#include "pretexbuiltinfunction.h"
#include "pretexeditormodule.h"
#include "pretexvariant.h"
#include "tokendata.h"
#include "token.h"

#include <BeQt>

#include <QDebug>
#include <QList>
#include <QRegExp>
#include <QString>

/*============================================================================
================================ GeneralFunction =============================
============================================================================*/

/*============================== Public constructors =======================*/

GeneralFunction::GeneralFunction(Type t)
{
    mtype = t;
}

/*============================== Public methods ============================*/

GeneralFunction::SpecialFlags GeneralFunction::acceptedFlags() const
{
    switch (mtype) {
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

bool GeneralFunction::execute(ExecutionContext *context, Function_TokenData *f, QString *err)
{
    switch (mtype)
    {
    case IfType: {
        if (!standardCheck(f, err))
            return false;
        bool b = false;
        PretexVariant a = ExecutionModule::executeSubprogram(context, f->obligatoryArgument(0), "", &b, err);
        if (!b)
            return false;
        if (a.type() != PretexVariant::Int)
            return bRet(err, tr("Conditional function argument must be an integer", "error") + " " + name(), false);
        QList<Token> specArgs;
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(1));
        specArgs << t;
        if (f->optionalArgumentCount()) {
            DATA_CAST(Subprogram, &t)->copyStatements(f->optionalArgument(0));
            specArgs << t;
        }
        ExecutionContext s(QList<PretexVariant>() << a, QList<PretexVariant>(), specArgs, name(), context);
        if (!execute(&s, err))
            return false;
        context->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    case ForType: {
        if (!standardCheck(f, err))
            return false;
        QList<PretexVariant> oblArgs;
        foreach (int i, bRangeD(0, 3)) {
            bool b = false;
            PretexVariant a = ExecutionModule::executeSubprogram(context, f->obligatoryArgument(i), "", &b, err);
            if (!b)
                return false;
            oblArgs << a;
        }
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(4));
        ExecutionContext s(oblArgs, QList<PretexVariant>(), QList<Token>() << t, name(), context);
        if (!execute(&s, err))
            return false;
        context->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    case WhileType:
    case UntilType:
    case DoWhileType:
    case DoUntilType: {
        if (!standardCheck(f, err))
            return false;
        QList<Token> specArgs;
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(0));
        specArgs << t;
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(1));
        specArgs << t;
        ExecutionContext s(QList<PretexVariant>(), QList<PretexVariant>(), specArgs, name(), context);
        if (!execute(&s, err))
            return false;
        context->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    default: {
        return PretexBuiltinFunction::execute(context, f, err);
    }
    }
}

GeneralFunction::SpecialFlags GeneralFunction::flagsPropagateMask() const
{
    switch (mtype) {
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

QString GeneralFunction::name() const
{
    switch (mtype) {
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
    switch (mtype) {
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
    switch (mtype) {
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

/*============================== Protected methods =========================*/

bool GeneralFunction::execute(ExecutionContext *context, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype) {
    case IsEmptyType:
        return isEmpty(context, err);
    case ToIntegerType:
        return toInteger(context, err);
    case ToRealType:
        return toReal(context, err);
    case ToStringType:
        return toString(context, err);
    case FormatType:
        return format(context, err);
    case IfType:
        return ifCondition(context, err);
    case WaitType:
        return waitFunction(context, err);
    case ForType:
        return forLoop(context, err);
    case WhileType:
        return whileLoop(context, err);
    case DoWhileType:
        return doWhileLoop(context, err);
    case UntilType:
        return untilLoop(context, err);
    case DoUntilType:
        return doUntilLoop(context, err);
    case ReturnType:
        return flagFunction(context, ReturnFlag, err);
    case BreakType:
        return flagFunction(context, BreakFlag, err);
    case ContinueType:
        return flagFunction(context, ContinueFlag, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}

/*============================== Static private methods ====================*/

bool GeneralFunction::doUntilLoop(ExecutionContext *context, QString *err)
{
    PretexVariant v;
    PretexVariant cond;
    bool b = false;
    Token bodyt = context->specialArg(0);
    Token condt = context->specialArg(1);
    do {
        b = false;
        PretexVariant vv = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &bodyt), "doUntil", &b,
                                                              err);
        if (!b)
            return false;
        if (PretexBuiltinFunction::ReturnFlag == context->flag()) {
            context->setReturnValue(vv);
            return bRet(err, QString(), true);
        } else if (PretexBuiltinFunction::BreakFlag == context->flag()) {
            break;
        }
        if (PretexBuiltinFunction::ContinueFlag != context->flag())
            v = vv;
        b = false;
        cond = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &condt), "", &b, err);
        if (!b)
            return false;
        if (cond.type() != PretexVariant::Int)
            return bRet(err, tr("Conditional argument must be an integer", "error"), false);
    } while (!cond.toInt());
    context->setReturnValue(v);
    return bRet(err, QString(), true);
}

bool GeneralFunction::doWhileLoop(ExecutionContext *context, QString *err)
{
    PretexVariant v;
    PretexVariant cond;
    Token bodyt = context->specialArg(0);
    Token condt = context->specialArg(1);
    bool b = false;
    do {
        b = false;
        PretexVariant vv = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &bodyt), "doWhile", &b, err);
        if (!b)
            return false;
        if (PretexBuiltinFunction::ReturnFlag == context->flag()) {
            context->setReturnValue(vv);
            return bRet(err, QString(), true);
        } else if (PretexBuiltinFunction::BreakFlag == context->flag()) {
            break;
        }
        if (PretexBuiltinFunction::ContinueFlag != context->flag())
            v = vv;
        b = false;
        cond = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &condt), "", &b, err);
        if (!b)
            return false;
        if (cond.type() != PretexVariant::Int)
            return bRet(err, tr("Conditional argument must be an integer", "error"), false);
    } while (cond.toInt());
    context->setReturnValue(v);
    return bRet(err, QString(), true);
}

bool GeneralFunction::flagFunction(ExecutionContext *context, PretexBuiltinFunction::SpecialFlag flag, QString *err)
{
    if (PretexBuiltinFunction::ReturnFlag != flag && !context->obligArg().isNull())
        return bRet(err, tr("Argument given to a function which does not accept arguments", "error"), false);
    if (!context->parent()->setFlag(flag, err))
        return false;
    if (PretexBuiltinFunction::ReturnFlag == flag)
        context->setReturnValue(context->obligArg());
    return bRet(err, QString(), true);
}

bool GeneralFunction::forLoop(ExecutionContext *context, QString *err)
{
    if (!context->isNameOccupied(context->obligArg(0).toString(), false)) {
        QString e;
        if (!context->declareVar(false, context->obligArg(0).toString(), context->obligArg(1), &e))
            return bRet(err, e, false);
    }
    PretexVariant v;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs().mid(1, 3))) {
    case PretexVariant::Int: {
        typedef bool(*PredicateI)(const int &, const int &);
        int lb = context->obligArg(1).toInt();
        int ub = context->obligArg(2).toInt();
        int step = context->obligArg(3).toInt();
        if (!step)
            break;
        PredicateI pred = (lb <= ub) ? &predLeqI : &predGeqI;
        if ((lb > ub && step > 0) || (lb < ub && step < 0))
            break;
        for (int i = lb; pred(i, ub); i += step) {
            if (!context->isNameOccupied(context->obligArg(0).toString(), false))
                return bRet(err, tr("Unexpected internal error", "error"), false);
            if (!context->setVar(context->obligArg(0).toString(), PretexVariant(i), err))
                return false;
            Token t = context->specialArg();
            bool b = false;
            PretexVariant vv = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &t), "for", &b, err);
            if (!b)
                return false;
            if (PretexBuiltinFunction::ReturnFlag == context->flag()) {
                context->setReturnValue(vv);
                return bRet(err, QString(), true);
            } else if (PretexBuiltinFunction::BreakFlag == context->flag()) {
                break;
            }
            if (PretexBuiltinFunction::ContinueFlag != context->flag())
                v = vv;
        }
        break;
    }
    case PretexVariant::Real: {
        typedef bool(*PredicateD)(const double &, const double &);
        double lb = context->obligArg(1).toReal();
        double ub = context->obligArg(2).toReal();
        double step = context->obligArg(3).toReal();
        if (qFuzzyIsNull(step))
            break;
        PredicateD pred = (lb < ub || qFuzzyCompare(lb, ub)) ? &predLeqD : &predGeqD;
        if ((lb > ub && step > 0.0) || (lb < ub && step < 0.0))
            break;
        for (double d = lb; pred(d, ub); d += step) {
            if (!context->isNameOccupied(context->obligArg(0).toString(), false))
                return bRet(err, tr("Unexpected internal error", "error"), false);
            if (!context->setVar(context->obligArg(0).toString(), PretexVariant(d), err))
                return false;
            Token t = context->specialArg();
            bool b = false;
            PretexVariant vv = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &t), "for", &b, err);
            if (!b)
                return false;
            if (PretexBuiltinFunction::ReturnFlag == context->flag()) {
                context->setReturnValue(vv);
                return bRet(err, QString(), true);
            } else if (PretexBuiltinFunction::BreakFlag == context->flag()) {
                break;
            }
            if (PretexBuiltinFunction::ContinueFlag != context->flag())
                v = vv;
        }
        break;
    }
    default: {
        return bRet(err, tr("Expected integer or real", "error"), false);
    }
    }
    context->setReturnValue(v);
    return bRet(err, QString(), true);
}

bool GeneralFunction::format(ExecutionContext *context, QString *err)
{
    if (context->obligArg(1).type() != PretexVariant::String)
        return bRet(err, tr("Expected a string", "error"), false);
    if (context->obligArg(0).type() == PretexVariant::String) {
        context->setReturnValue(context->obligArg(0));
        return bRet(err, QString(), true);
    }
    QString text = context->obligArg(0).toString();
    QString format = context->obligArg(1).toString();
    if (QRegExp("(([1-9][0-9]*)\\.)?i(\\.([1-9]|1[0-5]))?").exactMatch(format)) {
        QStringList sl = format.split('.');
        int prep = 0;
        int prec = 0;
        if (sl.size() == 3) {
            prep = sl.first().toInt();
            prec = sl.last().toInt();
        } else if (sl.size() == 2) {
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
    } else if ("b" == format.toLower()) {
        text = text.toInt() ? "true" : "false";
    } else if (QRegExp("(([1-9][0-9]*)\\.)?(f|e|E|g|G)(\\.([1-9]|1[0-5]))?").exactMatch(format)) {
        QStringList sl = format.split('.');
        int prep = 0;
        char f = '\0';
        int prec = 6;
        if (sl.size() == 3) {
            prep = sl.first().toInt();
            f = sl.at(1).at(0).toLatin1();
            prec = sl.last().toInt();
        } else if (sl.size() == 2) {
            if (QRegExp("f|e|E|g|G").exactMatch(sl.first())) {
                f = sl.first().at(0).toLatin1();
                prec = sl.last().toInt();
            } else {
                prep = sl.first().toInt();
                f = sl.last().at(0).toLatin1();
            }
        }
        double d = text.toDouble();
        text = QString::number(d, f, prec);
        if (prep)
            text.prepend(QString().fill('0', prep - text.length()));
    } else {
        return bRet(err, tr("Invalid argument", "error"), false);
    }
    if (text.contains('-')) {
        text.remove('-');
        text.prepend('-');
    }
    context->setReturnValue(text);
    return bRet(err, QString(), true);
}

bool GeneralFunction::ifCondition(ExecutionContext *context, QString *err)
{
    Token t;
    if (context->obligArg().toInt())
        t = context->specialArg(0);
    else if (context->specialArgCount() > 1)
        t = context->specialArg(1);
    else
        return bRet(err, QString(), true);
    bool b = false;
    PretexVariant v = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &t), "if", &b, err);
    if (!b)
        return false;
    context->setReturnValue(v);
    return bRet(err, QString(), true);
}

bool GeneralFunction::isEmpty(ExecutionContext *context, QString *err)
{
    context->setReturnValue(context->obligArg().isNull() ? 1 : 0);
    return bRet(err, QString(), true);
}

bool GeneralFunction::predGeqD(const double &t1, const double &t2)
{
    return t1 >= t2;
}

bool GeneralFunction::predGeqI(const int &t1, const int &t2)
{
    return t1 >= t2;
}

bool GeneralFunction::predLeqD(const double &t1, const double &t2)
{
    return t1 <= t2;
}

bool GeneralFunction::predLeqI(const int &t1, const int &t2)
{
    return t1 <= t2;
}

bool GeneralFunction::toInteger(ExecutionContext *context, QString *err)
{
    context->setReturnValue(context->obligArg().toInt());
    return bRet(err, QString(), true);
}

bool GeneralFunction::toReal(ExecutionContext *context, QString *err)
{
    context->setReturnValue(context->obligArg().toReal());
    return bRet(err, QString(), true);
}

bool GeneralFunction::toString(ExecutionContext *context, QString *err)
{
    context->setReturnValue(context->obligArg().toString());
    return bRet(err, QString(), true);
}

bool GeneralFunction::untilLoop(ExecutionContext *context, QString *err)
{
    Token condt = context->specialArg(0);
    Token bodyt = context->specialArg(1);
    bool b = false;
    PretexVariant cond = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &condt), "", &b, err);
    if (!b)
        return false;
    if (cond.type() != PretexVariant::Int)
        return bRet(err, tr("Conditional argument must be an integer", "error"), false);
    PretexVariant v;
    while (!cond.toInt()) {
        b = false;
        PretexVariant vv = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &bodyt), "until", &b,
                                                              err);
        if (!b)
            return false;
        if (PretexBuiltinFunction::ReturnFlag == context->flag()) {
            context->setReturnValue(vv);
            return bRet(err, QString(), true);
        } else if (PretexBuiltinFunction::BreakFlag == context->flag()) {
            break;
        }
        if (PretexBuiltinFunction::ContinueFlag != context->flag())
            v = vv;
        b = false;
        cond = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &condt), "", &b, err);
        if (!b)
            return false;
        if (cond.type() != PretexVariant::Int)
            return bRet(err, tr("Conditional argument must be an integer", "error"), false);
    }
    context->setReturnValue(v);
    return bRet(err, QString(), true);
}

bool GeneralFunction::waitFunction(ExecutionContext *context, QString *err)
{
    if (context->obligArg().type() != PretexVariant::Int)
        return bRet(err, tr("Expected an integer", "error"), false);
    int n = context->obligArg().toInt();
    if (n < 0)
        return bRet(err, tr("Invalid argument", "error"), false);
    int k = BeQt::Second;
    if (!context->optArg().isNull()) {
        if (context->optArg().type() != PretexVariant::String)
            return bRet(err, tr("Expected a string", "error"), false);
        QString s = context->optArg().toString();
        if (!s.isEmpty()) {
            if (!QString::compare(s, "ms") || !QString::compare(s, "msec") || !QString::compare(s, "millisec")
                    || !QString::compare(s, "millisecond") || !QString::compare(s, "milliseconds")) {
                k = 1;
            } else if (!QString::compare(s, "s") || !QString::compare(s, "sec") || !QString::compare(s, "second")
                     || !QString::compare(s, "seconds")) {
                k = BeQt::Second;
            } else if (!QString::compare(s, "m") || !QString::compare(s, "min") || !QString::compare(s, "minute")
                     || !QString::compare(s, "minutes")) {
                k = BeQt::Minute;
            } else {
                return bRet(err, tr("Invalid argument", "error"), false);
            }
        }
    }
    context->setReturnValue(1);
    PretexEditorModule *module = context->editorModule();
    BeQt::waitNonBlocking(module, SIGNAL(terminated()), n * k);
    if (module && module->shouldTerminate())
        return bRet(err, tr("Execution terminated by user", "error"), false);
    return bRet(err, QString(), true);
}

bool GeneralFunction::whileLoop(ExecutionContext *context, QString *err)
{
    Token condt = context->specialArg(0);
    Token bodyt = context->specialArg(1);
    bool b = false;
    PretexVariant cond = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &condt), "", &b, err);
    if (!b)
        return false;
    if (cond.type() != PretexVariant::Int)
        return bRet(err, tr("Conditional argument must be an integer", "error"), false);
    PretexVariant v;
    while (cond.toInt()) {
        b = false;
        PretexVariant vv = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &bodyt), "while", &b,
                                                              err);
        if (!b)
            return false;
        if (PretexBuiltinFunction::ReturnFlag == context->flag()) {
            context->setReturnValue(vv);
            return bRet(err, QString(), true);
        } else if (PretexBuiltinFunction::BreakFlag == context->flag()) {
            break;
        }
        if (PretexBuiltinFunction::ContinueFlag != context->flag())
            v = vv;
        b = false;
        cond = ExecutionModule::executeSubprogram(context, DATA_CAST(Subprogram, &condt), "", &b, err);
        if (!b)
            return false;
        if (cond.type() != PretexVariant::Int)
            return bRet(err, tr("Conditional argument must be an integer", "error"), false);
    }
    context->setReturnValue(v);
    return bRet(err, QString(), true);
}
