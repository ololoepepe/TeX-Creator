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

#include "specialfunction.h"

#include "executionmodule.h"
#include "executioncontext.h"
#include "global.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "tokendata.h"
#include "token.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QList>
#include <QString>

/*============================================================================
================================ SpecialFunction =============================
============================================================================*/

/*============================== Public constructors =======================*/

SpecialFunction::SpecialFunction(Type t)
{
    mtype = t;
}

/*============================== Public methods ============================*/

bool SpecialFunction::execute(ExecutionContext *context, Function_TokenData *f, QString *err)
{
    switch (mtype) {
    case RenewFuncType:
    case TryRenewFuncType: {
        if (!standardCheck(f, err))
            return false;
        bool b = false;
        PretexVariant a = ExecutionModule::executeSubprogram(context, f->obligatoryArgument(0), "renewFunc", &b, err);
        if (!b)
            return false;
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(1));
        ExecutionContext s(QList<PretexVariant>() << a, QList<PretexVariant>(), QList<Token>() << t, name(), context);
        if (!execute(&s, err))
            return false;
        context->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    case NewFuncType:
    case NewLocalFuncType:
    case NewGlobalFuncType:
    case TryNewFuncType:
    case TryNewLocalFuncType:
    case TryNewGlobalFuncType: {
        if (!standardCheck(f, err))
            return false;
        bool b = false;
        PretexVariant a0 = ExecutionModule::executeSubprogram(context, f->obligatoryArgument(0), "newFunc", &b, err);
        if (!b)
            return false;
        b = false;
        Token t(Token::Subprogram_Token);
        DATA_CAST(Subprogram, &t)->copyStatements(f->obligatoryArgument(1));
        QList<PretexVariant> optArgs;
        foreach (int i, bRangeD(0, f->optionalArgumentCount() - 1)) {
            PretexVariant a = ExecutionModule::executeSubprogram(context, f->optionalArgument(i), "newFunc", &b, err);
            if (!b)
                return false;
            if (i < 2 && a.type() != PretexVariant::Int)
                return bRet(err, tr("Argument count must be an integer", "error") + " " + name(), false);
            optArgs << a;
        }
        ExecutionContext s(QList<PretexVariant>() << a0, optArgs, QList<Token>() << t, name(), context);
        if (!execute(&s, err))
            return false;
        context->setReturnValue(s.returnValue());
        return bRet(err, QString(), true);
    }
    default: {
        return PretexBuiltinFunction::execute(context, f, err);
    }
    }
}

