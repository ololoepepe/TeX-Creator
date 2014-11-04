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

#include "mathfunction.h"

#include "executioncontext.h"
#include "global.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <BeQtGlobal>

#include <QDateTime>
#include <QList>
#include <QString>

#include <climits>
#include <cmath>

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
    switch (mtype) {
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
    case PowerType:
        return "pow";
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
    switch (mtype) {
    case AddType:
    case SubtractType:
    case MultiplyType:
    case DivideType:
    case ModuloType:
    case PowerType:
    case LogType:
    case RootType:
        return 2;
    case RoundType:
    case AbsType:
    case RandomType:
    case ExpType:
    case SqrtType:
    case LgType:
    case LnType:
    case FactorialType:
    case NegativeType:
        return 1;
    default:
        break;
    }
    return 0;
}

int MathFunction::optionalArgumentCount() const
{
    switch (mtype) {
    case AddType:
    case SubtractType:
    case MultiplyType:
    case DivideType:
    case ModuloType:
        return -1;
    case PowerType:
    case LogType:
    case RootType:
    case AbsType:
    case ExpType:
    case SqrtType:
    case LgType:
    case LnType:
    case FactorialType:
    case NegativeType:
        return 0;
    case RoundType:
    case RandomType:
        return 1;
    default:
        break;
    }
    return 0;
}

/*============================== Protected methods =========================*/

