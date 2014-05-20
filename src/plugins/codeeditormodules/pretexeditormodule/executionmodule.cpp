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

#include "executionmodule.h"
#include "pretexeditormodule.h"
#include "executionstack.h"
#include "token.h"
#include "tokendata.h"
#include "pretexbuiltinfunction.h"
#include "pretexfunction.h"
#include "pretexvariant.h"
#include "global.h"

#include <BAbstractCodeEditorDocument>

#include <QString>
#include <QList>
#include <QScopedPointer>

#include <QDebug>

/*============================================================================
================================ ExecutionModule =============================
============================================================================*/

/*============================== Static public methods =====================*/

PretexVariant ExecutionModule::executeSubprogram(ExecutionStack *stack, Subprogram_TokenData *a, const QString &caller,
                                                 bool *ok, QString *err)
{
    if (!a->statementCount())
        return bRet(ok, true, err, QString(), PretexVariant());
    ExecutionStack s(stack->obligArgs(), stack->optArgs(), stack->specialArgs(), caller, stack);
    QList<PretexVariant> list;
    foreach (int i, bRangeD(0, a->statementCount() - 1))
    {
        bool br = false;
        Statement_TokenData *st = a->statement(i);
        switch (st->statementType())
        {
        case Statement_TokenData::IntegerStatement:
            list << PretexVariant(st->integer());
            break;
        case Statement_TokenData::RealStatement:
            list << PretexVariant(st->real());
            break;
        case Statement_TokenData::StringStatement:
            list << PretexVariant(st->string());
            break;
        case Statement_TokenData::FunctionStatement:
        {
            bool b = false;
            PretexVariant v = executeFunction(&s, st->function(), &b, err);
            if (!b)
                return bRet(ok, false, PretexVariant());
            if (PretexBuiltinFunction::ReturnFlag == s.flag())
                return bRet(ok, true, err, QString(), v);
            else if (PretexBuiltinFunction::NoFlag != s.flag())
            {
                stack->setFlag(s.flag());
                br = true;
            }
            else
                list << v;
            break;
        }
        case Statement_TokenData::ArgumentNoStatement:
        {
            ArgumentNo_TokenData *an = st->argumentNo();
            int argNo = -1;
            if (an->argumentNoType() == ArgumentNo_TokenData::IntegerArgumentNo)
            {
                argNo = an->integer();
            }
            else
            {
                bool b = false;
                PretexVariant v = executeFunction(&s, an->function(), &b, err);
                if (!b)
                    return bRet(ok, false, PretexVariant());
                if (!v.type() != PretexVariant::Int)
                    return bRet(err, tr("Argument No must be an integer", "error"), ok, false, PretexVariant());
                argNo = v.toInt();
            }
            if (argNo < 0 || argNo >= s.maxArgCount())
                return bRet(err, tr("Invalid argument No", "error"), ok, false, PretexVariant());
            if (argNo < s.obligArgCount())
                list << s.obligArg(argNo);
            else
                list << s.optArg(argNo - s.obligArgCount());
            break;
        }
        default:
            //This can never happen
            break;
        }
        if (br)
            break;
    }
    PretexVariant r;
    switch (Global::typeToCastTo(PretexVariant::Int, list))
    {
    case PretexVariant::Int:
    {
        int i = 0;
        foreach (const PretexVariant &v, list)
            i += v.toInt();
        r = PretexVariant(i);
        break;
    }
    case PretexVariant::Real:
    {
        double d = 0.0;
        foreach (const PretexVariant &v, list)
            d += v.toReal();
        r = PretexVariant(d);
        break;
    }
    case PretexVariant::String:
    {
        QString s;
        foreach (const PretexVariant &v, list)
            s += v.toString();
        r = PretexVariant(s);
        break;
    }
    default:
        //This can never happen
        break;
    }
    return bRet(ok, true, err, QString(), r);
}

/*============================== Public constructors =======================*/

ExecutionModule::ExecutionModule()
{
    mprog = 0;
    mdoc = 0;
    mstack = 0;
}

ExecutionModule::ExecutionModule(Token *program, BAbstractCodeEditorDocument *doc, ExecutionStack *stack)
{
    mprog = program;
    mdoc = doc;
    mstack = stack;
}

/*============================== Public methods ============================*/

void ExecutionModule::setProgram(Token *prog)
{
    mprog = prog;
}

void ExecutionModule::setDocument(BAbstractCodeEditorDocument *doc)
{
    mdoc = doc;
}

