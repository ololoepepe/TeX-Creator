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

#include "pretexarray.h"

#include <BeQtGlobal>

#include <QDataStream>
#include <QDebug>
#include <QList>

/*============================================================================
================================ PretexArray =================================
============================================================================*/

/*============================== Public constructors =======================*/

PretexArray::PretexArray(const Dimensions &dimensions)
{
    if (dimensions.isEmpty())
        return;
    int n = 1;
    foreach (int dim, dimensions) {
        if (dim < 1)
            return;
        else
            n *= dim;
    }
    mdim = dimensions;
    foreach (int i, bRangeD(1, n)) {
        Q_UNUSED(i)
        mdata << PretexVariant();
    }
}

PretexArray::PretexArray(const PretexArray &other)
{
    *this = other;
}

/*============================== Public methods ============================*/

bool PretexArray::areIndexesValid(const Indexes &indexes) const
{
    if (!isValid())
        return false;
    if (mdim.size() != indexes.size())
        return false;
    foreach (int i, bRangeD(0, indexes.size() - 1)) {
        if (indexes.at(i) < 0 || indexes.at(i) >= mdim.at(i))
            return false;
    }
    return true;
}

const PretexVariant &PretexArray::at(const Indexes &indexes) const
{
    return mdata.at(index(mdim, indexes));
}

void PretexArray::clear()
{
    foreach (int i, bRangeD(0, mdata.size() - 1))
        mdata[i].clear();
}

int PretexArray::dimension(int indexNo)
{
    if (indexNo < 0 || indexNo >= mdim.size())
        return 0;
    return mdim.at(indexNo);
}

int PretexArray::dimensionCount() const
{
    return mdim.size();
}

PretexArray::Dimensions PretexArray::dimensions() const
{
    return mdim;
}

int PretexArray::elementCount() const
{
    if (!isValid())
        return 0;
    int n = 1;
    foreach (int dim, mdim)
        n *= dim;
    return n;
}

bool PretexArray::isValid() const
{
    return !mdim.isEmpty();
}

PretexVariant PretexArray::value(const Indexes &indexes) const
{
    return mdata.value(index(mdim, indexes));
}

/*============================== Public operators ==========================*/

bool PretexArray::operator!=(const PretexArray &other) const
{
    return !(*this == other);
}

PretexArray &PretexArray::operator= (const PretexArray &other)
{
    mdim = other.mdim;
    mdata = other.mdata;
    return *this;
}

bool PretexArray::operator== (const PretexArray &other) const
{
    return mdim == other.mdim && mdata == other.mdata;
}

PretexVariant &PretexArray::operator[] (const Indexes &indexes)
{
    return mdata[index(mdim, indexes)];
}

const PretexVariant &PretexArray::operator[] (const Indexes &indexes) const
{
    return mdata[index(mdim, indexes)];
}

/*============================== Public friend operators ===================*/

QDataStream &operator<< (QDataStream &s, const PretexArray &a)
{
    s << a.mdim;
    s << a.mdata;
    return s;
}

QDataStream &operator>> (QDataStream &s, PretexArray &a)
{
    PretexArray::Dimensions dim;
    QList<PretexVariant> data;
    s >> dim;
    s >> data;
    PretexArray aa(dim);
    if (!aa.isValid() || data.size() != aa.elementCount())
        return s;
    a = aa;
    return s;
}

QDebug operator<< (QDebug dbg, const PretexArray &a)
{
    if (a.isValid()) {
        QString s = "[" + QString::number(a.mdim.first());
        foreach (int i, bRangeD(1, a.mdim.size() - 1))
            s += "x" + QString::number(a.mdim.at(i));
        s += "]";
        dbg.nospace() << "PretexArray(" << s.toLatin1().constData() << ")";
    } else {
        dbg.nospace() << "PretexArray()";
    }
    return dbg.space();
}

/*============================== Static private methods ====================*/

int PretexArray::index(const Dimensions &dim, const Indexes &indexes)
{
    if (dim.isEmpty() || dim.size() != indexes.size())
        return -1;
    int n = 0;
    foreach (int i, bRangeD(0, dim.size() - 1)) {
        int ind = indexes.at(i);
        if (ind < 0 || ind >= dim.at(i))
            return -1;
        int p = product(dim, i);
        if (p <= 0)
            return -1;
        n += ind * p;
    }
    return n;
}

int PretexArray::product(const Dimensions &dim, int current)
{
    if (current < 0 || current >= dim.size())
        return -1;
    if (dim.size() - 1 == current)
        return 1;
    int n = 1;
    foreach (int i, bRangeD(current + 1, dim.size() - 1))
        n *= dim.at(i);
    return n;
}
