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

static bool add(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                PretexVariant &result, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
    {
    case PretexVariant::Int:
    {
        int i = 0;
        foreach (const PretexVariant &v, obligatoryArguments)
            i += v.toInt();
        foreach (const PretexVariant &v, optionalArguments)
            i += v.toInt();
        result = PretexVariant(i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = 0.0;
        foreach (const PretexVariant &v, obligatoryArguments)
            d += v.toReal();
        foreach (const PretexVariant &v, optionalArguments)
            d += v.toReal();
        result = PretexVariant(d);
        break;
    }
    case PretexVariant::String:
    {
        QString s;
        foreach (const PretexVariant &v, obligatoryArguments)
            s += v.toString();
        foreach (const PretexVariant &v, optionalArguments)
            s += v.toString();
        result = PretexVariant(s);
        break;
    }
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool subtract(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                     PretexVariant &result, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
    {
    case PretexVariant::Int:
    {
        int i = 0;
        foreach (const PretexVariant &v, obligatoryArguments)
            i -= v.toInt();
        foreach (const PretexVariant &v, optionalArguments)
            i -= v.toInt();
        result = PretexVariant(i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = 0.0;
        foreach (const PretexVariant &v, obligatoryArguments)
            d -= v.toReal();
        foreach (const PretexVariant &v, optionalArguments)
            d -= v.toReal();
        result = PretexVariant(d);
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

static bool multiply(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                     PretexVariant &result, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
    {
    case PretexVariant::Int:
    {
        int i = 1;
        foreach (const PretexVariant &v, obligatoryArguments)
            i *= v.toInt();
        foreach (const PretexVariant &v, optionalArguments)
            i *= v.toInt();
        result = PretexVariant(i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = 1.0;
        foreach (const PretexVariant &v, obligatoryArguments)
            d *= v.toReal();
        foreach (const PretexVariant &v, optionalArguments)
            d *= v.toReal();
        result = PretexVariant(d);
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

static bool divide(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                   PretexVariant &result, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
    {
    case PretexVariant::Int:
    {
        int i = obligatoryArguments.first().toInt();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            const PretexVariant &v = obligatoryArguments.at(n);
            int j = v.toInt();
            if (!j)
                return bRet(err, translate("divide", "Division by zero", "error"), false);
            i /= j;
        }
        foreach (const PretexVariant &v, optionalArguments)
        {
            int j = v.toInt();
            if (!j)
                return bRet(err, translate("divide", "Division by zero", "error"), false);
            i /= j;
        }
        result = PretexVariant(i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = obligatoryArguments.first().toReal();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
            d /= obligatoryArguments.at(n).toReal();
        foreach (const PretexVariant &v, optionalArguments)
            d /= v.toReal();
        result = PretexVariant(d);
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

static bool modulo(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                   PretexVariant &result, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
    {
    case PretexVariant::Int:
    {
        int i = obligatoryArguments.first().toInt();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            const PretexVariant &v = obligatoryArguments.at(n);
            int j = v.toInt();
            if (!j)
                return bRet(err, translate("modulo", "Division by zero", "error"), false);
            i %= j;
        }
        foreach (const PretexVariant &v, optionalArguments)
        {
            int j = v.toInt();
            if (!j)
                return bRet(err, translate("modulo", "Division by zero", "error"), false);
            i %= j;
        }
        result = PretexVariant(i);
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

static bool exponentiate(const QList<PretexVariant> &obligatoryArguments, PretexVariant &result, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments))
    {
    case PretexVariant::Int:
    {
        int i1 = obligatoryArguments.first().toInt();
        if (!i1)
            return bRet(err, translate("exponentiate", "Exponentiation of zero", "error"), false);
        result = PretexVariant(std::pow(i1, obligatoryArguments.last().toInt()));
        break;
    }
    case PretexVariant::Real:
        result = PretexVariant(std::pow(obligatoryArguments.first().toReal(), obligatoryArguments.last().toReal()));
        break;
    case PretexVariant::String:
        return bRet(err, translate("exponentiate", "Exponentiation of strings is not allowed", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

static bool log(const QList<PretexVariant> &obligatoryArguments, PretexVariant &result, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments))
    {
    case PretexVariant::Int:
    {
        int i1 = obligatoryArguments.first().toInt();
        if (i1 <= 0 || 1 == i1)
            return bRet(err, translate("log", "Invalid base of logarythm", "error"), false);
        int i2 = obligatoryArguments.last().toInt();
        if (i2 <= 0)
            return bRet(err, translate("log", "Invalid power of logarythm", "error"), false);
        result = PretexVariant(anyLog(i1, i2));
        break;
    }
    case PretexVariant::Real:
    {
        double d1 = obligatoryArguments.first().toReal();
        if (d1 <= 0.0 || 1.0 == d1)
            return bRet(err, translate("log", "Invalid base of logarythm", "error"), false);
        double d2 = obligatoryArguments.last().toReal();
        if (d2 <= 0.0)
            return bRet(err, translate("log", "Invalid power of logarythm", "error"), false);
        result = PretexVariant(anyLog(d1, d2));
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

static bool root(const QList<PretexVariant> &obligatoryArguments, PretexVariant &result, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments))
    {
    case PretexVariant::Int:
    {
        int i1 = obligatoryArguments.first().toInt();
        if (i1 < 0)
            return bRet(err, translate("root", "Negative radicant of root", "error"), false);
        int i2 = obligatoryArguments.last().toInt();
        if (i2 < 0)
            return bRet(err, translate("root", "Negative degree of root", "error"), false);
        result = PretexVariant(anyRoot(i1, i2));
        break;
    }
    case PretexVariant::Real:
    {
        double d1 = obligatoryArguments.first().toReal();
        if (d1 < 0.0)
            return bRet(err, translate("root", "Negative radicant of root", "error"), false);
        double d2 = obligatoryArguments.last().toReal();
        if (d2 < 0.0)
            return bRet(err, translate("root", "Negative degree of root", "error"), false);
        result = PretexVariant(anyRoot(d1, d2));
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

static bool round(const PretexVariant &obligatoryArgument, const PretexVariant &optionalArgument,
                  PretexVariant &result, QString *err)
{
    switch (obligatoryArgument.type())
    {
    case PretexVariant::Int:
        return bRet(err, translate("round", "Integers can not be rounded", "error"), false);
    case PretexVariant::Real:
    {
        bool up = false;
        if (!optionalArgument.isNull())
        {
            if (optionalArgument.type() != PretexVariant::String)
                return bRet(err, translate("round", "Optional argument must be a string", "error"), false);
            QString s = optionalArgument.toString();
            if (!QString::compare(s, "up", Qt::CaseInsensitive) || !QString::compare(s, "u", Qt::CaseInsensitive))
                up = true;
            else if (QString::compare(s, "down", Qt::CaseInsensitive) && QString::compare(s, "d", Qt::CaseInsensitive))
                return bRet(err, translate("round", "Optional argument must be either "
                                           "\"up\", \"u\", \"down\", or \"d\"", "error"), false);
        }
        result = PretexVariant(up ? std::ceil(obligatoryArgument.toReal()) : std::floor(obligatoryArgument.toReal()));
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

static bool abs(const PretexVariant &obligatoryArgument, PretexVariant &result, QString *err)
{
    switch (obligatoryArgument.type())
    {
    case PretexVariant::Int:
    {
        int i = obligatoryArgument.toInt();
        result = PretexVariant((i < 0) ? (-1 * i) : i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = obligatoryArgument.toReal();
        result = PretexVariant((d < 0.0) ? (-1.0 * d) : d);
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

static bool random(const PretexVariant &obligatoryArgument, const PretexVariant &optionalArgument,
                   PretexVariant &result, QString *err)
{
    if (obligatoryArgument.type() == PretexVariant::String
            || (!optionalArgument.isNull() && optionalArgument.type() == PretexVariant::String))
        return bRet(err, translate("random", "Unable to generate random string", "error"), false);
    init_once(bool, b, true)
        qsrand(QDateTime::currentMSecsSinceEpoch());
    Q_UNUSED(b)
    if (obligatoryArgument.type() == PretexVariant::Real || optionalArgument.type() == PretexVariant::Real)
    {
        double min = obligatoryArgument.toReal();
        double max = !optionalArgument.isNull() ? optionalArgument.toReal() : 100400.2;
        if (max <= min)
            return bRet(err, translate("random", "Upper bound is less than or equal to lower bound", "error"), false);
        result = PretexVariant(min + ((double) qrand() / RAND_MAX) * (max - min));
    }
    else
    {
        int min = obligatoryArgument.toInt();
        int max = !optionalArgument.isNull() ? optionalArgument.toInt() : RAND_MAX;
        if (max <= min)
            return bRet(err, translate("random", "Upper bound is less than or equal to lower bound", "error"), false);
        result = PretexVariant(min + (qrand() % (max - min + 1)));
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

bool MathFunction::execute(ExecutionStack *, const QList<PretexVariant> &obligatoryArguments,
                           const QList<PretexVariant> &optionalArguments, PretexVariant &result, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case AddType:
        return add(obligatoryArguments, optionalArguments, result, err);
    case SubtractType:
        return subtract(obligatoryArguments, optionalArguments, result, err);
    case MultiplyType:
        return multiply(obligatoryArguments, optionalArguments, result, err);
    case DivideType:
        return divide(obligatoryArguments, optionalArguments, result, err);
    case ModuloType:
        return modulo(obligatoryArguments, optionalArguments, result, err);
    case ExponentiateType:
        return exponentiate(obligatoryArguments, result, err);
    case LogType:
        return log(obligatoryArguments, result, err);
    case RootType:
        return root(obligatoryArguments, result, err);
    case RoundType:
        return round(Global::firstIfAny(obligatoryArguments), Global::firstIfAny(optionalArguments), result, err);
    case AbsType:
        return abs(Global::firstIfAny(obligatoryArguments), result, err);
    case RandomType:
        return random(Global::firstIfAny(obligatoryArguments), Global::firstIfAny(optionalArguments), result, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
