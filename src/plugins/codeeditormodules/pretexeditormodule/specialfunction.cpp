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

#include "specialfunction.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "executionstack.h"
#include "global.h"
#include "executionmodule.h"
#include "tokendata.h"
#include "token.h"

#include <BeQtGlobal>

#include <QList>
#include <QString>

#include <QDebug>

B_DECLARE_TRANSLATE_FUNCTION

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool declareVariable(ExecutionStack *stack, bool global, bool silent, QString *err)
{
    if (stack->parent()->isNameOccupied(stack->obligArg().toString(), false))
        return bRet(err, silent ? QString() : translate("declareVariable", "Identifier is occupied", "error"), silent);
    QString e;
    bool b = stack->parent()->declareVar(global, stack->obligArg().toString(), stack->obligArg(1), &e);
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, silent ? QString() : e, silent ? true : b);
}

static bool declareArray(ExecutionStack *stack, bool global, bool silent, QString *err)
{
    if (stack->parent()->isNameOccupied(stack->obligArg().toString(), false))
        return bRet(err, silent ? QString() : translate("declareArray", "Identifier is occupied", "error"), silent);
    PretexArray::Dimensions dimensions;
    if (stack->obligArg(1).type() != PretexVariant::Int)
        return bRet(err, silent ? QString() : translate("declareArray", "Array dimension must be an integer", "error"),
                    silent);
    dimensions << stack->obligArg(1).toInt();
    if (dimensions.last() <= 0)
        return bRet(err, silent ? QString() : translate("declareArray", "Invalid array dimension", "error"), silent);
    foreach (int i, bRangeD(0, stack->optArgCount() - 1))
    {
        if (stack->optArg(i).type() != PretexVariant::Int)
            return bRet(err, silent ? QString() : translate("declareArray", "Array dimension must be an integer",
                                                            "error"), silent);
        dimensions << stack->optArg(i).toInt();
        if (dimensions.last() <= 0)
            return bRet(err, silent ? QString() : translate("declareArray", "Invalid array dimension", "error"),
                        silent);
    }
    QString e;
    bool b = stack->parent()->declareArray(global, stack->obligArg(0).toString(), dimensions, &e);
    return bRet(err, silent ? QString() : e, silent ? true : b);
}

static bool declareFunction(ExecutionStack *stack, bool global, bool silent, QString *err)
{
    if (stack->parent()->isNameOccupied(stack->obligArg().toString(), false))
        return bRet(err, silent ? QString() : translate("declareFunction", "Identifier is occupied", "error"), silent);
    QString e;
    int opt = !stack->optArg().isNull() ? stack->optArg().toInt() : 0;
    bool b = stack->parent()->declareFunc(global, stack->obligArg(0).toString(), stack->obligArg(1).toInt(), opt,
                                          stack->specialArg(), err);
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, silent ? QString() : e, silent ? true : b);
}

static bool set(ExecutionStack *stack, bool silent, QString *err)
{
    ExecutionStack::NameType t = ExecutionStack::UnknownName;
    if (!stack->parent()->isNameOccupied(stack->obligArg().toString(), true, &t))
        return bRet(err, silent ? QString() : translate("set", "No such identifier", "error"), silent);
    switch (t)
    {
    case ExecutionStack::VariableName:
    {
        QString e;
        if (!stack->parent()->setVar(stack->obligArg(0).toString(), stack->obligArg(1), &e))
            return bRet(err, silent ? QString() : e, silent);
        break;
    }
    case ExecutionStack::ArrayName:
    {
        PretexArray::Dimensions dimensions = stack->parent()->arrayDimensions(stack->obligArg(0).toString());
        if (stack->optArgCount() != dimensions.size())
            return bRet(err, silent ? QString() : translate("set", "Array dimension mismatch", "error"), silent);
        PretexArray::Indexes indexes;
        foreach (int i, bRangeD(0, stack->optArgCount() - 1))
        {
            if (stack->optArg(i).type() != PretexVariant::Int)
                return bRet(err, silent ? QString() : translate("set", "Array index must be an integer", "error"),
                            silent);
            indexes << stack->optArg(i).toInt();
            if (indexes.at(i) < 0 || indexes.at(i) >= dimensions.at(i))
                return bRet(err, silent ? QString() : translate("set", "Invalid array index", "error"), silent);
        }
        QString e;
        if (!stack->parent()->setArrayElement(stack->obligArg(0).toString(), indexes, stack->obligArg(1), &e))
            return bRet(err, silent ? QString() : e, silent);
        break;
    }
    case ExecutionStack::UserFunctionName:
        return bRet(err, silent ? QString() : translate("set", "Attempt to set function", "error"), silent);
    case ExecutionStack::BuiltinFunctionName:
        return bRet(err, silent ? QString() : translate("set", "Attempt to set builtin function", "error"), silent);
    default:
        //This can never occur
        break;
    }
    return bRet(err, QString(), true);
}

