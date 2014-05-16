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

#ifndef EXECUTIONSTACK_H
#define EXECUTIONSTACK_H

class QByteArray;

#include "pretexvariant.h"
#include "pretexarray.h"
#include "pretexfunction.h"
#include "pretexstatement.h"

#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>

/*============================================================================
================================ ExecutionStack ==============================
============================================================================*/

class ExecutionStack
{
public:
    enum NameType
    {
        UnknownName = 0,
        VariableName,
        ArrayName,
        UserFunctionName,
        BuiltinFunctionName
    };
public:
    explicit ExecutionStack(ExecutionStack *parent = 0);
public:
    bool declareVar(bool global, const QString &name, const PretexVariant &value, QString *err = 0);
    bool declareArray(bool global, const QString &name, const PretexArray::Dimensions &dimensions, QString *err = 0);
    bool declareFunc(bool global, const QString &name, int obligatoryArgumentCount, int optionalAgrumentCount,
                     const QList<PretexStatement> &body, QString *err = 0);
    bool isNameOccupied(const QString &name, NameType *t = 0) const;
    ExecutionStack *parent() const;
    bool setVar(const QString &name, const PretexVariant &value, QString *err = 0);
    bool setArrayElement(const QString &name, const PretexArray::Indexes &indexes, const PretexVariant &value,
                         QString *err = 0);
    bool setFunc(const QString &name, const QList<PretexStatement> &body, QString *err = 0);
    bool undeclare(const QString &name, QString *err = 0);
    //
    void clear();
    QByteArray save() const;
    void restore(const QByteArray &data);
    //
    //
    bool define(const QString &id, const QString &value, bool global = false);
    bool defineF(const QString &id, const QString &value, bool global = true);
    bool undefine(const QString &id);
    bool set(const QString &id, const QString &value);
    bool setF(const QString &id, const QString &value);
    bool get(const QString &id, QString &value) const;
    bool getF(const QString &id, QString &value) const;
    bool isDefined(const QString &id) const;
private:
    ExecutionStack *mparent;
    QMap<QString, PretexVariant> mvars;
    QMap<QString, PretexArray> marrays;
    QMap<QString, PretexFunction> mfuncs;
    //
    //
    QMap<QString, QString> mmap;
    QMap<QString, QString> mmapF;
};

#endif // EXECUTIONSTACK_H
