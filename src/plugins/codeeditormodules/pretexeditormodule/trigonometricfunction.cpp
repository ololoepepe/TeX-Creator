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

#include "trigonometricfunction.h"

#include "executionstack.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QList>
#include <QString>

#include <cmath>

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
    switch (mtype) {
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
    switch (mtype) {
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

/*============================== Static private methods ====================*/

bool TrigonometricFunction::unaryAcos(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(std::acos(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::acos(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAcot(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(acot(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(acot(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAcsc(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        if (!stack->obligArg().toInt())
            return bRet(err, tr("Taking acsc of zero", "error"), false);
        stack->setReturnValue(acsc(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(acsc(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArch(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        if (stack->obligArg().toInt() < 1)
            return bRet(err, tr("Invalid argument", "error"), false);
        stack->setReturnValue(arch(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arch(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArcsch(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(arcsch(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arcsch(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArcth(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        if (stack->obligArg().toInt() == 1)
            return bRet(err, tr("Invalid argument", "error"), false);
        stack->setReturnValue(arcth(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arcth(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArsch(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(arsch(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arsch(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArsh(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(arsh(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arsh(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArth(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        if (stack->obligArg().toInt() == 1)
            return bRet(err, tr("Invalid argument", "error"), false);
        stack->setReturnValue(arth(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(arth(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAsec(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        if (!stack->obligArg().toInt())
            return bRet(err, tr("Taking asec of zero", "error"), false);
        stack->setReturnValue(asec(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(asec(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAsin(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(std::asin(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::asin(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAtan(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(std::atan(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::atan(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCh(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(std::cosh(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::cosh(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCos(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(std::cos(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::cos(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCot(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        //TODO: check
        stack->setReturnValue(ctan(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(ctan(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCsc(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        //TODO: check
        stack->setReturnValue(1.0 / std::cos(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(1.0 / std::cos(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCsch(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(csch(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(csch(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCth(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(cth(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(cth(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unarySec(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        //TODO: check
        stack->setReturnValue(sec(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(sec(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unarySech(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(sech(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(sech(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unarySh(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(std::sinh(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::sinh(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unarySin(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(std::sin(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::sin(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryTan(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        //TODO: check
        stack->setReturnValue(std::tan(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::tan(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryTh(ExecutionStack *stack, QString *err)
{
    switch (stack->obligArg().type()) {
    case PretexVariant::Int:
        stack->setReturnValue(std::tanh(stack->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        stack->setReturnValue(std::tanh(stack->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}