static bool renew(ExecutionStack *stack, bool silent, QString *err)
{
    ExecutionStack::NameType t = ExecutionStack::UnknownName;
    if (!stack->parent()->isNameOccupied(stack->obligArg().toString(), true, &t))
        return bRet(err, silent ? QString() : translate("renew", "No such identifier", "error"), silent);
    switch (t)
    {
    case ExecutionStack::VariableName:
        return bRet(err, silent ? QString() : translate("renew", "Attempt to renew a vriable", "error"), silent);
    case ExecutionStack::ArrayName:
        return bRet(err, silent ? QString() : translate("renew", "Attempt to renew an array", "error"), silent);
    case ExecutionStack::UserFunctionName:
    {
        QString e;
        if (!stack->parent()->setFunc(stack->obligArg(0).toString(), stack->specialArg(), &e))
            return bRet(err, silent ? QString() : e, silent);
        break;
    }
    case ExecutionStack::BuiltinFunctionName:
        return bRet(err, silent ? QString() : translate("renew", "Attempt to set builtin function", "error"), silent);
    default:
        //This can never occur
        break;
    }
    return bRet(err, QString(), true);
}

static bool deleteEntity(ExecutionStack *stack, bool silent, QString *err)
{
    QString e;
    bool b = stack->parent()->undeclare(stack->obligArg().toString(), &e);
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, silent ? QString() : e, silent ? true : b);
}

