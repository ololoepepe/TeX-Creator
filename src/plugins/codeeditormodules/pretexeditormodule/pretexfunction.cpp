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

#include "pretexfunction.h"
#include "pretexstatement.h"
#include "executionstack.h"
#include "pretexbuiltinfunction.h"

#include <BeQtGlobal>

#include <QList>
#include <QString>
#include <QDataStream>
#include <QDebug>
#include <QVariantList>

/*============================================================================
================================ PretexFunction ==============================
============================================================================*/

/*============================== Public constructors =======================*/

PretexFunction::PretexFunction()
{
    mbfunc = 0;
    mobligArgCount = 0;
    moptArgCount = 0;
}

PretexFunction::PretexFunction(PretexBuiltinFunction *builtinFunction)
{
    mbfunc = builtinFunction;
    mobligArgCount = 0;
    moptArgCount = 0;
}

PretexFunction::PretexFunction(const QString &builtinFunctionName)
{
    mbfunc = PretexBuiltinFunction::functionForName(builtinFunctionName);
    mobligArgCount = 0;
    moptArgCount = 0;
}

PretexFunction::PretexFunction(const QString &name, int obligatoryArgumentCount, int optionalArgumentCount,
                               const QList<PretexStatement> &body)
{
    mbfunc = 0;
    if (name.isEmpty() || PretexBuiltinFunction::isBuiltinFunction(name) || obligatoryArgumentCount <= 0)
    {
        mobligArgCount = 0;
        moptArgCount = 0;
        return;
    }
    mname = name;
    mobligArgCount = obligatoryArgumentCount;
    moptArgCount = optionalArgumentCount >= 0 ? optionalArgumentCount : -1;
    mbody = body;
}

PretexFunction::PretexFunction(const PretexFunction &other)
{
    *this = other;
}

/*============================== Public methods ============================*/

QList<PretexStatement> PretexFunction::body() const
{
    return mbody;
}

PretexBuiltinFunction *PretexFunction::builtinFunction() const
{
    return mbfunc;
}

void PretexFunction::clear()
{
    mbfunc = 0;
    mname.clear();
    mobligArgCount = 0;
    moptArgCount = 0;
    mbody.clear();
}

bool PretexFunction::execute(ExecutionStack *stack, const QList<PretexVariant> &obligatoryArguments,
                             const QList<PretexVariant> &optionalArguments, PretexVariant &result, QString *err)
{
    if (!isValid())
        return bRet(err, tr("Attempted to execute invalid function", "error"), false);
    if (obligatoryArguments.size() != obligatoryArgumentCount())
        return bRet(err, tr("Obligatory argument count mismatch", "error"), false);
    if (optionalArguments.size() > optionalArgumentCount() && optionalArgumentCount() >= 0)
        return bRet(err, tr("Optional argument count mismatch", "error"), false);
    if (mbfunc)
        return mbfunc->execute(stack, obligatoryArguments, optionalArguments, result, err);
    //TODO
    return false;
}

bool PretexFunction::isEmpty() const
{
    return !isValid() || (!mbfunc && mbody.isEmpty());
}

bool PretexFunction::isValid() const
{
    return mbfunc || (!mname.isEmpty() && mobligArgCount > 0);
}

QString PretexFunction::name() const
{
    return mbfunc ? mbfunc->name() : mname;
}

int PretexFunction::obligatoryArgumentCount()
{
    return mbfunc ? mbfunc->obligatoryArgumentCount() : mobligArgCount;
}

int PretexFunction::optionalArgumentCount()
{
    return mbfunc ? mbfunc->optionalArgumentCount() : moptArgCount;
}

void PretexFunction::setBody(const QList<PretexStatement> &list)
{
    if (mbfunc)
        return;
    mbody = list;
}

PretexFunction::Type PretexFunction::type() const
{
    return mbfunc ? Builtinfunction : UserFunction;
}

/*============================== Public operators ==========================*/

bool PretexFunction::operator!=(const PretexFunction &other) const
{
    return !(*this == other);
}

PretexFunction &PretexFunction::operator= (const PretexFunction &other)
{
    mbfunc = other.mbfunc;
    mname = other.mname;
    mobligArgCount = other.mobligArgCount;
    moptArgCount = other.moptArgCount;
    mbody = other.mbody;
    return *this;
}

bool PretexFunction::operator== (const PretexFunction &other) const
{
    return mbfunc ? (mbfunc == other.mbfunc) : (mname == other.mname && mobligArgCount == other.mobligArgCount
                                                && moptArgCount == other.moptArgCount && mbody == other.mbody);
}

/*============================== Public friend operators ===================*/

QDataStream &operator<< (QDataStream &s, const PretexFunction &f)
{
    QVariantMap m;
    if (f.mbfunc)
    {
        m.insert("builtin_function_name", QVariant::fromValue(f.mbfunc->name()));
    }
    else
    {
        m.insert("name", f.mname);
        m.insert("obligatory_argument_count", f.mobligArgCount);
        m.insert("optional_argument_count", f.moptArgCount);
        QVariantList list;
        foreach (const PretexStatement &st, f.mbody)
            list << QVariant::fromValue(st);
        m.insert("statement_list", list);
    }
    s << m;
    return s;
}

QDataStream &operator>> (QDataStream &s, PretexFunction &f)
{
    QVariantMap m;
    s >> m;
    if (f.mbfunc)
    {
        f = PretexFunction(m.value("builtin_function_name").toString());
    }
    else
    {
        QList<PretexStatement> list;
        foreach (const QVariant &v, m.value("statement_list").toList())
            list << v.value<PretexStatement>();
        f = PretexFunction(m.value("name").toString(), m.value("obligatory_argument_count").toInt(),
                           m.value("optional_argument_count").toInt(), list);
    }
    return s;
}

QDebug operator<< (QDebug dbg, const PretexFunction &f)
{
    //TODO
    if (f.mbfunc)
        dbg.nospace() << "PretexFunction(BuiltinFunction)";
    else
        dbg.nospace() << "PretexFunction(UserFunction)";
    return dbg.space();
}
