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

#include "booleanfunction.h"

#include "executioncontext.h"
#include "global.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QList>
#include <QString>

/*============================================================================
================================ BooleanFunction =============================
============================================================================*/

/*============================== Public constructors =======================*/

BooleanFunction::BooleanFunction(Type t)
{
    mtype = t;
}

/*============================== Public methods ============================*/

QString BooleanFunction::name() const
{
    switch (mtype) {
    case NotType:
        return "not";
    case EqualType:
        return "equal";
    case NotEqualType:
        return "notEqual";
    case LessType:
        return "less";
    case LessOrEqualType:
        return "lessOrEqual";
    case GreaterType:
        return "greater";
    case GreaterOrEqualType:
        return "greaterOrEqual";
    case OrType:
        return "or";
    case AndType:
        return "and";
    case XorType:
        return "xor";
    default:
        break;
    }
    return QString();
}

int BooleanFunction::obligatoryArgumentCount() const
{
    switch (mtype) {
    case EqualType:
    case NotEqualType:
    case LessType:
    case LessOrEqualType:
    case GreaterType:
    case GreaterOrEqualType:
    case OrType:
    case AndType:
    case XorType:
        return 2;
    case NotType:
        return 1;
    default:
        break;
    }
    return 0;
}

int BooleanFunction::optionalArgumentCount() const
{
    switch (mtype) {
    case EqualType:
    case OrType:
    case AndType:
    case XorType:
        return -1;
    case NotType:
    case NotEqualType:
    case LessType:
    case LessOrEqualType:
    case GreaterType:
    case GreaterOrEqualType:
        return 0;
    default:
        break;
    }
    return 0;
}

/*============================== Protected methods =========================*/

bool BooleanFunction::execute(ExecutionContext *context, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype) {
    case NotType:
        return booleanNot(context, err);
    case EqualType:
        return booleanEqual(context, err);
    case NotEqualType:
        return booleanNotEqual(context, err);
    case LessType:
        return booleanLess(context, err);
    case LessOrEqualType:
        return booleanLessOrEqual(context, err);
    case GreaterType:
        return booleanGreater(context, err);
    case GreaterOrEqualType:
        return booleanGreaterOrEqual(context, err);
    case OrType:
        return booleanOr(context, err);
    case AndType:
        return booleanAnd(context, err);
    case XorType:
        return booleanXor(context, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}

/*============================== Static private methods ====================*/

bool BooleanFunction::booleanAnd(ExecutionContext *context, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs(), context->optArgs())) {
    case PretexVariant::Int:
    case PretexVariant::Real:
        b = context->obligArg().toInt();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1)) {
            if (!b)
                break;
            b = b && context->obligArg(n).toInt();
        }
        foreach (int n, bRangeD(1, context->optArgCount() - 1)) {
            if (!b)
                break;
            b = b && context->optArg(n).toInt();
        }
        break;
    case PretexVariant::String:
        return bRet(err, tr("Unable to cast strings to boolean", "error"), false);
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool BooleanFunction::booleanEqual(ExecutionContext *context, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs(), context->optArgs())) {
    case PretexVariant::Int: {
        int i = context->obligArg().toInt();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1)) {
            b = b && (context->obligArg(n).toInt() == i);
            if (!b)
                break;
        }
        foreach (int n, bRangeD(1, context->optArgCount() - 1)) {
            if (!b)
                break;
            b = b && (context->optArg(n).toInt() == i);
        }
        break;
    }
    case PretexVariant::Real: {
        double d = context->obligArg().toReal();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1)) {
            b = b && (context->obligArg(n).toReal() == d);
            if (!b)
                break;
        }
        foreach (int n, bRangeD(1, context->optArgCount() - 1)) {
            if (!b)
                break;
            b = b && (context->optArg(n).toReal() == d);
        }
        break;
    }
    case PretexVariant::String: {
        QString s = context->obligArg().toString();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1)) {
            b = b && (context->obligArg(n).toString() == s);
            if (!b)
                break;
        }
        foreach (int n, bRangeD(1, context->optArgCount() - 1)) {
            if (!b)
                break;
            b = b && (context->optArg(n).toString() == s);
        }
        break;
    }
    default: {
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    }
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool BooleanFunction::booleanGreater(ExecutionContext *context, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int:
        b = context->obligArg().toInt() > context->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = context->obligArg().toReal() > context->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = context->obligArg().toString() > context->obligArg(1).toString();
        break;
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool BooleanFunction::booleanGreaterOrEqual(ExecutionContext *context, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int:
        b = context->obligArg().toInt() >= context->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = context->obligArg().toReal() >= context->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = context->obligArg().toString() >= context->obligArg(1).toString();
        break;
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool BooleanFunction::booleanLess(ExecutionContext *context, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int:
        b = context->obligArg().toInt() < context->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = context->obligArg().toReal() < context->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = context->obligArg().toString() < context->obligArg(1).toString();
        break;
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool BooleanFunction::booleanLessOrEqual(ExecutionContext *context, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int:
        b = context->obligArg().toInt() <= context->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = context->obligArg().toReal() <= context->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = context->obligArg().toString() <= context->obligArg(1).toString();
        break;
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool BooleanFunction::booleanNot(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(context->obligArg().toInt() ? 0 : 1);
        break;
    case PretexVariant::Real:
        return bRet(err, tr("Unable to cast real numbers to boolean", "error"), false);
    case PretexVariant::String:
        return bRet(err, tr("Unable to cast strings to boolean", "error"), false);
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool BooleanFunction::booleanNotEqual(ExecutionContext *context, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int:
        b = context->obligArg().toInt() != context->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = context->obligArg().toReal() != context->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = context->obligArg().toString() != context->obligArg(1).toString();
        break;
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool BooleanFunction::booleanOr(ExecutionContext *context, QString *err)
{
    bool b = false;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs(), context->optArgs())) {
    case PretexVariant::Int:
    case PretexVariant::Real:
    {
        b = context->obligArg().toInt();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1)) {
            if (b)
                break;
            b = b || context->obligArg(n).toInt();
        }
        foreach (int n, bRangeD(1, context->optArgCount() - 1)) {
            if (b)
                break;
            b = b || context->optArg(n).toInt();
        }
        break;
    }
    case PretexVariant::String:
        return bRet(err, tr("Unable to cast strings to boolean", "error"), false);
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool BooleanFunction::booleanXor(ExecutionContext *context, QString *err)
{
    bool b = false;
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs(), context->optArgs()))
    {
    case PretexVariant::Int:
    case PretexVariant::Real:
        b = context->obligArg().toInt();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1))
            b = b ^ context->obligArg(n).toInt();
        foreach (int n, bRangeD(1, context->optArgCount() - 1))
            b = b ^ context->optArg(n).toInt();
        break;
    case PretexVariant::String:
        return bRet(err, tr("Unable to cast strings to boolean", "error"), false);
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}
