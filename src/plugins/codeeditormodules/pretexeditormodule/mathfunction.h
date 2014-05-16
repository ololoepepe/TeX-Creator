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

#ifndef MATHFUNCTION_H
#define MATHFUNCTION_H

class ExecutionStack;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QList>
#include <QCoreApplication>

/*============================================================================
================================ MathFunction ================================
============================================================================*/

class MathFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(MathFunction)
public:
    enum Type
    {
        SumType
    };
public:
    explicit MathFunction(Type t);
public:
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
    bool execute(ExecutionStack *stack, const QList<PretexVariant> &obligatoryArguments,
                 const QList<PretexVariant> &optionalArguments, PretexVariant &result, QString *err = 0);
private:
    Type mtype;
};

#endif // MATHFUNCTION_H
