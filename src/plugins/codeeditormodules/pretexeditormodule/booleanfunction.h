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

#ifndef BOOLEANFUNCTION_H
#define BOOLEANFUNCTION_H

class ExecutionStack;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QList>
#include <QCoreApplication>

/*============================================================================
================================ BooleanFunction =============================
============================================================================*/

class BooleanFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(BooleanFunction)
public:
    enum Type
    {
        EqualType,
        NotEqualType,
        LesserType,
        LesserOrEqualType,
        GreaterType,
        GreaterOrEqualType,
        OrType,
        AndType,
        XorType
    };
public:
    explicit BooleanFunction(Type t);
public:
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
    bool execute(ExecutionStack *stack, QString *err = 0);
private:
    Type mtype;
private:
    Q_DISABLE_COPY(BooleanFunction)
};

#endif // BOOLEANFUNCTION_H
