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
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "executionstack.h"
#include "global.h"

#include <BeQtGlobal>

#include <QList>
#include <QString>

#include <QDebug>

B_DECLARE_TRANSLATE_FUNCTION

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool booleanNot(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(stack->obligArg().toInt() ? 0 : 1);
        break;
    case PretexVariant::Real:
        return bRet(err, translate("booleanNot", "Unable to cast real numbers to boolean", "error"), false);
    case PretexVariant::String:
        return bRet(err, translate("booleanNot", "Unable to cast strings to boolean", "error"), false);
    default:
        return bRet(err, translate("booleanNot", "Null argument(s)", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool equal(ExecutionStack *stack, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs(), stack->optArgs()))
    {
    case PretexVariant::Int:
    {
        int i = stack->obligArg().toInt();
        foreach (int n, bRangeD(1, stack->obligArgCount() - 1))
        {
            b = b && (stack->obligArg(n).toInt() == i);
            if (!b)
                break;
        }
        foreach (int n, bRangeD(1, stack->optArgCount() - 1))
        {
            if (!b)
                break;
            b = b && (stack->optArg(n).toInt() == i);
        }
        break;
    }
    case PretexVariant::Real:
    {
        double d = stack->obligArg().toReal();
        foreach (int n, bRangeD(1, stack->obligArgCount() - 1))
        {
            b = b && (stack->obligArg(n).toReal() == d);
            if (!b)
                break;
        }
        foreach (int n, bRangeD(1, stack->optArgCount() - 1))
        {
            if (!b)
                break;
            b = b && (stack->optArg(n).toReal() == d);
        }
        break;
    }
    case PretexVariant::String:
    {
        QString s = stack->obligArg().toString();
        foreach (int n, bRangeD(1, stack->obligArgCount() - 1))
        {
            b = b && (stack->obligArg(n).toString() == s);
            if (!b)
                break;
        }
        foreach (int n, bRangeD(1, stack->optArgCount() - 1))
        {
            if (!b)
                break;
            b = b && (stack->optArg(n).toString() == s);
        }
        break;
    }
    default:
        return bRet(err, translate("equal", "Null argument(s)", "error"), false);
    }
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool notEqual(ExecutionStack *stack, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs()))
    {
    case PretexVariant::Int:
        b = stack->obligArg().toInt() != stack->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = stack->obligArg().toReal() != stack->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = stack->obligArg().toString() != stack->obligArg(1).toString();
        break;
    default:
        return bRet(err, translate("notEqual", "Null argument(s)", "error"), false);
    }
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool lesser(ExecutionStack *stack, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs()))
    {
    case PretexVariant::Int:
        b = stack->obligArg().toInt() < stack->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = stack->obligArg().toReal() < stack->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = stack->obligArg().toString() < stack->obligArg(1).toString();
        break;
    default:
        return bRet(err, translate("lesser", "Null argument(s)", "error"), false);
    }
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool lesserOrEqual(ExecutionStack *stack, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs()))
    {
    case PretexVariant::Int:
        b = stack->obligArg().toInt() <= stack->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = stack->obligArg().toReal() <= stack->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = stack->obligArg().toString() <= stack->obligArg(1).toString();
        break;
    default:
        return bRet(err, translate("lesserOrEqual", "Null argument(s)", "error"), false);
    }
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool booleanGreater(ExecutionStack *stack, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs()))
    {
    case PretexVariant::Int:
        b = stack->obligArg().toInt() > stack->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = stack->obligArg().toReal() > stack->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = stack->obligArg().toString() > stack->obligArg(1).toString();
        break;
    default:
        return bRet(err, translate("booleanGreater", "Null argument(s)", "error"), false);
    }
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool greaterOrEqual(ExecutionStack *stack, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs()))
    {
    case PretexVariant::Int:
        b = stack->obligArg().toInt() >= stack->obligArg(1).toInt();
        break;
    case PretexVariant::Real:
        b = stack->obligArg().toReal() >= stack->obligArg(1).toReal();
        break;
    case PretexVariant::String:
        b = stack->obligArg().toString() >= stack->obligArg(1).toString();
        break;
    default:
        return bRet(err, translate("greaterOrEqual", "Null argument(s)", "error"), false);
    }
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool booleanOr(ExecutionStack *stack, QString *err)
{
    bool b = false;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs(), stack->optArgs()))
    {
    case PretexVariant::Int:
    case PretexVariant::Real:
    {
        b = stack->obligArg().toInt();
        foreach (int n, bRangeD(1, stack->obligArgCount() - 1))
        {
            if (b)
                break;
            b = b || stack->obligArg(n).toInt();
        }
        foreach (int n, bRangeD(1, stack->optArgCount() - 1))
        {
            if (b)
                break;
            b = b || stack->optArg(n).toInt();
        }
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("booleanOr", "Unable to cast strings to boolean", "error"), false);
    default:
        return bRet(err, translate("booleanOr", "Null argument(s)", "error"), false);
    }
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool booleanAnd(ExecutionStack *stack, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs(), stack->optArgs()))
    {
    case PretexVariant::Int:
    case PretexVariant::Real:
    {
        b = stack->obligArg().toInt();
        foreach (int n, bRangeD(1, stack->obligArgCount() - 1))
        {
            if (!b)
                break;
            b = b && stack->obligArg(n).toInt();
        }
        foreach (int n, bRangeD(1, stack->optArgCount() - 1))
        {
            if (!b)
                break;
            b = b && stack->optArg(n).toInt();
        }
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("booleanAnd", "Unable to cast strings to boolean", "error"), false);
    default:
        return bRet(err, translate("booleanAnd", "Null argument(s)", "error"), false);
    }
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool booleanXor(ExecutionStack *stack, QString *err)
{
    bool b = false;
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs(), stack->optArgs()))
    {
    case PretexVariant::Int:
    case PretexVariant::Real:
    {
        b = stack->obligArg().toInt();
        foreach (int n, bRangeD(1, stack->obligArgCount() - 1))
            b = b ^ stack->obligArg(n).toInt();
        foreach (int n, bRangeD(1, stack->optArgCount() - 1))
            b = b ^ stack->optArg(n).toInt();
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("booleanXor", "Unable to cast strings to boolean", "error"), false);
    default:
        return bRet(err, translate("booleanXor", "Null argument(s)", "error"), false);
    }
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

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
    switch (mtype)
    {
    case NotType:
        return "not";
    case EqualType:
        return "equal";
    case NotEqualType:
        return "notEqual";
    case LesserType:
        return "lesser";
    case LesserOrEqualType:
        return "lesserOrEqual";
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
    switch (mtype)
    {
    case EqualType:
    case NotEqualType:
    case LesserType:
    case LesserOrEqualType:
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
    switch (mtype)
    {
    case EqualType:
    case OrType:
    case AndType:
    case XorType:
        return -1;
    case NotType:
    case NotEqualType:
    case LesserType:
    case LesserOrEqualType:
    case GreaterType:
    case GreaterOrEqualType:
        return 0;
    default:
        break;
    }
    return 0;
}

/*============================== Protected methods =========================*/

bool BooleanFunction::execute(ExecutionStack *stack, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case NotType:
        return booleanNot(stack, err);
    case EqualType:
        return equal(stack, err);
    case NotEqualType:
        return notEqual(stack, err);
    case LesserType:
        return lesser(stack, err);
    case LesserOrEqualType:
        return lesserOrEqual(stack, err);
    case GreaterType:
        return booleanGreater(stack, err);
    case GreaterOrEqualType:
        return greaterOrEqual(stack, err);
    case OrType:
        return booleanOr(stack, err);
    case AndType:
        return booleanAnd(stack, err);
    case XorType:
        return booleanXor(stack, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
