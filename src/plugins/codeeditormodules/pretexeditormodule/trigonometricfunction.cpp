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

#include "trigonometricfunction.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "executionstack.h"
#include "global.h"

#include <BeQtGlobal>

#include <QList>
#include <QString>

#include <QDebug>

#include <cmath>

B_DECLARE_TRANSLATE_FUNCTION

/*============================================================================
================================ Template functions ==========================
============================================================================*/

template<typename T> double ctan(T t)
{
    return 1.0 / std::tan(t);
}

template<typename T> double sec(T t)
{
    return 1.0 / std::sin(t);
}

template<typename T> double csc(T t)
{
    return 1.0 / std::cos(t);
}

template<typename T> double acot(T t)
{
    if (t >= 0.0)
        return std::asin(1.0 / std::sqrt(1.0 + std::pow(t, 2.0)));
    else
        return std::atan(1.0) * 4.0 - std::asin(1.0 / std::sqrt(1.0 + std::pow(t, 2.0)));
}

template<typename T> double asec(T t)
{
    return std::acos(1.0 / t);
}

template<typename T> double acsc(T t)
{
    return std::asin(1.0 / t);
}

template<typename T> double cth(T t)
{
    return 1.0 / std::tanh(t);
}

template<typename T> double sech(T t)
{
    return 1.0 / std::sinh(t);
}

template<typename T> double csch(T t)
{
    return 1.0 / std::cosh(t);
}

template<typename T> double arsh(T t)
{
    return std::log(t + std::sqrt(std::pow(t, 2.0) + 1.0));
}

template<typename T> double arch(T t)
{
    return std::log(t + std::sqrt(std::pow(t, 2.0) - 1.0));
}

template<typename T> double arth(T t)
{
    return 0.5 * std::log((1.0 + t) / (1.0 - t));
}

template<typename T> double arcth(T t)
{
    return 0.5 * std::log((t + 1.0) / (t - 1.0));
}

template<typename T> double arsch(T t)
{
    return 1.0 / std::sinh(t);
}

template<typename T> double arcsch(T t)
{
    return 1.0 / std::cosh(t);
}

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool unarySin(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(std::sin(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::sin(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unarySin", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryCos(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(std::cos(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::cos(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryCos", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryTan(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        //TODO: check
        stack->setReturnValue(std::tan(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::tan(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryTan", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryCot(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        //TODO: check
        stack->setReturnValue(ctan(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(ctan(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryCot", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unarySec(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        //TODO: check
        stack->setReturnValue(sec(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(sec(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unarySec", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryCsc(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        //TODO: check
        stack->setReturnValue(1.0 / std::cos(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(1.0 / std::cos(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryCsc", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryAsin(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(std::asin(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::asin(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryAsin", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryAcos(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(std::acos(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::acos(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryAcos", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryAtan(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(std::atan(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::atan(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryAtan", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryAcot(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(acot(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(acot(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryAcot", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryAsec(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        if (!stack->obligArg().toInt())
            return bRet(err, translate("unaryAsec", "Taking asec of zero", "error"), false);
        stack->setReturnValue(asec(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(asec(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryAsec", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryAcsc(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        if (!stack->obligArg().toInt())
            return bRet(err, translate("unaryAcsc", "Taking acsc of zero", "error"), false);
        stack->setReturnValue(acsc(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(acsc(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryAcsc", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unarySh(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(std::sinh(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::sinh(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unarySh", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryCh(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(std::cosh(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::cosh(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryCh", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryTh(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(std::tanh(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::tanh(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryTh", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryCth(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(cth(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(cth(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryCth", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unarySech(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(sech(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(sech(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unarySech", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryCsch(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(csch(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(csch(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryCsch", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryArsh(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(arsh(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arsh(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryArsh", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryArch(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        if (stack->obligArg().toInt() < 1)
            return bRet(err, translate("unaryArch", "Invalid argument", "error"), false);
        stack->setReturnValue(arch(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arch(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryArch", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryArth(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        if (stack->obligArg().toInt() == 1)
            return bRet(err, translate("unaryArth", "Invalid argument", "error"), false);
        stack->setReturnValue(arth(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arth(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryArth", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryArcth(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        if (stack->obligArg().toInt() == 1)
            return bRet(err, translate("unaryArcth", "Invalid argument", "error"), false);
        stack->setReturnValue(arcth(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arcth(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryArcth", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryArsch(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(arsch(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arsch(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryArsch", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool unaryArcsch(ExecutionStack *stack, QString *err = 0)
{
    switch (stack->obligArg().type())
    {
    case PretexVariant::Int:
        stack->setReturnValue(arcsch(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arcsch(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, translate("unaryArcsch", "Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

/*============================================================================
================================ TrigonometricFunction =======================
============================================================================*/

/*============================== Public constructors =======================*/

TrigonometricFunction::TrigonometricFunction(Type t)
{
    mtype = t;
}

/*============================== Public methods ============================*/

QString TrigonometricFunction::name() const
{
    switch (mtype)
    {
    case SinType:
        return "sin";
    case CosType:
        return "cos";
    case TanType:
        return "tan";
    case CotType:
        return "cot";
    case SecType:
        return "sec";
    case CscType:
        return "csc";
    case AsinType:
        return "asin";
    case AcosType:
        return "acot";
    case AtanType:
        return "atan";
    case AcotType:
        return "acot";
    case AsecType:
        return "asec";
    case AcscType:
        return "ascs";
    case ShType:
        return "sh";
    case ChType:
        return "ch";
    case ThType:
        return "th";
    case CthType:
        return "cth";
    case SechType:
        return "sech";
    case CschType:
        return "csch";
    case ArshType:
        return "arsh";
    case ArchType:
        return "arch";
    case ArthType:
        return "arth";
    case ArcthType:
        return "arcth";
    case ArschType:
        return "arsch";
    case ArcschType:
        return "arcsch";
    default:
        break;
    }
    return QString();
}

int TrigonometricFunction::obligatoryArgumentCount() const
{
    return 1;
}

int TrigonometricFunction::optionalArgumentCount() const
{
    return 0;
}

/*============================== Protected methods =========================*/

bool TrigonometricFunction::execute(ExecutionStack *stack, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case SinType:
        return unarySin(stack, err);
    case CosType:
        return unaryCos(stack, err);
    case TanType:
        return unaryTan(stack, err);
    case CotType:
        return unaryCot(stack, err);
    case SecType:
        return unarySec(stack, err);
    case CscType:
        return unaryCsc(stack, err);
    case AsinType:
        return unaryAsin(stack, err);
    case AcosType:
        return unaryAcos(stack, err);
    case AtanType:
        return unaryAtan(stack, err);
    case AcotType:
        return unaryAcot(stack, err);
    case AsecType:
        return unaryAsec(stack, err);
    case AcscType:
        return unaryAcsc(stack, err);
    case ShType:
        return unarySh(stack, err);
    case ChType:
        return unaryCh(stack, err);
    case ThType:
        return unaryTh(stack, err);
    case CthType:
        return unaryCth(stack, err);
    case SechType:
        return unarySech(stack, err);
    case CschType:
        return unaryCsch(stack, err);
    case ArshType:
        return unaryArsh(stack, err);
    case ArchType:
        return unaryArch(stack, err);
    case ArthType:
        return unaryArth(stack, err);
    case ArcthType:
        return unaryArcth(stack, err);
    case ArschType:
        return unaryArsch(stack, err);
    case ArcschType:
        return unaryArcsch(stack, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
