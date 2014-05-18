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

#include "mathfunction.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "executionstack.h"
#include "global.h"

#include <BeQtGlobal>

#include <QList>
#include <QString>
#include <QDateTime>

#include <cmath>
#include <climits>

B_DECLARE_TRANSLATE_FUNCTION

/*============================================================================
================================ Template functions ==========================
============================================================================*/

template<typename T> T anyLog(T base, T t)
{
    return std::log(t) / std::log(base);
}

template<typename T> T anyRoot(T base, T p)
{
    static const T t1 = (T) 1;
    return std::pow(base, t1 / p);
}

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool add(ExecutionStack *stack, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs(), stack->optArgs()))
    {
    case PretexVariant::Int:
    {
        int i = 0;
        foreach (const PretexVariant &v, stack->obligArgs())
            i += v.toInt();
        foreach (const PretexVariant &v, stack->optArgs())
            i += v.toInt();
        stack->setReturnValue(i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = 0.0;
        foreach (const PretexVariant &v, stack->obligArgs())
            d += v.toReal();
        foreach (const PretexVariant &v, stack->optArgs())
            d += v.toReal();
        stack->setReturnValue(d);
        break;
    }
    case PretexVariant::String:
    {
        QString s;
        foreach (const PretexVariant &v, stack->obligArgs())
            s += v.toString();
        foreach (const PretexVariant &v, stack->optArgs())
            s += v.toString();
        stack->setReturnValue(s);
        break;
    }
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool subtract(ExecutionStack *stack, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs(), stack->optArgs()))
    {
    case PretexVariant::Int:
    {
        int i = 0;
        foreach (const PretexVariant &v, stack->obligArgs())
            i -= v.toInt();
        foreach (const PretexVariant &v, stack->optArgs())
            i -= v.toInt();
        stack->setReturnValue(i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = 0.0;
        foreach (const PretexVariant &v, stack->obligArgs())
            d -= v.toReal();
        foreach (const PretexVariant &v, stack->optArgs())
            d -= v.toReal();
        stack->setReturnValue(d);
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("subtract", "Subtraction of strings is not allowed", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool multiply(ExecutionStack *stack, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs(), stack->optArgs()))
    {
    case PretexVariant::Int:
    {
        int i = 1;
        foreach (const PretexVariant &v, stack->obligArgs())
            i *= v.toInt();
        foreach (const PretexVariant &v, stack->optArgs())
            i *= v.toInt();
        stack->setReturnValue(i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = 1.0;
        foreach (const PretexVariant &v, stack->obligArgs())
            d *= v.toReal();
        foreach (const PretexVariant &v, stack->optArgs())
            d *= v.toReal();
        stack->setReturnValue(d);
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("multiply", "Multiplication of strings is not allowed", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool divide(ExecutionStack *stack, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs(), stack->optArgs()))
    {
    case PretexVariant::Int:
    {
        int i = stack->optArg().toInt();
        foreach (int n, bRangeD(1, stack->obligArgCount() - 1))
        {
            const PretexVariant &v = stack->obligArg(n);
            int j = v.toInt();
            if (!j)
                return bRet(err, translate("divide", "Division by zero", "error"), false);
            i /= j;
        }
        foreach (const PretexVariant &v, stack->optArgs())
        {
            int j = v.toInt();
            if (!j)
                return bRet(err, translate("divide", "Division by zero", "error"), false);
            i /= j;
        }
        stack->setReturnValue(i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = stack->optArg().toReal();
        foreach (int n, bRangeD(1, stack->obligArgCount() - 1))
            d /= stack->obligArg(n).toReal();
        foreach (const PretexVariant &v, stack->optArgs())
            d /= v.toReal();
        stack->setReturnValue(d);
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("divide", "Division of strings is not allowed", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool modulo(ExecutionStack *stack, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs(), stack->optArgs()))
    {
    case PretexVariant::Int:
    {
        int i = stack->optArg().toInt();
        foreach (int n, bRangeD(1, stack->obligArgCount() - 1))
        {
            const PretexVariant &v = stack->obligArg(n);
            int j = v.toInt();
            if (!j)
                return bRet(err, translate("modulo", "Division by zero", "error"), false);
            i %= j;
        }
        foreach (const PretexVariant &v, stack->optArgs())
        {
            int j = v.toInt();
            if (!j)
                return bRet(err, translate("modulo", "Division by zero", "error"), false);
            i %= j;
        }
        stack->setReturnValue(i);
        break;
    }
    case PretexVariant::Real:
        return bRet(err, translate("modulo", "Modulo of variables of type \"real\" is not allowed", "error"), false);
    case PretexVariant::String:
        return bRet(err, translate("modulo", "Modulo of strings is not allowed", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool exponentiate(ExecutionStack *stack, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs()))
    {
    case PretexVariant::Int:
    {
        int i1 = stack->optArg().toInt();
        if (!i1)
            return bRet(err, translate("exponentiate", "Exponentiation of zero", "error"), false);
        stack->setReturnValue(std::pow(i1, stack->optArg(1).toInt()));
        break;
    }
    case PretexVariant::Real:
        stack->setReturnValue(std::pow(stack->optArg().toReal(), stack->optArg(1).toReal()));
        break;
    case PretexVariant::String:
        return bRet(err, translate("exponentiate", "Exponentiation of strings is not allowed", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool log(ExecutionStack *stack, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs()))
    {
    case PretexVariant::Int:
    {
        int i1 = stack->optArg().toInt();
        if (i1 <= 0 || 1 == i1)
            return bRet(err, translate("log", "Invalid base of logarythm", "error"), false);
        int i2 = stack->optArg(1).toInt();
        if (i2 <= 0)
            return bRet(err, translate("log", "Invalid power of logarythm", "error"), false);
        stack->setReturnValue(anyLog(i1, i2));
        break;
    }
    case PretexVariant::Real:
    {
        double d1 = stack->optArg().toReal();
        if (d1 <= 0.0 || 1.0 == d1)
            return bRet(err, translate("log", "Invalid base of logarythm", "error"), false);
        double d2 = stack->optArg(1).toReal();
        if (d2 <= 0.0)
            return bRet(err, translate("log", "Invalid power of logarythm", "error"), false);
        stack->setReturnValue(anyLog(d1, d2));
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("root", "Strings are not allowed when calculating root", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool root(ExecutionStack *stack, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, stack->obligArgs()))
    {
    case PretexVariant::Int:
    {
        int i1 = stack->optArg().toInt();
        if (i1 < 0)
            return bRet(err, translate("root", "Negative radicant of root", "error"), false);
        int i2 = stack->optArg(1).toInt();
        if (i2 < 0)
            return bRet(err, translate("root", "Negative degree of root", "error"), false);
        stack->setReturnValue(anyRoot(i1, i2));
        break;
    }
    case PretexVariant::Real:
    {
        double d1 = stack->optArg().toReal();
        if (d1 < 0.0)
            return bRet(err, translate("root", "Negative radicant of root", "error"), false);
        double d2 = stack->optArg(1).toReal();
        if (d2 < 0.0)
            return bRet(err, translate("root", "Negative degree of root", "error"), false);
        stack->setReturnValue(anyRoot(d1, d2));
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("root", "Strings are not allowed when calculating root", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool round(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        return bRet(err, translate("round", "Integers can not be rounded", "error"), false);
    case PretexVariant::Real:
    {
        bool up = false;
        if (!stack->optArg().isNull())
        {
            if (stack->optArg().type() != PretexVariant::String)
                return bRet(err, translate("round", "Optional argument must be a string", "error"), false);
            QString s = stack->optArg().toString();
            if (!QString::compare(s, "up", Qt::CaseInsensitive) || !QString::compare(s, "u", Qt::CaseInsensitive))
                up = true;
            else if (QString::compare(s, "down", Qt::CaseInsensitive) && QString::compare(s, "d", Qt::CaseInsensitive))
                return bRet(err, translate("round", "Optional argument must be either "
                                           "\"up\", \"u\", \"down\", or \"d\"", "error"), false);
        }
        stack->setReturnValue(up ? std::ceil(stack->obligArg().toReal()) : std::floor(stack->obligArg().toReal()));
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("round", "Strings can not be rounded", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool abs(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
    {
        int i = stack->obligArg().toInt();
        stack->setReturnValue((i < 0) ? (-1 * i) : i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = stack->obligArg().toReal();
        stack->setReturnValue((d < 0.0) ? (-1.0 * d) : d);
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("abs", "Unable to find absolute value of a string", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool random(ExecutionStack *stack, QString *err)
{
    if (stack->obligArg().type() == PretexVariant::String
            || (!stack->optArg().isNull() && stack->optArg().type() == PretexVariant::String))
        return bRet(err, translate("random", "Unable to generate random string", "error"), false);
    init_once(bool, b, true)
        qsrand(QDateTime::currentMSecsSinceEpoch());
    Q_UNUSED(b)
    if (stack->obligArg().type() == PretexVariant::Real || stack->optArg().type() == PretexVariant::Real)
    {
        double min = stack->obligArg().toReal();
        double max = !stack->optArg().isNull() ? stack->optArg().toReal() : 100400.2;
        if (max <= min)
            return bRet(err, translate("random", "Upper bound is less than or equal to lower bound", "error"), false);
        stack->setReturnValue(min + ((double) qrand() / RAND_MAX) * (max - min));
    }
    else
    {
        int min = stack->obligArg().toInt();
        int max = !stack->optArg().isNull() ? stack->optArg().toInt() : RAND_MAX;
        if (max <= min)
            return bRet(err, translate("random", "Upper bound is less than or equal to lower bound", "error"), false);
        stack->setReturnValue(min + (qrand() % (max - min + 1)));
    }
    return bRet(err, QString(), true);
}

/*============================================================================
================================ MathFunction ================================
============================================================================*/

/*============================== Public constructors =======================*/

MathFunction::MathFunction(Type t)
{
    mtype = t;
}

/*============================== Public methods ============================*/

QString MathFunction::name() const
{
    switch (mtype)
    {
    case AddType:
        return "add";
    case SubtractType:
        return "subtract";
    case MultiplyType:
        return "multiply";
    case DivideType:
        return "divide";
    case ModuloType:
        return "modulo";
    case ExponentiateType:
        return "exponentiate";
    case LogType:
        return "log";
    case RootType:
        return "root";
    case RoundType:
        return "round";
    case AbsType:
        return "abs";
    case RandomType:
        return "random";
    default:
        break;
    }
    return QString();
}

int MathFunction::obligatoryArgumentCount() const
{
    switch (mtype)
    {
    case AddType:
    case SubtractType:
    case MultiplyType:
    case DivideType:
    case ModuloType:
    case ExponentiateType:
    case LogType:
    case RootType:
        return 2;
    case RoundType:
    case AbsType:
    case RandomType:
        return 1;
    default:
        break;
    }
    return 0;
}

int MathFunction::optionalArgumentCount() const
{
    switch (mtype)
    {
    case AddType:
    case SubtractType:
    case MultiplyType:
    case DivideType:
    case ModuloType:
        return -1;
    case ExponentiateType:
    case LogType:
    case RootType:
    case AbsType:
        return 0;
    case RoundType:
    case RandomType:
        return 1;
    default:
        break;
    }
    return 0;
}

bool MathFunction::execute(ExecutionStack *stack, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case AddType:
        return add(stack, err);
    case SubtractType:
        return subtract(stack, err);
    case MultiplyType:
        return multiply(stack, err);
    case DivideType:
        return divide(stack, err);
    case ModuloType:
        return modulo(stack, err);
    case ExponentiateType:
        return exponentiate(stack, err);
    case LogType:
        return log(stack, err);
    case RootType:
        return root(stack, err);
    case RoundType:
        return round(stack, err);
    case AbsType:
        return abs(stack, err);
    case RandomType:
        return random(stack, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
