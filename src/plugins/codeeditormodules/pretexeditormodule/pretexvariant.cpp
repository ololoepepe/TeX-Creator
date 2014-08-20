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

#include "pretexvariant.h"

#include <QDataStream>
#include <QDebug>
#include <QString>
#include <QVariant>

/*============================================================================
================================ PretexVariant ===============================
============================================================================*/

/*============================== Public constructors =======================*/

PretexVariant::PretexVariant()
{
    //
}

PretexVariant::PretexVariant(Type t)
{
    mvariant = QVariant(t);
}

PretexVariant::PretexVariant(const QString &s)
{
    mvariant = QVariant(s);
}

PretexVariant::PretexVariant(int i)
{
    mvariant = QVariant(i);
}

PretexVariant::PretexVariant(double d)
{
    mvariant = QVariant(d);
}

PretexVariant::PretexVariant(const PretexVariant &other)
{
    *this = other;
}

/*============================== Public methods ============================*/

bool PretexVariant::canConvert(Type t) const
{
    return mvariant.canConvert(static_cast<QVariant::Type>(t));
}

void PretexVariant::clear()
{
    return mvariant.clear();
}

bool PretexVariant::convert(Type t)
{
    return mvariant.convert(static_cast<QVariant::Type>(t));
}

bool PretexVariant::isNull() const
{
    return mvariant.isNull();
}

bool PretexVariant::isValid() const
{
    return mvariant.isValid();
}

int PretexVariant::toInt(bool *ok) const
{
    return mvariant.toInt(ok);
}

double PretexVariant::toReal(bool *ok) const
{
    return mvariant.toDouble(ok);
}

QString PretexVariant::toString() const
{
    return mvariant.toString();
}

PretexVariant::Type PretexVariant::type() const
{
    return static_cast<Type>(mvariant.type());
}

const char *PretexVariant::typeName() const
{
    return mvariant.typeName();
}

/*============================== Public operators ==========================*/

bool PretexVariant::operator!=(const PretexVariant &other) const
{
    return !(*this == other);
}

PretexVariant &PretexVariant::operator= (const PretexVariant &other)
{
    mvariant = other.mvariant;
    return *this;
}

bool PretexVariant::operator== (const PretexVariant &other) const
{
    return mvariant == other.mvariant;
}

/*============================== Public friend operators ===================*/

QDataStream &operator<< (QDataStream &s, const PretexVariant &v)
{
    s << v.mvariant;
    return s;
}

QDataStream &operator>> (QDataStream &s, PretexVariant &v)
{
    QVariant vv;
    s >> vv;
    switch (vv.type()) {
    case QVariant::String:
    case QVariant::Int:
    case QVariant::Double:
        v.mvariant = vv;
        break;
    default:
        v.mvariant = QVariant();
        break;
    }
    return s;
}

QDebug operator<< (QDebug dbg, const PretexVariant &v)
{
    switch (v.type()) {
    case PretexVariant::String:
        dbg.nospace() << "PretexVariant(" << v.typeName() << ", " << v.toString() << ")";
        break;
    case PretexVariant::Int:
        dbg.nospace() << "PretexVariant(" << v.typeName() << ", " << v.toInt() << ")";
        break;
    case PretexVariant::Real:
        dbg.nospace() << "PretexVariant(" << v.typeName() << ", " << v.toReal() << ")";
        break;
    default:
        dbg.nospace() << "PretexVariant()";
        break;
    }
    return dbg.space();
}