bool MathFunction::execute(ExecutionContext *context, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype) {
    case AddType:
        return add(context, err);
    case SubtractType:
        return subtract(context, err);
    case MultiplyType:
        return multiply(context, err);
    case DivideType:
        return divide(context, err);
    case ModuloType:
        return modulo(context, err);
    case PowerType:
        return power(context, err);
    case ExpType:
        return exp(context, err);
    case LogType:
        return log(context, err);
    case LnType:
        return ln(context, err);
    case LgType:
        return lg(context, err);
    case RootType:
        return root(context, err);
    case SqrtType:
        return sqrt(context, err);
    case RoundType:
        return round(context, err);
    case AbsType:
        return abs(context, err);
    case RandomType:
        return random(context, err);
    case NegativeType:
        return negative(context, err);
    case FactorialType:
        return factorial(context, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}

/*============================== Static private methods ====================*/

bool MathFunction::abs(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int: {
        int i = context->obligArg().toInt();
        context->setReturnValue((i < 0) ? (-1 * i) : i);
        break;
    }
    case PretexVariant::Real: {
        double d = context->obligArg().toReal();
        context->setReturnValue((d < 0.0) ? (-1.0 * d) : d);
        break;
    }
    case PretexVariant::String: {
        return bRet(err, tr("Unable to find absolute value of a string", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::add(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs(), context->optArgs())) {
    case PretexVariant::Int: {
        int i = 0;
        foreach (const PretexVariant &v, context->obligArgs())
            i += v.toInt();
        foreach (const PretexVariant &v, context->optArgs())
            i += v.toInt();
        context->setReturnValue(i);
        break;
    }
    case PretexVariant::Real: {
        double d = 0.0;
        foreach (const PretexVariant &v, context->obligArgs())
            d += v.toReal();
        foreach (const PretexVariant &v, context->optArgs())
            d += v.toReal();
        context->setReturnValue(d);
        break;
    }
    case PretexVariant::String: {
        QString s;
        foreach (const PretexVariant &v, context->obligArgs())
            s += v.toString();
        foreach (const PretexVariant &v, context->optArgs())
            s += v.toString();
        context->setReturnValue(s);
        break;
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::divide(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs(), context->optArgs())) {
    case PretexVariant::Int: {
        int i = context->obligArg().toInt();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1)) {
            const PretexVariant &v = context->obligArg(n);
            int j = v.toInt();
            if (!j)
                return bRet(err, tr("Division by zero", "error"), false);
            i /= j;
        }
        foreach (const PretexVariant &v, context->optArgs()) {
            int j = v.toInt();
            if (!j)
                return bRet(err, tr("Division by zero", "error"), false);
            i /= j;
        }
        context->setReturnValue(i);
        break;
    }
    case PretexVariant::Real: {
        double d = context->obligArg().toReal();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1))
            d /= context->obligArg(n).toReal();
        foreach (const PretexVariant &v, context->optArgs())
            d /= v.toReal();
        context->setReturnValue(d);
        break;
    }
    case PretexVariant::String: {
        return bRet(err, tr("Division of strings is not allowed", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::exp(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int: {
        int i1 = context->obligArg().toInt();
        context->setReturnValue(std::exp(i1));
        break;
    }
    case PretexVariant::Real: {
        context->setReturnValue(std::exp(context->obligArg().toReal()));
        break;
    }
    case PretexVariant::String: {
        return bRet(err, tr("Exponentiation of strings is not allowed", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

int MathFunction::fact(int i)
{
    if (i < 0)
        return -1;
    if (!i || 1 == i)
        return 1;
    return i * fact(i - 1);
}

bool MathFunction::factorial(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int: {
        int i = context->obligArg().toInt();
        if (i < 0)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(fact(i));
        break;
    }
    case PretexVariant::Real: {
        return bRet(err, tr("Real numbers are not allowed when calculating factorial", "error"), false);
    }
    case PretexVariant::String: {
        return bRet(err, tr("Strings are not allowed when calculating factorial", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::lg(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int:
        if (context->optArg().toInt() <= 0)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(std::log10(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        if (context->optArg().toReal() < 0.0)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(std::log10(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
        return bRet(err, tr("Strings are not allowed when calculating log", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

bool MathFunction::ln(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int:
        if (context->optArg().toInt() < 0)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(std::log(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        if (context->optArg().toReal() < 0.0)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(std::log(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
        return bRet(err, tr("Strings are not allowed when calculating ln", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

bool MathFunction::log(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int: {
        int i1 = context->obligArg().toInt();
        if (i1 <= 0 || 1 == i1)
            return bRet(err, tr("Invalid base of logarythm", "error"), false);
        int i2 = context->obligArg(1).toInt();
        if (i2 <= 0)
            return bRet(err, tr("Invalid power of logarythm", "error"), false);
        context->setReturnValue(anyLog(i1, i2));
        break;
    }
    case PretexVariant::Real: {
        double d1 = context->obligArg().toReal();
        if (d1 <= 0.0 || 1.0 == d1)
            return bRet(err, tr("Invalid base of logarythm", "error"), false);
        double d2 = context->obligArg(1).toReal();
        if (d2 <= 0.0)
            return bRet(err, tr("Invalid power of logarythm", "error"), false);
        context->setReturnValue(anyLog(d1, d2));
        break;
    }
    case PretexVariant::String: {
        return bRet(err, tr("Strings are not allowed when calculating root", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::modulo(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs(), context->optArgs())) {
    case PretexVariant::Int: {
        int i = context->obligArg().toInt();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1)) {
            const PretexVariant &v = context->obligArg(n);
            int j = v.toInt();
            if (!j)
                return bRet(err, tr("Division by zero", "error"), false);
            i %= j;
        }
        foreach (const PretexVariant &v, context->optArgs()) {
            int j = v.toInt();
            if (!j)
                return bRet(err, tr("Division by zero", "error"), false);
            i %= j;
        }
        context->setReturnValue(i);
        break;
    }
    case PretexVariant::Real: {
        return bRet(err, tr("Modulo of variables of type \"real\" is not allowed", "error"), false);
    }
    case PretexVariant::String: {
        return bRet(err, tr("Modulo of strings is not allowed", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::multiply(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs(), context->optArgs())) {
    case PretexVariant::Int: {
        int i = 1;
        foreach (const PretexVariant &v, context->obligArgs())
            i *= v.toInt();
        foreach (const PretexVariant &v, context->optArgs())
            i *= v.toInt();
        context->setReturnValue(i);
        break;
    }
    case PretexVariant::Real: {
        double d = 1.0;
        foreach (const PretexVariant &v, context->obligArgs())
            d *= v.toReal();
        foreach (const PretexVariant &v, context->optArgs())
            d *= v.toReal();
        context->setReturnValue(d);
        break;
    }
    case PretexVariant::String: {
        return bRet(err, tr("Multiplication of strings is not allowed", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::negative(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(-1 * context->obligArg().toInt());
        break;
    case PretexVariant::Real:
        context->setReturnValue(-1 * context->obligArg().toReal());
        break;
    case PretexVariant::String:
        return bRet(err, tr("Negation is not defined on strings", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

bool MathFunction::power(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int: {
        int i1 = context->obligArg().toInt();
        if (!i1)
            return bRet(err, tr("Exponentiation of zero", "error"), false);
        context->setReturnValue(std::pow(i1, context->obligArg(1).toInt()));
        break;
    }
    case PretexVariant::Real: {
        context->setReturnValue(std::pow(context->obligArg().toReal(), context->obligArg(1).toReal()));
        break;
    }
    case PretexVariant::String: {
        return bRet(err, tr("Exponentiation of strings is not allowed", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::random(ExecutionContext *context, QString *err)
{
    static const PretexVariant::Type Type = PretexVariant::String;
    if (context->obligArg().type() == Type || (!context->optArg().isNull() && context->optArg().type() == Type))
        return bRet(err, tr("Unable to generate random string", "error"), false);
    init_once(bool, b, true)
        qsrand(QDateTime::currentMSecsSinceEpoch());
    Q_UNUSED(b)
    if (context->obligArg().type() == PretexVariant::Real || context->optArg().type() == PretexVariant::Real) {
        double min = context->obligArg().toReal();
        double max = !context->optArg().isNull() ? context->optArg().toReal() : (double) RAND_MAX;
        if (max <= min)
            return bRet(err, tr("Upper bound is less than or equal to lower bound", "error"), false);
        context->setReturnValue(min + ((double) qrand() / RAND_MAX) * (max - min));
    } else {
        int min = context->obligArg().toInt();
        int max = !context->optArg().isNull() ? context->optArg().toInt() : RAND_MAX;
        if (max <= min)
            return bRet(err, tr("Upper bound is less than or equal to lower bound", "error"), false);
        context->setReturnValue(min + (qrand() % (max - min + 1)));
    }
    return bRet(err, QString(), true);
}

bool MathFunction::root(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int: {
        int i1 = context->obligArg().toInt();
        if (i1 < 0)
            return bRet(err, tr("Negative radicant of root", "error"), false);
        int i2 = context->obligArg(1).toInt();
        if (i2 < 0)
            return bRet(err, tr("Negative degree of root", "error"), false);
        context->setReturnValue(anyRoot(i1, i2));
        break;
    }
    case PretexVariant::Real: {
        double d1 = context->obligArg().toReal();
        if (d1 < 0.0)
            return bRet(err, tr("Negative radicant of root", "error"), false);
        double d2 = context->obligArg(1).toReal();
        if (d2 < 0.0)
            return bRet(err, tr("Negative degree of root", "error"), false);
        context->setReturnValue(anyRoot(d1, d2));
        break;
    }
    case PretexVariant::String: {
        return bRet(err, tr("Strings are not allowed when calculating root", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::round(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int: {
        return bRet(err, tr("Integers can not be rounded", "error"), false);
    }
    case PretexVariant::Real: {
        bool up = false;
        if (!context->optArg().isNull()) {
            if (context->optArg().type() != PretexVariant::String)
                return bRet(err, tr("Optional argument must be a string", "error"), false);
            QString s = context->optArg().toString();
            static const Qt::CaseSensitivity Cs = Qt::CaseInsensitive;
            if (!QString::compare(s, "up", Cs) || !QString::compare(s, "u", Cs)) {
                up = true;
            } else if (QString::compare(s, "down", Cs) && QString::compare(s, "d", Cs)) {
                return bRet(err, tr("Optional argument must be either \"up\", \"u\", \"down\", or \"d\"", "error"),
                            false);
            }
        }
        context->setReturnValue(up ? std::ceil(context->obligArg().toReal()) :
                                     std::floor(context->obligArg().toReal()));
        break;
    }
    case PretexVariant::String: {
        return bRet(err, tr("Strings can not be rounded", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool MathFunction::sqrt(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs())) {
    case PretexVariant::Int:
        if (context->optArg().toInt() < 0)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(std::sqrt(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        if (context->optArg().toReal() < 0.0)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(std::sqrt(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
        return bRet(err, tr("Strings are not allowed when calculating sqrt", "error"), false);
    default:
        //This can never happen
        break;
    }
    return bRet(err, QString(), true);
}

bool MathFunction::subtract(ExecutionContext *context, QString *err)
{
    switch (Global::typeToCastTo(PretexVariant::Int, context->obligArgs(), context->optArgs())) {
    case PretexVariant::Int: {
        int i = context->obligArg().toInt();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1))
            i -= context->obligArg(n).toInt();
        foreach (const PretexVariant &v, context->optArgs())
            i -= v.toInt();
        context->setReturnValue(i);
        break;
    }
    case PretexVariant::Real: {
        double d = context->obligArg().toReal();
        foreach (int n, bRangeD(1, context->obligArgCount() - 1))
            d -= context->obligArg(n).toReal();
        foreach (const PretexVariant &v, context->optArgs())
            d -= v.toReal();
        context->setReturnValue(d);
        break;
    }
    case PretexVariant::String: {
        return bRet(err, tr("Subtraction of strings is not allowed", "error"), false);
    }
    default: {
        //This can never happen
        break;
    }
    }
    return bRet(err, QString(), true);
}
