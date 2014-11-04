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

#ifndef BOOLEANFUNCTION_H
#define BOOLEANFUNCTION_H

class ExecutionContext;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QCoreApplication>
#include <QList>

/*============================================================================
================================ BooleanFunction =============================
============================================================================*/

class BooleanFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(BooleanFunction)
public:
    enum Type
    {
        AndType,
        EqualType,
        GreaterOrEqualType,
        GreaterType,
        LessOrEqualType,
        LessType,
        NotEqualType,
        NotType,
        OrType,
        XorType
    };
private:
    Type mtype;
public:
    explicit BooleanFunction(Type t);
public:
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
protected:
    bool execute(ExecutionContext *context, QString *err = 0);
private:
    static bool booleanAnd(ExecutionContext *context, QString *err);
    static bool booleanEqual(ExecutionContext *context, QString *err);
    static bool booleanGreater(ExecutionContext *context, QString *err);
    static bool booleanGreaterOrEqual(ExecutionContext *context, QString *err);
    static bool booleanLess(ExecutionContext *context, QString *err);
    static bool booleanLessOrEqual(ExecutionContext *context, QString *err);
    static bool booleanNot(ExecutionContext *context, QString *err);
    static bool booleanNotEqual(ExecutionContext *context, QString *err);
    static bool booleanOr(ExecutionContext *context, QString *err);
    static bool booleanXor(ExecutionContext *context, QString *err);
private:
    Q_DISABLE_COPY(BooleanFunction)
};

#endif // BOOLEANFUNCTION_H
