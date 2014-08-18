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

#ifndef GENERALFUNCTION_H
#define GENERALFUNCTION_H

class ExecutionStack;
class Function_TokenData;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QList>
#include <QCoreApplication>

/*============================================================================
================================ GeneralFunction =============================
============================================================================*/

class GeneralFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(GeneralFunction)
public:
    enum Type
    {
        IsEmptyType,
        ToIntegerType,
        ToRealType,
        ToStringType,
        FormatType,
        IfType,
        WaitType,
        ForType,
        WhileType,
        DoWhileType,
        UntilType,
        DoUntilType,
        ReturnType,
        BreakType,
        ContinueType
    };
public:
    explicit GeneralFunction(Type t);
public:
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
    bool execute(ExecutionStack *stack, Function_TokenData *f, QString *err = 0);
    SpecialFlags acceptedFlags() const;
    SpecialFlags flagsPropagateMask() const;
protected:
    bool execute(ExecutionStack *stack, QString *err = 0);
private:
    Type mtype;
private:
    Q_DISABLE_COPY(GeneralFunction)
};

#endif // GENERALFUNCTION_H
