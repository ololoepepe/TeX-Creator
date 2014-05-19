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

#ifndef PRETEXSTATEMENT_H
#define PRETEXSTATEMENT_H

class PretexFunction;
class PretexBuiltinFunction;

class QDataStream;
class QDebug;
class QString;

#include "pretexvariant.h"

#include <QMetaType>

/*============================================================================
================================ PretexStatement =============================
============================================================================*/

class PretexStatement
{
public:
    enum Type
    {
        Value,
        BuiltinFunction,
        UserFunction,
        ArgumentNo
    };
public:
    explicit PretexStatement();
    explicit PretexStatement(PretexBuiltinFunction *f);
    explicit PretexStatement(const QString &builtinFunctionName);
    explicit PretexStatement(const PretexVariant &value);
    explicit PretexStatement(const PretexFunction &func);
    PretexStatement(const PretexStatement &other);
    ~PretexStatement();
public:
    void clear();
    PretexFunction *userFunction() const;
    PretexBuiltinFunction *builtinFunction() const;
    QString builtinFunctionName() const;
    bool isNull() const;
    Type type() const;
    PretexVariant value() const;
public:
    bool operator!=(const PretexStatement &other) const;
    PretexStatement &operator= (const PretexStatement &other);
    bool operator== (const PretexStatement &other) const;
public:
    friend QDataStream &operator<< (QDataStream &s, const PretexStatement &st);
    friend QDataStream &operator>> (QDataStream &s, PretexStatement &st);
    friend QDebug operator<< (QDebug dbg, const PretexStatement &st);
private:
    PretexVariant mvalue;
    PretexBuiltinFunction *mbuiltinFunc;
    PretexFunction *muserFunc;
};

Q_DECLARE_METATYPE(PretexStatement)

#endif // PRETEXSTATEMENT_H
