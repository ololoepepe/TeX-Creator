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
    mfunc = 0;
}

PretexStatement::PretexStatement(const PretexVariant &value)
{
    mfunc = 0;
    mvalue = value;
}

PretexStatement::PretexStatement(const PretexFunction &func)
{
    mfunc = new PretexFunction(func);
}


PretexStatement::PretexStatement(const PretexStatement &other)
{
    mfunc = 0;
    *this = other;
}

PretexStatement::~PretexStatement()
{
    delete mfunc;
}

/*============================== Public methods ============================*/

void PretexStatement::clear()
{
    delete mfunc;
    mfunc = 0;
    mvalue.clear();
}

PretexFunction *PretexStatement::function() const
{
    return mfunc;
}

bool PretexStatement::isNull() const
{
    return mfunc ? false : mvalue.isNull();
}

PretexStatement::Type PretexStatement::type() const
{
    return mfunc ? Function : Value;
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
    delete mfunc;
    mfunc = other.mfunc ? new PretexFunction(*other.mfunc) : 0;
    mvalue = other.mvalue;
    return *this;
}

bool PretexStatement::operator== (const PretexStatement &other) const
{
    return mfunc ? (other.mfunc && *mfunc == *other.mfunc) : (mvalue == other.mvalue);
}

/*============================== Public friend operators ===================*/

QDataStream &operator<< (QDataStream &s, const PretexStatement &st)
{
    QVariantMap m;
    if (st.mfunc)
        m.insert("function", QVariant::fromValue(*st.mfunc));
    else
        m.insert("value", QVariant::fromValue(st.mvalue));
    s << m;
    return s;
}

QDataStream &operator>> (QDataStream &s, PretexStatement &st)
{
    QVariantMap m;
    s >> m;
    if (m.contains("function"))
        st = PretexStatement(m.value("function").value<PretexFunction>());
    else
        st = PretexStatement(m.value("value").value<PretexVariant>());
    return s;
}

QDebug operator<< (QDebug dbg, const PretexStatement &st)
{
    //TODO
    if (st.mfunc)
        dbg.nospace() << "PretexStatement(Function)";
    else
        dbg.nospace() << "PretexStatement(Value)";
    return dbg.space();
}