static bool isDefined(ExecutionStack *stack, QString *err)
{
    bool b = stack->parent()->isNameOccupied(stack->obligArg().toString(), true);
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

/*============================================================================
================================ SpecialFunction =============================
============================================================================*/

/*============================== Public constructors =======================*/

SpecialFunction::SpecialFunction(Type t)
{
    mtype = t;
}

/*============================== Public methods ============================*/

QString SpecialFunction::name() const
{
    switch (mtype)
    {
    case NewVarType:
        return "newVar";
    case NewLocalVarType:
        return "newLocalVar";
    case NewGlobalVarType:
        return "newGlobalVar";
    case TryNewVarType:
        return "tryNewVar";
    case TryNewLocalVarType:
        return "tryNewLocalVar";
    case TryNewGlobalVarType:
        return "tryNewGlobalVar";
    case NewArrayType:
        return "newArray";
    case NewLocalArrayType:
        return "newLocalArray";
    case NewGlobalArrayType:
        return "newGlobalArray";
    case TryNewArrayType:
        return "tryNewArray";
    case TryNewLocalArrayType:
        return "tryNewLocalArray";
    case TryNewGlobalArrayType:
        return "tryNewGlobalArray";
    case NewFuncType:
        return "newFunc";
    case NewLocalFuncType:
        return "newLocalFunc";
    case NewGlobalFuncType:
        return "newGlobalFunc";
    case TryNewFuncType:
        return "tryNewFunc";
    case TryNewLocalFuncType:
        return "tryNewLocalFunc";
    case TryNewGlobalFuncType:
        return "tryNewGlobalFunc";
    case SetType:
        return "set";
    case RenewFuncType:
        return "renewFunc";
    case DeleteType:
        return "delete";
    case TrySetType:
        return "trySet";
    case TryRenewFuncType:
        return "tryRenewFunc";
    case TryDeleteType:
        return "tryDelete";
    case IsDefinedType:
        return "isDefined";
    default:
        break;
    }
    return QString();
}

int SpecialFunction::obligatoryArgumentCount() const
{
    switch (mtype)
    {
    case NewVarType:
    case NewLocalVarType:
    case NewGlobalVarType:
    case TryNewVarType:
    case TryNewLocalVarType:
    case TryNewGlobalVarType:
    case NewArrayType:
    case NewLocalArrayType:
    case NewGlobalArrayType:
    case TryNewArrayType:
    case TryNewLocalArrayType:
    case TryNewGlobalArrayType:
    case SetType:
    case RenewFuncType:
    case TrySetType:
    case TryRenewFuncType:
        return 2;
    case NewFuncType:
    case NewLocalFuncType:
    case NewGlobalFuncType:
    case TryNewFuncType:
    case TryNewLocalFuncType:
    case TryNewGlobalFuncType:
        return 3;
    case DeleteType:
    case TryDeleteType:
    case IsDefinedType:
        return 1;
    default:
        break;
    }
    return 0;
}

int SpecialFunction::optionalArgumentCount() const
{
    switch (mtype)
    {
    case NewVarType:
    case NewLocalVarType:
    case NewGlobalVarType:
    case TryNewVarType:
    case TryNewLocalVarType:
    case TryNewGlobalVarType:
    case RenewFuncType:
    case DeleteType:
    case TryRenewFuncType:
    case TryDeleteType:
    case IsDefinedType:
        return 0;
    case NewFuncType:
    case NewLocalFuncType:
    case NewGlobalFuncType:
    case TryNewFuncType:
    case TryNewLocalFuncType:
    case TryNewGlobalFuncType:
        return 1;
    case NewArrayType:
    case NewLocalArrayType:
    case NewGlobalArrayType:
    case TryNewArrayType:
    case TryNewLocalArrayType:
    case TryNewGlobalArrayType:
    case SetType:
    case TrySetType:
        return -1;
    default:
        break;
    }
    return 0;
}

bool SpecialFunction::execute(ExecutionStack *stack, Function_TokenData *f, QString *err)
{
    switch (mtype)
    {
    case RenewFuncType:
    case TryRenewFuncType:
    {
        if (!standardCheck(f, err))
            return false;
        bool b = false;
        PretexVariant a = ExecutionModule::executeSubprogram(stack, f->obligatoryArgument(0), &b, err);
        if (!b)
            return false;
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(1));
        ExecutionStack s(0, QList<PretexVariant>() << a, QList<PretexVariant>(), QList<Token>() << t, name(), stack,
                         functionFlags(name()));
        if (!execute(&s, err))
            return false;
        stack->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    case NewFuncType:
    case NewLocalFuncType:
    case NewGlobalFuncType:
    case TryNewFuncType:
    case TryNewLocalFuncType:
    case TryNewGlobalFuncType:
    {
        if (!standardCheck(f, err))
            return false;
        bool b = false;
        PretexVariant a0 = ExecutionModule::executeSubprogram(stack, f->obligatoryArgument(0), &b, err);
        if (!b)
            return false;
        b = false;
        PretexVariant a1 = ExecutionModule::executeSubprogram(stack, f->obligatoryArgument(2), &b, err);
        if (!b)
            return false;
        if (a1.type() != PretexVariant::Int)
            return bRet(err, tr("Argument count must be an integer", "error") + " " + name(), false);
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(1));
        QList<PretexVariant> optArgs;
        if (f->optionalArgumentCount())
        {
            PretexVariant a = ExecutionModule::executeSubprogram(stack, f->optionalArgument(0), &b, err);
            if (!b)
                return false;
            if (a.type() != PretexVariant::Int)
                return bRet(err, tr("Argument count must be an integer", "error") + " " + name(), false);
            optArgs << a;
        }
        ExecutionStack s(0, QList<PretexVariant>() << a0 << a1, optArgs, QList<Token>() << t, name(), stack,
                         functionFlags(name()));
        if (!execute(&s, err))
            return false;
        stack->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    default:
        return PretexBuiltinFunction::execute(stack, f, err);
    }
}

/*============================== Protected methods =========================*/

bool SpecialFunction::execute(ExecutionStack *stack, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case NewVarType:
        return declareVariable(stack, false, false, err);
    case NewLocalVarType:
        return declareVariable(stack, false, false, err);
    case NewGlobalVarType:
        return declareVariable(stack, true, false, err);
    case TryNewVarType:
        return declareVariable(stack, false, true, err);
    case TryNewLocalVarType:
        return declareVariable(stack, false, true, err);
    case TryNewGlobalVarType:
        return declareVariable(stack, true, true, err);
    case NewArrayType:
        return declareArray(stack, false, false, err);
    case NewLocalArrayType:
        return declareArray(stack, false, false, err);
    case NewGlobalArrayType:
        return declareArray(stack, true, false, err);
    case TryNewArrayType:
        return declareArray(stack, false, true, err);
    case TryNewLocalArrayType:
        return declareArray(stack, false, true, err);
    case TryNewGlobalArrayType:
        return declareArray(stack, true, true, err);
    case NewFuncType:
        return declareFunction(stack, false, false, err);
    case NewLocalFuncType:
        return declareFunction(stack, false, false, err);
    case NewGlobalFuncType:
        return declareFunction(stack, true, false, err);
    case TryNewFuncType:
        return declareFunction(stack, false, true, err);
    case TryNewLocalFuncType:
        return declareFunction(stack, false, true, err);
    case TryNewGlobalFuncType:
        return declareFunction(stack, true, true, err);
    case SetType:
        return set(stack, false, err);
    case TrySetType:
        return set(stack, true, err);
    case RenewFuncType:
        return renew(stack, false, err);
    case TryRenewFuncType:
        return renew(stack, true, err);
    case DeleteType:
        return deleteEntity(stack, false, err);
    case TryDeleteType:
        return deleteEntity(stack, true, err);
    case IsDefinedType:
        return isDefined(stack, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
