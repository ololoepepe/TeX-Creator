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

#ifndef PRETEXARRAY_H
#define PRETEXARRAY_H

class QDataStream;
class QDebug;

#include "pretexvariant.h"

#include <QList>
#include <QMetaType>

/*============================================================================
================================ PretexArray =================================
============================================================================*/

class PretexArray
{
public:
    typedef QList<int> Dimensions;
    typedef QList<int> Indexes;
private:
    QList<PretexVariant> mdata;
    Dimensions mdim;
public:
    explicit PretexArray(const Dimensions &dimensions = Dimensions());
    PretexArray(const PretexArray &other);
public:
    bool areIndexesValid(const Indexes &indexes) const;
    const PretexVariant &at(const Indexes &indexes) const;
    void clear();
    int dimension(int indexNo);
    int dimensionCount() const;
    Dimensions dimensions() const;
    int elementCount() const;
    bool isValid() const;
    PretexVariant value(const Indexes &indexes) const;
public:
    bool operator!=(const PretexArray &other) const;
    PretexArray &operator= (const PretexArray &other);
    bool operator== (const PretexArray &other) const;
    PretexVariant &operator[] (const Indexes &indexes);
    const PretexVariant &operator[] (const Indexes &indexes) const;
public:
    friend QDataStream &operator<< (QDataStream &s, const PretexArray &a);
    friend QDataStream &operator>> (QDataStream &s, PretexArray &a);
    friend QDebug operator<< (QDebug dbg, const PretexArray &a);
private:
    static int index(const Dimensions &dim, const Indexes &indexes);
    static int product(const Dimensions &dim, int current);
};

Q_DECLARE_METATYPE(PretexArray)

#endif // PRETEXARRAY_H
