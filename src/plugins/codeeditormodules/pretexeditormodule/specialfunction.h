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

#ifndef SPECIALFUNCTION_H
#define SPECIALFUNCTION_H

class ExecutionStack;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QList>
#include <QCoreApplication>

/*============================================================================
================================ SpecialFunction =============================
============================================================================*/

class SpecialFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(SpecialFunction)
public:
    enum Type
    {
        NewVarType,
        NewLocalVarType,
        NewGlobalVarType,
        TryNewVarType,
        TryNewLocalVarType,
        TryNewGlobalVarType,
        NewArrayType,
        NewLocalArrayType,
        NewGlobalArrayType,
        TryNewArrayType,
        TryNewLocalArrayType,
        TryNewGlobalArrayType,
        NewFuncType,
        NewLocalFuncType,
        NewGlobalFuncType,
        TryNewFuncType,
        TryNewLocalFuncType,
        TryNewGlobalFuncType,
        SetType,
        TrySetType,
        RenewFuncType,
        TryRenewFuncType,
        DeleteType,
        TryDeleteType,
        IsDefinedType
    };
public:
    explicit SpecialFunction(Type t);
public:
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
    bool execute(ExecutionStack *stack, Function_TokenData *f, QString *err = 0);
protected:
    bool execute(ExecutionStack *stack, QString *err = 0);
private:
    Type mtype;
private:
    Q_DISABLE_COPY(SpecialFunction)
};

#endif // SPECIALFUNCTION_H
