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

#include "executioncontext.h"
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

bool TrigonometricFunction::execute(ExecutionContext *context, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype) {
    case SinType:
        return unarySin(context, err);
    case CosType:
        return unaryCos(context, err);
    case TanType:
        return unaryTan(context, err);
    case CotType:
        return unaryCot(context, err);
    case SecType:
        return unarySec(context, err);
    case CscType:
        return unaryCsc(context, err);
    case AsinType:
        return unaryAsin(context, err);
    case AcosType:
        return unaryAcos(context, err);
    case AtanType:
        return unaryAtan(context, err);
    case AcotType:
        return unaryAcot(context, err);
    case AsecType:
        return unaryAsec(context, err);
    case AcscType:
        return unaryAcsc(context, err);
    case ShType:
        return unarySh(context, err);
    case ChType:
        return unaryCh(context, err);
    case ThType:
        return unaryTh(context, err);
    case CthType:
        return unaryCth(context, err);
    case SechType:
        return unarySech(context, err);
    case CschType:
        return unaryCsch(context, err);
    case ArshType:
        return unaryArsh(context, err);
    case ArchType:
        return unaryArch(context, err);
    case ArthType:
        return unaryArth(context, err);
    case ArcthType:
        return unaryArcth(context, err);
    case ArschType:
        return unaryArsch(context, err);
    case ArcschType:
        return unaryArcsch(context, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}

/*============================== Static private methods ====================*/

bool TrigonometricFunction::unaryAcos(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(std::acos(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(std::acos(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAcot(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(acot(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(acot(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAcsc(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        if (!context->obligArg().toInt())
            return bRet(err, tr("Taking acsc of zero", "error"), false);
        context->setReturnValue(acsc(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(acsc(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArch(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        if (context->obligArg().toInt() < 1)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(arch(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(arch(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArcsch(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(arcsch(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(arcsch(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArcth(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        if (context->obligArg().toInt() == 1)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(arcth(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(arcth(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArsch(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(arsch(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(arsch(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArsh(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(arsh(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(arsh(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryArth(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        if (context->obligArg().toInt() == 1)
            return bRet(err, tr("Invalid argument", "error"), false);
        context->setReturnValue(arth(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(arth(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAsec(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        if (!context->obligArg().toInt())
            return bRet(err, tr("Taking asec of zero", "error"), false);
        context->setReturnValue(asec(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(asec(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAsin(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(std::asin(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(std::asin(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryAtan(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(std::atan(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(std::atan(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCh(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(std::cosh(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(std::cosh(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCos(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(std::cos(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(std::cos(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCot(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(ctan(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(ctan(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCsc(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(1.0 / std::cos(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(1.0 / std::cos(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCsch(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(csch(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(csch(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryCth(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(cth(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(cth(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unarySec(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(sec(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(sec(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unarySech(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(sech(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(sech(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unarySh(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(std::sinh(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(std::sinh(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unarySin(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(std::sin(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(std::sin(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryTan(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(std::tan(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(std::tan(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool TrigonometricFunction::unaryTh(ExecutionContext *context, QString *err)
{
    switch (context->obligArg().type()) {
    case PretexVariant::Int:
        context->setReturnValue(std::tanh(context->obligArg().toInt()));
        break;
    case PretexVariant::Real:
        context->setReturnValue(std::tanh(context->obligArg().toReal()));
        break;
    case PretexVariant::String:
    default:
        return bRet(err, tr("Invalid argument type", "error"), false);
    }
    return bRet(err, QString(), true);
}
