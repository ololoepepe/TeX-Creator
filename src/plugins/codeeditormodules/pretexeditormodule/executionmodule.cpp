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

#include "executionmodule.h"

#include "executioncontext.h"
#include "global.h"
#include "pretexbuiltinfunction.h"
#include "pretexeditormodule.h"
#include "pretexfunction.h"
#include "pretexvariant.h"
#include "tokendata.h"
#include "token.h"

#include <BAbstractCodeEditorDocument>

#include <QDebug>
#include <QList>
#include <QScopedPointer>
#include <QString>

/*============================================================================
================================ ExecutionModule =============================
============================================================================*/

/*============================== Public constructors =======================*/

ExecutionModule::ExecutionModule()
{
    mprog = 0;
    mdoc = 0;
    mcontext = 0;
}

ExecutionModule::ExecutionModule(Token *program, BAbstractCodeEditorDocument *doc, ExecutionContext *context)
{
    mprog = program;
    mdoc = doc;
    mcontext = context;
}

/*============================== Static public methods =====================*/

PretexVariant ExecutionModule::executeSubprogram(ExecutionContext *context, Subprogram_TokenData *a,
                                                 const QString &caller, bool *ok, QString *err)
{
    if (!a->statementCount())
        return bRet(ok, true, err, QString(), PretexVariant());
    ExecutionContext s(context->obligArgs(), context->optArgs(), context->specialArgs(), caller, context);
    QList<PretexVariant> list;
    foreach (int i, bRangeD(0, a->statementCount() - 1)) {
        bool br = false;
        Statement_TokenData *st = a->statement(i);
        switch (st->statementType()) {
        case Statement_TokenData::IntegerStatement: {
            list << PretexVariant(st->integer());
            break;
        }
        case Statement_TokenData::RealStatement: {
            list << PretexVariant(st->real());
            break;
        }
        case Statement_TokenData::StringStatement: {
            list << PretexVariant(st->string());
            break;
        }
        case Statement_TokenData::FunctionStatement: {
            bool b = false;
            PretexVariant v = executeFunction(&s, st->function(), &b, err);
            if (!b)
                return bRet(ok, false, PretexVariant());
            if (PretexBuiltinFunction::ReturnFlag == s.flag()) {
                return bRet(ok, true, err, QString(), v);
            } else if (PretexBuiltinFunction::NoFlag != s.flag()) {
                context->setFlag(s.flag());
                br = true;
            } else {
                list << v;
            }
            break;
        }
        case Statement_TokenData::ArgumentNoStatement: {
            ArgumentNo_TokenData *an = st->argumentNo();
            int argNo = -1;
            if (an->argumentNoType() == ArgumentNo_TokenData::IntegerArgumentNo) {
                argNo = an->integer();
            } else {
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
        default: {
            //This can never happen
            break;
        }
        }
        if (br)
            break;
    }
    PretexVariant r;
    switch (Global::typeToCastTo(PretexVariant::Int, list)) {
    case PretexVariant::Int: {
        int i = 0;
        foreach (const PretexVariant &v, list)
            i += v.toInt();
        r = PretexVariant(i);
        break;
    }
    case PretexVariant::Real: {
        double d = 0.0;
        foreach (const PretexVariant &v, list)
            d += v.toReal();
        r = PretexVariant(d);
        break;
    }
    case PretexVariant::String: {
        QString s;
        foreach (const PretexVariant &v, list)
            s += v.toString();
        r = PretexVariant(s);
        break;
    }
    default: {
        QString s;
        foreach (const PretexVariant &v, list)
            s += !v.isNull() ? v.toString() : QString();
        r = PretexVariant(s);
        break;
    }
    }
    return bRet(ok, true, err, QString(), r);
}

/*============================== Public methods ============================*/

BAbstractCodeEditorDocument *ExecutionModule::document() const
{
    return mdoc;
}

bool ExecutionModule::execute(QString *err)
{
    if (!mprog)
        return bRet(err, tr("No program token provided", "error"), false);
    if (!mdoc)
        return bRet(err, tr("No document provided", "error"), false);
    if (!mcontext)
        return bRet(err, tr("No context provided", "error"), false);
    if (mprog->type() != Token::Program_Token)
        return bRet(err, tr("Invalid program token", "error"), false);
    Program_TokenData *td = DATA_CAST(Program, mprog);
    ExecutionContext context(mdoc, mcontext);
    foreach (int i, bRangeD(0, td->functionCount() - 1)) {
        bool b = false;
        executeFunction(&context, td->function(i), &b, err);
        if (!b)
            return false;
        if (context.flag() == PretexBuiltinFunction::ReturnFlag)
            return true;
    }
    return bRet(err, QString(), true);
}

ExecutionContext *ExecutionModule::executionContext() const
{
    return mcontext;
}

Token *ExecutionModule::program() const
{
    return mprog;
}

void ExecutionModule::setDocument(BAbstractCodeEditorDocument *doc)
{
    mdoc = doc;
}

void ExecutionModule::setExecutionContext(ExecutionContext *context)
{
    mcontext = context;
}

void ExecutionModule::setProgram(Token *prog)
{
    mprog = prog;
}

/*============================== Static private methods ====================*/

PretexVariant ExecutionModule::executeFunction(ExecutionContext *context, Function_TokenData *f, bool *ok,
                                               QString *err)
{
    QString name = f->name(); //NOTE: Name can not be empty, thanks to LexicalAnalyzer/Parser
    ExecutionContext::NameType nt = ExecutionContext::UnknownName;
    if (!context->isNameOccupied(name, true, &nt))
        return bRet(err, tr("Unknown identifier:", "error") + " " + name, ok, false, PretexVariant());
    switch (nt) {
    case ExecutionContext::VariableName: {
        if (f->obligatoryArgumentCount() != 1 || f->optionalArgumentCount()
                || f->obligatoryArgument(0)->statementCount())
            return bRet(err, tr("Argument(s) given to a variable:", "error") + " " + name, ok, false, PretexVariant());
        context->setReturnValue(context->variable(name));
        break;
    }
    case ExecutionContext::ArrayName: {
        if (f->obligatoryArgumentCount() != 1)
            return bRet(err, tr("Array access sintax error:", "error") + " " + name, ok, false, PretexVariant());
        PretexArray::Dimensions dimensions = context->arrayDimensions(name);
        if (f->optionalArgumentCount() + 1 != dimensions.size())
            return bRet(err, tr("Invalid array dimension:", "error") + " " + name, ok, false, PretexVariant());
        QList<int> indexes;
        bool b = false;
        PretexVariant v = executeSubprogram(context, f->obligatoryArgument(0), "", &b, err);
        if (!b)
            return bRet(ok, false, PretexVariant());
        if (v.type() != PretexVariant::Int)
            return bRet(err, tr("Array index must be an integer:", "error") + " " + name, ok, false, PretexVariant());
        indexes << v.toInt();
        if (indexes.first() < 0 || indexes.first() >= dimensions.first())
            return bRet(err, tr("Array index out of range:", "error") + " " + name, ok, false, PretexVariant());
        foreach (int i, bRangeD(0, f->optionalArgumentCount() - 1)) {
            b = false;
            v = executeSubprogram(context, f->optionalArgument(i), "", &b, err);
            if (!b)
                return bRet(ok, false, PretexVariant());
            if (v.type() != PretexVariant::Int) {
                return bRet(err, tr("Array index must be an integer:", "error") + " " + name, ok, false,
                            PretexVariant());
            }
            indexes << v.toInt();
            if (indexes.at(i + i) < 0 || indexes.at(i + 1) >= dimensions.at(i + 1))
                return bRet(err, tr("Array index out of range:", "error") + " " + name, ok, false, PretexVariant());
        }
        context->setReturnValue(context->arrayElement(name, indexes));
        break;
    }
    case ExecutionContext::UserFunctionName: {
        if (!context->function(name)->execute(context, f, err))
            return bRet(ok, false, PretexVariant());
        return bRet(err, QString(), ok, true, context->returnValue());
    }
    case ExecutionContext::BuiltinFunctionName: {
        if (!PretexBuiltinFunction::functionForName(name)->execute(context, f, err))
            return bRet(ok, false, PretexVariant());
        return bRet(err, QString(), ok, true, context->returnValue());
    }
    default: {
        //This must not ever happen
        break;
    }
    }
    return bRet(err, QString(), ok, true, context->returnValue());
}