QString SpecialFunction::name() const
{
    switch (mtype) {
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
    switch (mtype) {
    case NewArrayType:
    case NewLocalArrayType:
    case NewGlobalArrayType:
    case TryNewArrayType:
    case TryNewLocalArrayType:
    case TryNewGlobalArrayType:
    case NewFuncType:
    case NewLocalFuncType:
    case NewGlobalFuncType:
    case TryNewFuncType:
    case TryNewLocalFuncType:
    case TryNewGlobalFuncType:
    case SetType:
    case RenewFuncType:
    case TrySetType:
    case TryRenewFuncType:
        return 2;
    case NewVarType:
    case NewLocalVarType:
    case NewGlobalVarType:
    case TryNewVarType:
    case TryNewLocalVarType:
    case TryNewGlobalVarType:
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
    switch (mtype) {
    case RenewFuncType:
    case DeleteType:
    case TryRenewFuncType:
    case TryDeleteType:
    case IsDefinedType:
        return 0;
    case NewVarType:
    case NewLocalVarType:
    case NewGlobalVarType:
    case TryNewVarType:
    case TryNewLocalVarType:
    case TryNewGlobalVarType:
        return 1;
    case NewFuncType:
    case NewLocalFuncType:
    case NewGlobalFuncType:
    case TryNewFuncType:
    case TryNewLocalFuncType:
    case TryNewGlobalFuncType:
        return 2;
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

/*============================== Protected methods =========================*/

bool SpecialFunction::execute(ExecutionContext *context, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype) {
    case NewVarType:
        return declareVariable(context, false, false, err);
    case NewLocalVarType:
        return declareVariable(context, false, false, err);
    case NewGlobalVarType:
        return declareVariable(context, true, false, err);
    case TryNewVarType:
        return declareVariable(context, false, true, err);
    case TryNewLocalVarType:
        return declareVariable(context, false, true, err);
    case TryNewGlobalVarType:
        return declareVariable(context, true, true, err);
    case NewArrayType:
        return declareArray(context, false, false, err);
    case NewLocalArrayType:
        return declareArray(context, false, false, err);
    case NewGlobalArrayType:
        return declareArray(context, true, false, err);
    case TryNewArrayType:
        return declareArray(context, false, true, err);
    case TryNewLocalArrayType:
        return declareArray(context, false, true, err);
    case TryNewGlobalArrayType:
        return declareArray(context, true, true, err);
    case NewFuncType:
        return declareFunction(context, false, false, err);
    case NewLocalFuncType:
        return declareFunction(context, false, false, err);
    case NewGlobalFuncType:
        return declareFunction(context, true, false, err);
    case TryNewFuncType:
        return declareFunction(context, false, true, err);
    case TryNewLocalFuncType:
        return declareFunction(context, false, true, err);
    case TryNewGlobalFuncType:
        return declareFunction(context, true, true, err);
    case SetType:
        return set(context, false, err);
    case TrySetType:
        return set(context, true, err);
    case RenewFuncType:
        return renew(context, false, err);
    case TryRenewFuncType:
        return renew(context, true, err);
    case DeleteType:
        return deleteEntity(context, false, err);
    case TryDeleteType:
        return deleteEntity(context, true, err);
    case IsDefinedType:
        return isDefined(context, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}

/*============================== Static private methods ====================*/

bool SpecialFunction::declareArray(ExecutionContext *context, bool global, bool silent, QString *err)
{
    ExecutionContext::NameType t = ExecutionContext::UnknownName;
    if (context->parent()->isNameOccupied(context->obligArg().toString(), false, &t)) {
        if (silent && ExecutionContext::ArrayName == t)
            return bRet(err, QString(), true);
        else
            return bRet(err, tr("Identifier is occupied", "error"), false);
    }
    PretexArray::Dimensions dimensions;
    if (context->obligArg(1).type() != PretexVariant::Int)
        return bRet(err, silent ? QString() : tr("Array dimension must be an integer", "error"), silent);
    dimensions << context->obligArg(1).toInt();
    if (dimensions.last() <= 0)
        return bRet(err, silent ? QString() : tr("Invalid array dimension", "error"), silent);
    foreach (int i, bRangeD(0, context->optArgCount() - 1)) {
        if (context->optArg(i).type() != PretexVariant::Int)
            return bRet(err, silent ? QString() : tr("Array dimension must be an integer", "error"), silent);
        dimensions << context->optArg(i).toInt();
        if (dimensions.last() <= 0)
            return bRet(err, silent ? QString() : tr("Invalid array dimension", "error"), silent);
    }
    QString e;
    bool b = context->parent()->declareArray(global, context->obligArg(0).toString(), dimensions, &e);
    return bRet(err, silent ? QString() : e, silent ? true : b);
}

bool SpecialFunction::declareFunction(ExecutionContext *context, bool global, bool silent, QString *err)
{
    ExecutionContext::NameType t = ExecutionContext::UnknownName;
    if (context->parent()->isNameOccupied(context->obligArg().toString(), false, &t)) {
        if (silent && ExecutionContext::UserFunctionName == t)
            return bRet(err, QString(), true);
        else
            return bRet(err, tr("Identifier is occupied", "error"), false);
    }
    QString e;
    int obl = !context->optArg(0).isNull() ? context->optArg(0).toInt() : 1;
    if (obl < 1)
        return bRet(err, silent ? QString() : tr("Invalid argument count", "error"), silent);
    int opt = !context->optArg(1).isNull() ? context->optArg(1).toInt() : 0;
    bool b = context->parent()->declareFunc(global, context->obligArg().toString(), obl, opt, context->specialArg(),
                                            err);
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, silent ? QString() : e, silent ? true : b);
}

bool SpecialFunction::declareVariable(ExecutionContext *context, bool global, bool silent, QString *err)
{
    ExecutionContext::NameType t = ExecutionContext::UnknownName;
    if (context->parent()->isNameOccupied(context->obligArg().toString(), false, &t)) {
        if (silent && ExecutionContext::VariableName == t)
            return bRet(err, QString(), true);
        else
            return bRet(err, tr("Identifier is occupied", "error"), false);
    }
    QString e;
    PretexVariant v;
    if (context->optArgCount())
        v = context->optArg();
    bool b = context->parent()->declareVar(global, context->obligArg().toString(), v, &e);
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, silent ? QString() : e, silent ? true : b);
}

bool SpecialFunction::deleteEntity(ExecutionContext *context, bool silent, QString *err)
{
    QString e;
    bool b = context->parent()->undeclare(context->obligArg().toString(), &e);
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, silent ? QString() : e, silent ? true : b);
}

bool SpecialFunction::isDefined(ExecutionContext *context, QString *err)
{
    bool b = context->parent()->isNameOccupied(context->obligArg().toString(), true);
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool SpecialFunction::renew(ExecutionContext *context, bool silent, QString *err)
{
    ExecutionContext::NameType t = ExecutionContext::UnknownName;
    if (!context->parent()->isNameOccupied(context->obligArg().toString(), true, &t))
        return bRet(err, silent ? QString() : tr("No such identifier", "error"), silent);
    switch (t) {
    case ExecutionContext::VariableName: {
        return bRet(err, silent ? QString() : tr("Attempt to renew a vriable", "error"), silent);
    }
    case ExecutionContext::ArrayName: {
        return bRet(err, silent ? QString() : tr("Attempt to renew an array", "error"), silent);
    }
    case ExecutionContext::UserFunctionName: {
        QString e;
        if (!context->parent()->setFunc(context->obligArg(0).toString(), context->specialArg(), &e))
            return bRet(err, silent ? QString() : e, silent);
        break;
    }
    case ExecutionContext::BuiltinFunctionName: {
        return bRet(err, silent ? QString() : tr("Attempt to set builtin function", "error"), silent);
    }
    default: {
        break;
    }
    }
    return bRet(err, QString(), true);
}

bool SpecialFunction::set(ExecutionContext *context, bool silent, QString *err)
{
    ExecutionContext::NameType t = ExecutionContext::UnknownName;
    if (!context->parent()->isNameOccupied(context->obligArg().toString(), true, &t))
        return bRet(err, silent ? QString() : tr("No such identifier", "error"), silent);
    switch (t) {
    case ExecutionContext::VariableName: {
        QString e;
        if (!context->parent()->setVar(context->obligArg(0).toString(), context->obligArg(1), &e))
            return bRet(err, silent ? QString() : e, silent);
        break;
    }
    case ExecutionContext::ArrayName: {
        PretexArray::Dimensions dimensions = context->parent()->arrayDimensions(context->obligArg(0).toString());
        if (context->optArgCount() != dimensions.size())
            return bRet(err, silent ? QString() : tr("Array dimension mismatch", "error"), silent);
        PretexArray::Indexes indexes;
        foreach (int i, bRangeD(0, context->optArgCount() - 1)) {
            if (context->optArg(i).type() != PretexVariant::Int)
                return bRet(err, tr("Array index must be an integer", "error"), false);
            indexes << context->optArg(i).toInt();
            if (indexes.at(i) < 0 || indexes.at(i) >= dimensions.at(i))
                return bRet(err, tr("Invalid array index", "error"), false);
        }
        QString e;
        if (!context->parent()->setArrayElement(context->obligArg(0).toString(), indexes, context->obligArg(1), &e))
            return bRet(err, silent ? QString() : e, silent);
        break;
    }
    case ExecutionContext::UserFunctionName: {
        return bRet(err, tr("Attempt to set function", "error"), silent);
    }
    case ExecutionContext::BuiltinFunctionName: {
        return bRet(err, silent ? QString() : tr("Attempt to set builtin function", "error"), silent);
    }
    default: {
        break;
    }
    }
    return bRet(err, QString(), true);
}
