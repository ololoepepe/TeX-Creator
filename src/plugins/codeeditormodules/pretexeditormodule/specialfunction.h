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

#ifndef SPECIALFUNCTION_H
#define SPECIALFUNCTION_H

class ExecutionStack;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QCoreApplication>
#include <QList>

/*============================================================================
================================ SpecialFunction =============================
============================================================================*/

class SpecialFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(SpecialFunction)
public:
    enum Type
    {
        DeleteType,
        IsDefinedType,
        NewArrayType,
        NewFuncType,
        NewGlobalArrayType,
        NewGlobalFuncType,
        NewGlobalVarType,
        NewLocalArrayType,
        NewLocalFuncType,
        NewLocalVarType,
        NewVarType,
        RenewFuncType,
        SetType,
        TryDeleteType,
        TryNewArrayType,
        TryNewFuncType,
        TryNewGlobalArrayType,
        TryNewGlobalFuncType,
        TryNewGlobalVarType,
        TryNewLocalArrayType,
        TryNewLocalFuncType,
        TryNewLocalVarType,
        TryNewVarType,
        TryRenewFuncType,
        TrySetType
    };
private:
    Type mtype;
public:
    explicit SpecialFunction(Type t);
public:
    bool execute(ExecutionStack *stack, Function_TokenData *f, QString *err = 0);
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
protected:
    bool execute(ExecutionStack *stack, QString *err = 0);
private:
    static bool declareArray(ExecutionStack *stack, bool global, bool silent, QString *err = 0);
    static bool declareFunction(ExecutionStack *stack, bool global, bool silent, QString *err = 0);
    static bool declareVariable(ExecutionStack *stack, bool global, bool silent, QString *err = 0);
    static bool deleteEntity(ExecutionStack *stack, bool silent, QString *err = 0);
    static bool isDefined(ExecutionStack *stack, QString *err = 0);
    static bool renew(ExecutionStack *stack, bool silent, QString *err = 0);
    static bool set(ExecutionStack *stack, bool silent, QString *err = 0);
private:
    Q_DISABLE_COPY(SpecialFunction)
};

#endif // SPECIALFUNCTION_H
