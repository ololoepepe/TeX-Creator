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

#include "booleanfunction.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "executionstack.h"
#include "global.h"

#include <BeQtGlobal>

#include <QList>
#include <QString>

B_DECLARE_TRANSLATE_FUNCTION

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool equal(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                  PretexVariant &result, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
    {
    case PretexVariant::Int:
    {
        int i = obligatoryArguments.first().toInt();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            b = b && (obligatoryArguments.at(n).toInt() == i);
            if (!b)
                break;
        }
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            if (!b)
                break;
            b = b && (optionalArguments.at(n).toInt() == i);
        }
        break;
    }
    case PretexVariant::Real:
    {
        double d = obligatoryArguments.first().toReal();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            b = b && (obligatoryArguments.at(n).toReal() == d);
            if (!b)
                break;
        }
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            if (!b)
                break;
            b = b && (optionalArguments.at(n).toReal() == d);
        }
        break;
    }
    case PretexVariant::String:
    {
        QString s = obligatoryArguments.first().toString();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            b = b && (obligatoryArguments.at(n).toString() == s);
            if (!b)
                break;
        }
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            if (!b)
                break;
            b = b && (optionalArguments.at(n).toString() == s);
        }
        break;
    }
    default:
        //This can never happen
        break;
    }
    result = PretexVariant(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool notEqual(const QList<PretexVariant> &obligatoryArguments, PretexVariant &result, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments))
    {
    case PretexVariant::Int:
        b = obligatoryArguments.first().toInt() != obligatoryArguments.last().toInt();
        break;
    case PretexVariant::Real:
        b = obligatoryArguments.first().toReal() != obligatoryArguments.last().toReal();
        break;
    case PretexVariant::String:
        b = obligatoryArguments.first().toString() != obligatoryArguments.last().toString();
        break;
    default:
        //This can never happen
        break;
    }
    result = PretexVariant(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool lesser(const QList<PretexVariant> &obligatoryArguments, PretexVariant &result, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments))
    {
    case PretexVariant::Int:
        b = obligatoryArguments.first().toInt() < obligatoryArguments.last().toInt();
        break;
    case PretexVariant::Real:
        b = obligatoryArguments.first().toReal() < obligatoryArguments.last().toReal();
        break;
    case PretexVariant::String:
        b = obligatoryArguments.first().toString() < obligatoryArguments.last().toString();
        break;
    default:
        //This can never happen
        break;
    }
    result = PretexVariant(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool lesserOrEqual(const QList<PretexVariant> &obligatoryArguments, PretexVariant &result, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments))
    {
    case PretexVariant::Int:
        b = obligatoryArguments.first().toInt() <= obligatoryArguments.last().toInt();
        break;
    case PretexVariant::Real:
        b = obligatoryArguments.first().toReal() <= obligatoryArguments.last().toReal();
        break;
    case PretexVariant::String:
        b = obligatoryArguments.first().toString() <= obligatoryArguments.last().toString();
        break;
    default:
        //This can never happen
        break;
    }
    result = PretexVariant(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool booleanGreater(const QList<PretexVariant> &obligatoryArguments, PretexVariant &result, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments))
    {
    case PretexVariant::Int:
        b = obligatoryArguments.first().toInt() > obligatoryArguments.last().toInt();
        break;
    case PretexVariant::Real:
        b = obligatoryArguments.first().toReal() > obligatoryArguments.last().toReal();
        break;
    case PretexVariant::String:
        b = obligatoryArguments.first().toString() > obligatoryArguments.last().toString();
        break;
    default:
        //This can never happen
        break;
    }
    result = PretexVariant(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool greaterOrEqual(const QList<PretexVariant> &obligatoryArguments, PretexVariant &result, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments))
    {
    case PretexVariant::Int:
        b = obligatoryArguments.first().toInt() >= obligatoryArguments.last().toInt();
        break;
    case PretexVariant::Real:
        b = obligatoryArguments.first().toReal() >= obligatoryArguments.last().toReal();
        break;
    case PretexVariant::String:
        b = obligatoryArguments.first().toString() >= obligatoryArguments.last().toString();
        break;
    default:
        //This can never happen
        break;
    }
    result = PretexVariant(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool booleanOr(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                      PretexVariant &result, QString *err)
{
    bool b = false;
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
    {
    case PretexVariant::Int:
    case PretexVariant::Real:
    {
        b = obligatoryArguments.first().toInt();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            if (b)
                break;
            b = b || obligatoryArguments.at(n).toInt();
        }
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            if (b)
                break;
            b = b || optionalArguments.at(n).toInt();
        }
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("booleanOr", "Unable to cast strings to boolean", "error"), false);
    default:
        //This can never happen
        break;
    }
    result = PretexVariant(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool booleanAnd(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                       PretexVariant &result, QString *err)
{
    bool b = true;
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
    {
    case PretexVariant::Int:
    case PretexVariant::Real:
    {
        b = obligatoryArguments.first().toInt();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            if (!b)
                break;
            b = b && obligatoryArguments.at(n).toInt();
        }
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
        {
            if (!b)
                break;
            b = b && optionalArguments.at(n).toInt();
        }
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("booleanAnd", "Unable to cast strings to boolean", "error"), false);
    default:
        //This can never happen
        break;
    }
    result = PretexVariant(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool booleanXor(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                       PretexVariant &result, QString *err)
{
    bool b = false;
    switch (Global::typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
    {
    case PretexVariant::Int:
    case PretexVariant::Real:
    {
        b = obligatoryArguments.first().toInt();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
            b = b ^ obligatoryArguments.at(n).toInt();
        foreach (int n, bRangeD(1, obligatoryArguments.size() - 1))
            b = b ^ optionalArguments.at(n).toInt();
        break;
    }
    case PretexVariant::String:
        return bRet(err, translate("booleanXor", "Unable to cast strings to boolean", "error"), false);
    default:
        //This can never happen
        break;
    }
    result = PretexVariant(b ? 1 : 0);
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
        return -1;
    case NotEqualType:
    case LesserType:
    case LesserOrEqualType:
    case GreaterType:
    case GreaterOrEqualType:
    case XorType:
        return 0;
    default:
        break;
    }
    return 0;
}

bool BooleanFunction::execute(ExecutionStack *, const QList<PretexVariant> &obligatoryArguments,
                           const QList<PretexVariant> &optionalArguments, PretexVariant &result, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case EqualType:
        return equal(obligatoryArguments, optionalArguments, result, err);
    case NotEqualType:
        return notEqual(obligatoryArguments, result, err);
    case LesserType:
        return lesser(obligatoryArguments, result, err);
    case LesserOrEqualType:
        return lesserOrEqual(obligatoryArguments, result, err);
    case GreaterType:
        return booleanGreater(obligatoryArguments, result, err);
    case GreaterOrEqualType:
        return greaterOrEqual(obligatoryArguments, result, err);
    case OrType:
        return booleanOr(obligatoryArguments, optionalArguments, result, err);
    case AndType:
        return booleanAnd(obligatoryArguments, optionalArguments, result, err);
    case XorType:
        return booleanXor(obligatoryArguments, optionalArguments, result, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
