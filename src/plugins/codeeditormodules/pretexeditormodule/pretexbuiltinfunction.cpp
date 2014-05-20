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

#include "pretexbuiltinfunction.h"
#include "mathfunction.h"
#include "booleanfunction.h"
#include "iofunction.h"
#include "specialfunction.h"
#include "tokendata.h"
#include "pretexvariant.h"
#include "executionmodule.h"
#include "generalfunction.h"

#include <BeQtGlobal>

#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

#include <QDebug>

/*============================================================================
================================ PretexBuiltinFunction =======================
============================================================================*/

/*============================== Static public methods =====================*/

PretexBuiltinFunction *PretexBuiltinFunction::functionForName(const QString &name)
{
    return mmap.value(name);
}

bool PretexBuiltinFunction::isBuiltinFunction(const QString &name)
{
    return mmap.contains(name);
}

QStringList PretexBuiltinFunction::specFuncNames()
{
    static const QStringList names = QStringList()
    << "for" << "set" << "renewFunc" << "delete" << "isDefined" << "trySet" << "tryRenewFunc" << "tryDelete"
    << "newVar" << "newLocalVar" << "newGlobalVar" << "tryNewVar" << "tryNewLocalVar" << "tryNewGlobalVar"
    << "newFunc" << "newLocalFunc" << "newGlobalFunc" << "tryNewFunc" << "tryNewLocalFunc" << "tryNewGlobalFunc"
    << "newArray" << "newLocalArray" << "newGlobalArray" << "tryNewArray" << "tryNewLocalArray" << "tryNewGlobalArray";
    return names;
}

QStringList PretexBuiltinFunction::normalFuncNames()
{
    static const QStringList names = QStringList() << "+" << "-" << "*" << "^" << "==" << "!=" << "<=" << "<" << ">="
        << ">" << "||" << "&&" << "add" << "subtract" << "multiply" << "divide" << "modulo" << "exponentiate" << "log"
        << "root" << "round" << "abs" << "random" << "equal" << "notEqual" << "lesserOrEqual" << "lesser"
        << "greaterOrEqual" << "greater" << "or" << "and" << "xor" << "insert" << "find" << "replace" << "press"
        << "showMessage" << "getInput" << "readFile" << "runDetached" << "run" << "isEmpty" << "toInteger"
        << "toReal" << "toString" << "format" << "if" << "wait" << "while" << "doWhile" << "until" << "doUntil";
    return names;
}

QStringList PretexBuiltinFunction::funcNames()
{
    init_once(QStringList, names, QStringList())
        names << normalFuncNames() << specFuncNames();
    return names;
}

ExecutionStack::SpecialFlags PretexBuiltinFunction::functionFlags(const QString &name)
{
    //
    return ExecutionStack::NoFlag;
}

