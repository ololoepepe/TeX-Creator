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

#include <BeQtGlobal>

#include <QList>
#include <QString>

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static PretexVariant::Type typeToCastTo(PretexVariant::Type preferredType,
                                        const QList<PretexVariant> &obligatoryArguments,
                                        const QList<PretexVariant> &optionalArguments)
{
    switch (preferredType)
    {
    case PretexVariant::Int:
        foreach (const PretexVariant &v, obligatoryArguments)
            if (v.type() == PretexVariant::String)
                return PretexVariant::String;
        foreach (const PretexVariant &v, optionalArguments)
            if (v.type() == PretexVariant::String)
                return PretexVariant::String;
        foreach (const PretexVariant &v, obligatoryArguments)
            if (v.type() == PretexVariant::Real)
                return PretexVariant::Real;
        foreach (const PretexVariant &v, optionalArguments)
            if (v.type() == PretexVariant::Real)
                return PretexVariant::Real;
        return PretexVariant::Int;
    case PretexVariant::Invalid:
    default:
        return PretexVariant::Invalid;
    }
}

static bool sum(const QList<PretexVariant> &obligatoryArguments, const QList<PretexVariant> &optionalArguments,
                PretexVariant &result, QString *err)
{
    switch (typeToCastTo(PretexVariant::Int, obligatoryArguments, optionalArguments))
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
    case SumType:
        return "sum";
    default:
        break;
    }
    return QString();
}

int MathFunction::obligatoryArgumentCount() const
{
    switch (mtype)
    {
    case SumType:
        return 2;
    default:
        break;
    }
    return 0;
}

int MathFunction::optionalArgumentCount() const
{
    switch (mtype)
    {
    case SumType:
        return -1;
    default:
        break;
    }
    return 0;
}

bool MathFunction::execute(ExecutionStack *, const QList<PretexVariant> &obligatoryArguments,
                           const QList<PretexVariant> &optionalArguments, PretexVariant &result, QString *err)
{
    switch (mtype)
    {
    case SumType:
        return sum(obligatoryArguments, optionalArguments, result, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