void ExecutionModule::setExecutionStack(ExecutionStack *stack)
{
    mstack = stack;
}

Token *ExecutionModule::program() const
{
    return mprog;
}

BAbstractCodeEditorDocument *ExecutionModule::document() const
{
    return mdoc;
}

ExecutionStack *ExecutionModule::executionStack() const
{
    return mstack;
}

bool ExecutionModule::execute(QString *err)
{
    if (!mprog)
        return bRet(err, tr("No program token provided", "error"), false);
    if (!mdoc)
        return bRet(err, tr("No document provided", "error"), false);
    if (!mstack)
        return bRet(err, tr("No stack provided", "error"), false);
    if (mprog->type() != Token::Program_Token)
        return bRet(err, tr("Invalid program token", "error"), false);
    Program_TokenData *td = DATA_CAST(Program, mprog);
    ExecutionStack stack(mdoc, mstack);
    foreach (int i, bRangeD(0, td->functionCount() - 1))
    {
        bool b = false;
        executeFunction(&stack, td->function(i), &b, err);
        if (!b)
            return false;
    }
    return bRet(err, QString(), true);
}

/*============================== Static private methods ====================*/

PretexVariant ExecutionModule::executeFunction(ExecutionStack *stack, Function_TokenData *f, bool *ok, QString *err)
{
    QString name = f->name(); //Name can not be empty, thanks to LexicalAnalyzer/Parser
    ExecutionStack::NameType nt = ExecutionStack::UnknownName;
    if (!stack->isNameOccupied(name, true, &nt))
        return bRet(err, tr("Unknown identifier:", "error") + " " + name, ok, false, PretexVariant());
    switch (nt)
    {
    case ExecutionStack::VariableName:
    {
        if (f->obligatoryArgumentCount() != 1 || f->optionalArgumentCount()
                || f->obligatoryArgument(0)->statementCount())
            return bRet(err, tr("Argument(s) given to a variable:", "error") + " " + name, ok, false, PretexVariant());
        stack->setReturnValue(stack->variable(name));
        break;
    }
    case ExecutionStack::ArrayName:
    {
        if (f->obligatoryArgumentCount() != 1)
            return bRet(err, tr("Array access sintax error:", "error") + " " + name, ok, false, PretexVariant());
        PretexArray::Dimensions dimensions = stack->arrayDimensions(name);
        if (f->optionalArgumentCount() + 1 != dimensions.size())
            return bRet(err, tr("Invalid array dimension:", "error") + " " + name, ok, false, PretexVariant());
        QList<int> indexes;
        bool b = false;
        PretexVariant v = executeSubprogram(stack, f->obligatoryArgument(0), "", &b, err);
        if (!b)
            return bRet(ok, false, PretexVariant());
        if (v.type() != PretexVariant::Int)
            return bRet(err, tr("Array index must be an integer:", "error") + " " + name, ok, false, PretexVariant());
        indexes << v.toInt();
        if (indexes.first() < 0 || indexes.first() >= dimensions.first())
            return bRet(err, tr("Array index out of range:", "error") + " " + name, ok, false, PretexVariant());
        foreach (int i, bRangeD(0, f->optionalArgumentCount() - 1))
        {
            b = false;
            v = executeSubprogram(stack, f->optionalArgument(i), "", &b, err);
            if (!b)
                return bRet(ok, false, PretexVariant());
            if (v.type() != PretexVariant::Int)
                return bRet(err, tr("Array index must be an integer:", "error") + " " + name, ok, false,
                            PretexVariant());
            indexes << v.toInt();
            if (indexes.at(i + i) < 0 || indexes.at(i + 1) >= dimensions.at(i + 1))
                return bRet(err, tr("Array index out of range:", "error") + " " + name, ok, false, PretexVariant());
        }
        stack->setReturnValue(stack->arrayElement(name, indexes));
        break;
    }
    case ExecutionStack::UserFunctionName:
    {
        if (!stack->function(name)->execute(stack, f, err))
            return bRet(ok, false, PretexVariant());
        //TODO: Check flags
        return bRet(err, QString(), ok, true, stack->returnValue());
    }
    case ExecutionStack::BuiltinFunctionName:
    {
        if (!PretexBuiltinFunction::functionForName(name)->execute(stack, f, err))
            return bRet(ok, false, PretexVariant());
        //TODO: Check flags
        return bRet(err, QString(), ok, true, stack->returnValue());
    }
    default:
        //This must not ever occur
        break;
    }
    return bRet(err, QString(), ok, true, stack->returnValue());
}