void PretexBuiltinFunction::init()
{
    addFunc(new MathFunction(MathFunction::AddType), "add", "+");
    addFunc(new MathFunction(MathFunction::SubtractType), "subtract", "-");
    addFunc(new MathFunction(MathFunction::MultiplyType), "multiply", "*");
    addFunc(new MathFunction(MathFunction::DivideType), "divide");
    addFunc(new MathFunction(MathFunction::ModuloType), "modulo");
    addFunc(new MathFunction(MathFunction::ExponentiateType), "exponentiate", "^");
    addFunc(new MathFunction(MathFunction::LogType), "log");
    addFunc(new MathFunction(MathFunction::RootType), "root");
    addFunc(new MathFunction(MathFunction::RoundType), "round");
    addFunc(new MathFunction(MathFunction::AbsType), "abs");
    addFunc(new MathFunction(MathFunction::RandomType), "random");
    addFunc(new BooleanFunction(BooleanFunction::EqualType), "equal", "==");
    addFunc(new BooleanFunction(BooleanFunction::NotEqualType), "notEqual", "!=");
    addFunc(new BooleanFunction(BooleanFunction::LesserType), "lesser", "<");
    addFunc(new BooleanFunction(BooleanFunction::LesserOrEqualType), "lesserOrEqual", "<=");
    addFunc(new BooleanFunction(BooleanFunction::GreaterType), "greater", ">");
    addFunc(new BooleanFunction(BooleanFunction::GreaterOrEqualType), "greaterOrEqual", ">=");
    addFunc(new BooleanFunction(BooleanFunction::OrType), "or", "||");
    addFunc(new BooleanFunction(BooleanFunction::AndType), "and", "&&");
    addFunc(new BooleanFunction(BooleanFunction::XorType), "xor");
    addFunc(new IOFunction(IOFunction::InsertType), "insert");
    addFunc(new IOFunction(IOFunction::FindType), "find");
    addFunc(new IOFunction(IOFunction::ReplaceType), "replace");
    addFunc(new IOFunction(IOFunction::PressType), "press");
    addFunc(new IOFunction(IOFunction::ShowMessageType), "showMessage");
    addFunc(new IOFunction(IOFunction::GetInputType), "getInput");
    addFunc(new IOFunction(IOFunction::ReadFileType), "readFile");
    addFunc(new IOFunction(IOFunction::RunType), "run");
    addFunc(new IOFunction(IOFunction::RunDetachedType), "runDetached");
    addFunc(new SpecialFunction(SpecialFunction::NewVarType), "newVar");
    addFunc(new SpecialFunction(SpecialFunction::NewLocalVarType), "newLocalVar");
    addFunc(new SpecialFunction(SpecialFunction::NewGlobalVarType), "newGlobalVar");
    addFunc(new SpecialFunction(SpecialFunction::TryNewVarType), "tryNewVar");
    addFunc(new SpecialFunction(SpecialFunction::TryNewLocalVarType), "tryNewLocalVar");
    addFunc(new SpecialFunction(SpecialFunction::TryNewGlobalVarType), "tryNewGlobalVar");
    addFunc(new SpecialFunction(SpecialFunction::NewArrayType), "newArray");
    addFunc(new SpecialFunction(SpecialFunction::NewLocalArrayType), "newLocalArray");
    addFunc(new SpecialFunction(SpecialFunction::NewGlobalArrayType), "newGlobalArray");
    addFunc(new SpecialFunction(SpecialFunction::TryNewArrayType), "tryNewArray");
    addFunc(new SpecialFunction(SpecialFunction::TryNewLocalArrayType), "tryNewLocalArray");
    addFunc(new SpecialFunction(SpecialFunction::TryNewGlobalArrayType), "tryNewGlobalArray");
    addFunc(new SpecialFunction(SpecialFunction::NewFuncType), "newFunc");
    addFunc(new SpecialFunction(SpecialFunction::NewLocalFuncType), "newLocalFunc");
    addFunc(new SpecialFunction(SpecialFunction::NewGlobalFuncType), "newGlobalFunc");
    addFunc(new SpecialFunction(SpecialFunction::TryNewFuncType), "tryNewFunc");
    addFunc(new SpecialFunction(SpecialFunction::TryNewLocalFuncType), "tryNewLocalFunc");
    addFunc(new SpecialFunction(SpecialFunction::TryNewGlobalFuncType), "tryNewGlobalFunc");
    addFunc(new SpecialFunction(SpecialFunction::SetType), "set");
    addFunc(new SpecialFunction(SpecialFunction::RenewFuncType), "renewFunc");
    addFunc(new SpecialFunction(SpecialFunction::DeleteType), "delete");
    addFunc(new SpecialFunction(SpecialFunction::TrySetType), "trySet");
    addFunc(new SpecialFunction(SpecialFunction::TryRenewFuncType), "tryRenewFunc");
    addFunc(new SpecialFunction(SpecialFunction::TryDeleteType), "tryDelete");
    addFunc(new SpecialFunction(SpecialFunction::IsDefinedType), "isDefined");
    addFunc(new GeneralFunction(GeneralFunction::IsEmptyType), "isEmpty");
    addFunc(new GeneralFunction(GeneralFunction::ToIntegerType), "toInteger");
    addFunc(new GeneralFunction(GeneralFunction::ToRealType), "toReal");
    addFunc(new GeneralFunction(GeneralFunction::ToStringType), "toString");
    addFunc(new GeneralFunction(GeneralFunction::FormatType), "format");
    addFunc(new GeneralFunction(GeneralFunction::IfType), "if");
    addFunc(new GeneralFunction(GeneralFunction::WaitType), "wait");
    addFunc(new GeneralFunction(GeneralFunction::ForType), "for");
    addFunc(new GeneralFunction(GeneralFunction::WhileType), "while");
    addFunc(new GeneralFunction(GeneralFunction::DoWhileType), "doWhile");
    addFunc(new GeneralFunction(GeneralFunction::UntilType), "until");
    addFunc(new GeneralFunction(GeneralFunction::DoUntilType), "doUntil");
}

