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

#include "pretexstatement.h"
#include "pretexfunction.h"
#include "pretexvariant.h"
#include "pretexbuiltinfunction.h"

#include <QDebug>
#include <QDataStream>
#include <QVariantMap>
#include <QVariant>
#include <QString>

/*============================================================================
================================ PretexStatement =============================
============================================================================*/

/*============================== Public constructors =======================*/

PretexStatement::PretexStatement()
{
    muserFunc = 0;
    mbuiltinFunc = 0;
}

PretexStatement::PretexStatement(PretexBuiltinFunction *f)
{
    muserFunc = 0;
    mbuiltinFunc = f;
}

PretexStatement::PretexStatement(const QString &builtinFunctionName)
{
    muserFunc = 0;
    mbuiltinFunc = PretexBuiltinFunction::functionForName(builtinFunctionName);
}

PretexStatement::PretexStatement(const PretexVariant &value)
{
    muserFunc = 0;
    mbuiltinFunc = 0;
    mvalue = value;
}

PretexStatement::PretexStatement(const PretexFunction &func)
{
    muserFunc = new PretexFunction(func);
    mbuiltinFunc = 0;
}

PretexStatement::PretexStatement(const PretexStatement &other)
{
    clear();
    *this = other;
}

PretexStatement::~PretexStatement()
{
    delete muserFunc;
}

/*============================== Public methods ============================*/

void PretexStatement::clear()
{
    delete muserFunc;
    muserFunc = 0;
    mbuiltinFunc = 0;
    mvalue.clear();
}

PretexFunction *PretexStatement::userFunction() const
{
    return muserFunc;
}

PretexBuiltinFunction *PretexStatement::builtinFunction() const
{
    return mbuiltinFunc;
}

QString PretexStatement::builtinFunctionName() const
{
    return mbuiltinFunc ? mbuiltinFunc->name() : QString();
}

bool PretexStatement::isNull() const
{
    return (muserFunc || mbuiltinFunc) ? false : mvalue.isNull();
}

PretexStatement::Type PretexStatement::type() const
{
    if (muserFunc)
        return UserFunction;
    else if (mbuiltinFunc)
        return BuiltinFunction;
    else
        return Value;
}

PretexVariant PretexStatement::value() const
{
    return mvalue;
}

/*============================== Public operators ==========================*/

bool PretexStatement::operator!=(const PretexStatement &other) const
{
    return !(*this == other);
}

PretexStatement &PretexStatement::operator= (const PretexStatement &other)
{
    clear();
    muserFunc = other.muserFunc ? new PretexFunction(*other.muserFunc) : 0;
    mbuiltinFunc = other.mbuiltinFunc;
    mvalue = other.mvalue;
    return *this;
}

bool PretexStatement::operator== (const PretexStatement &other) const
{
    if ((bool) muserFunc != (bool) other.muserFunc || (bool) mbuiltinFunc != (bool) other.mbuiltinFunc)
        return false;
    if (muserFunc)
        return *muserFunc == *other.muserFunc;
    else if (mbuiltinFunc)
        return mbuiltinFunc == other.mbuiltinFunc;
    else
        return mvalue == other.mvalue;
}

/*============================== Public friend operators ===================*/

QDataStream &operator<< (QDataStream &s, const PretexStatement &st)
{
    QVariantMap m;
    if (st.muserFunc)
        m.insert("user_function", QVariant::fromValue(*st.muserFunc));
    else if (st.mbuiltinFunc)
        m.insert("builtin_function_name", QVariant::fromValue(st.mbuiltinFunc->name()));
    else
        m.insert("value", QVariant::fromValue(st.mvalue));
    s << m;
    return s;
}

QDataStream &operator>> (QDataStream &s, PretexStatement &st)
{
    QVariantMap m;
    s >> m;
    if (m.contains("user_function"))
        st = PretexStatement(m.value("user_function").value<PretexFunction>());
    else if (m.contains("builtin_function_name"))
        st = PretexStatement(m.value("builtin_function_name").toString());
    else
        st = PretexStatement(m.value("value").value<PretexVariant>());
    return s;
}

QDebug operator<< (QDebug dbg, const PretexStatement &st)
{
    //TODO
    if (st.muserFunc)
        dbg.nospace() << "PretexStatement(UserFunction)";
    else if (st.mbuiltinFunc)
        dbg.nospace() << "PretexStatement(BuiltinFunction)";
    else
        dbg.nospace() << "PretexStatement(Value)";
    return dbg.space();
}