void PretexBuiltinFunction::cleanup()
{
    foreach (PretexBuiltinFunction *f, mlist)
        delete f;
    mmap.clear();
    mlist.clear();
}

/*============================== Public constructors =======================*/

PretexBuiltinFunction::PretexBuiltinFunction()
{
    //
}

/*============================== Public methods ============================*/

bool PretexBuiltinFunction::execute(ExecutionStack *stack, Function_TokenData *f, QString *err)
{
    if (!standardCheck(f, err))
        return false;
    QList<PretexVariant> oblArgs;
    foreach (int i, bRangeD(0, f->obligatoryArgumentCount() - 1))
    {
        bool b = false;
        PretexVariant a = ExecutionModule::executeSubprogram(stack, f->obligatoryArgument(i), &b, err);
        if (!b)
            return false;
        oblArgs << a;
    }
    QList<PretexVariant> optArgs;
    foreach (int i, bRangeD(0, f->optionalArgumentCount() - 1))
    {
        bool b = false;
        PretexVariant a = ExecutionModule::executeSubprogram(stack, f->optionalArgument(i), &b, err);
        if (!b)
            return false;
        optArgs << a;
    }
    ExecutionStack s(0, oblArgs, optArgs, name(), stack, functionFlags(name()));
    if (!execute(&s, err))
        return false;
    stack->setReturnValue(s.returnValue());
    return bRet(err, QString(), true);
}

int PretexBuiltinFunction::maxArgCount() const
{
    int opt = optionalArgumentCount();
    return (opt >= 0) ? (obligatoryArgumentCount() + opt) : -1;
}

/*============================== Static private methods ====================*/

void PretexBuiltinFunction::addFunc(PretexBuiltinFunction *f, const QString &name1, const QString &name2)
{
    //No checks to gain maximum speed
    mmap.insert(name1, f);
    if (!name2.isEmpty())
        mmap.insert(name2, f);
    mlist << f;
}

/*============================== Protected methods =========================*/

bool PretexBuiltinFunction::standardCheck(Function_TokenData *f, QString *err) const
{
    int oblArgCount = obligatoryArgumentCount();
    if (f->obligatoryArgumentCount() != oblArgCount)
        return bRet(err, tr("Argument count mismatch:", "error") + " " + name(), false);
    int optArgCount = optionalArgumentCount();
    if (optArgCount >= 0 && f->optionalArgumentCount() > optArgCount)
        return bRet(err, tr("Argument count mismatch:", "error") + " " + name(), false);
    return bRet(err, QString(), true);
}

QMap<QString, PretexBuiltinFunction *> PretexBuiltinFunction::mmap = QMap<QString, PretexBuiltinFunction *>();
QList<PretexBuiltinFunction *> PretexBuiltinFunction::mlist = QList<PretexBuiltinFunction *>();
