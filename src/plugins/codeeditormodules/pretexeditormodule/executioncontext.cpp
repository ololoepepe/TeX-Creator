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

#include "executioncontext.h"

#include "pretexarray.h"
#include "pretexbuiltinfunction.h"
#include "pretexfunction.h"
#include "pretexvariant.h"

#include <BeQt>

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QIODevice>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

/*============================================================================
================================ ExecutionContext ============================
============================================================================*/

/*============================== Public constructors =======================*/

ExecutionContext::ExecutionContext(PretexEditorModule *editorModule) :
    EditorModule(editorModule)
{
    mparent = 0;
    mdocument = 0;
    mflag = PretexBuiltinFunction::NoFlag;
}

ExecutionContext::ExecutionContext(ExecutionContext *parent) :
    EditorModule(0)
{
    mparent = parent;
    mdocument = 0;
    mflag = PretexBuiltinFunction::NoFlag;
}

ExecutionContext::ExecutionContext(BAbstractCodeEditorDocument *document, ExecutionContext *parent) :
    EditorModule(0)
{
    mparent = parent;
    mdocument = document;
    mflag = PretexBuiltinFunction::NoFlag;
}

ExecutionContext::ExecutionContext(const QList<PretexVariant> &obligatoryArguments,
                                   const QList<PretexVariant> &optionalArguments, const QString &caller,
                                   ExecutionContext *parent) :
    EditorModule(0)
{
    mparent = parent;
    mdocument = 0;
    mobligArgs = obligatoryArguments;
    moptArgs = optionalArguments;
    mflag = PretexBuiltinFunction::NoFlag;
    mcaller = caller;
}

ExecutionContext::ExecutionContext(const QList<PretexVariant> &obligatoryArguments,
                                   const QList<PretexVariant> &optionalArguments, const QList<Token> &specialArgs,
                                   const QString &caller, ExecutionContext *parent) :
    EditorModule(0)
{
    mparent = parent;
    mdocument = 0;
    mobligArgs = obligatoryArguments;
    moptArgs = optionalArguments;
    mspecialArgs = specialArgs;
    mflag = PretexBuiltinFunction::NoFlag;
    mcaller = caller;
}

/*============================== Public methods ============================*/

PretexArray::Dimensions ExecutionContext::arrayDimensions(const QString &name) const
{
    if (marrays.contains(name))
        return marrays.value(name).dimensions();
    return mparent ? mparent->arrayDimensions(name) : PretexArray::Dimensions();
}

PretexVariant ExecutionContext::arrayElement(const QString &name, const PretexArray::Indexes &indexes) const
{
    if (marrays.contains(name))
        return marrays.value(name).value(indexes);
    return mparent ? mparent->arrayElement(name, indexes) : PretexVariant();
}

QString ExecutionContext::caller() const
{
    if (!mcaller.isEmpty())
        return mcaller;
    return mparent ? mparent->caller() : QString();
}

void ExecutionContext::clear()
{
    mvars.clear();
    marrays.clear();
    mfuncs.clear();
}

bool ExecutionContext::declareArray(bool global, const QString &name, const PretexArray::Dimensions &dimensions,
                                  QString *err)
{
    if (name.isEmpty())
        return bRet(err, tr("Empty array name", "error"), false);
    NameType t = UnknownName;
    if (isNameOccupied(name, global, &t))
        return bRet(err, tr("Identifier is occupied", "error"), false);
    if (global && mparent)
        return mparent->declareArray(global, name, dimensions, err);
    PretexArray a(dimensions);
    if (!a.isValid())
        return bRet(err, tr("Invalid array", "error"), false);
    marrays.insert(name, a);
    return bRet(err, QString(), true);
}

bool ExecutionContext::declareFunc(bool global, const QString &name, int obligatoryArgumentCount,
                                 int optionalAgrumentCount, const Token &body, QString *err)
{
    if (name.isEmpty())
        return bRet(err, tr("Empty function name", "error"), false);
    NameType t = UnknownName;
    if (isNameOccupied(name, global, &t))
        return bRet(err, tr("Identifier is occupied", "error"), false);
    if (global && mparent)
        return mparent->declareFunc(global, name, obligatoryArgumentCount, optionalAgrumentCount, body, err);
    PretexFunction f(name, obligatoryArgumentCount, optionalAgrumentCount, body);
    if (!f.isValid())
        return bRet(err, tr("Invalid function", "error"), false);
    mfuncs.insert(name, f);
    return bRet(err, QString(), true);
}

bool ExecutionContext::declareVar(bool global, const QString &name, const PretexVariant &value, QString *err)
{
    if (name.isEmpty())
        return bRet(err, tr("Empty variable name", "error"), false);
    NameType t = UnknownName;
    if (isNameOccupied(name, global, &t))
        return bRet(err, tr("Identifier is occupied", "error"), false);
    if (global && mparent)
        return mparent->declareVar(global, name, value, err);
    mvars.insert(name, value);
    return bRet(err, QString(), true);
}

BAbstractCodeEditorDocument *ExecutionContext::doc() const
{
    if (mdocument)
        return mdocument;
    return mparent ? mparent->doc() : 0;
}

PretexEditorModule *ExecutionContext::editorModule() const
{
    if (EditorModule)
        return EditorModule;
    return mparent ? mparent->editorModule() : 0;
}

PretexBuiltinFunction::SpecialFlag ExecutionContext::flag() const
{
    return mflag;
}

PretexFunction *ExecutionContext::function(const QString &name) const
{
    if (mfuncs.contains(name))
        return &const_cast<ExecutionContext *>(this)->mfuncs[name];
    return mparent ? mparent->function(name) : 0;
}

bool ExecutionContext::isNameOccupied(const QString &name, bool global, NameType *t) const
{
    if (name.isEmpty())
        return bRet(t, UnknownName, false);
    if (PretexBuiltinFunction::isBuiltinFunction(name))
        return bRet(t, BuiltinFunctionName, true);
    if (mvars.contains(name))
        return bRet(t, VariableName, true);
    if (marrays.contains(name))
        return bRet(t, ArrayName, true);
    if (mfuncs.contains(name))
        return bRet(t, UserFunctionName, true);
    return (global && mparent) ? mparent->isNameOccupied(name, global, t) : bRet(t, UnknownName, false);
}

int ExecutionContext::maxArgCount() const
{
    QString c = caller();
    if (c.isEmpty())
        return 0;
    NameType t;
    if (!isNameOccupied(c, true, &t))
        return 0;
    if (UserFunctionName == t)
        return function(c)->maxArgCount();
    return mparent ? mparent->maxArgCount() : 0;
}

PretexVariant ExecutionContext::obligArg(int index) const
{
    if (index < 0 || index >= mobligArgs.size())
        return PretexVariant();
    return mobligArgs.at(index);
}

int ExecutionContext::obligArgCount() const
{
    return mobligArgs.size();
}

const QList<PretexVariant> &ExecutionContext::obligArgs() const
{
    return mobligArgs;
}

PretexVariant ExecutionContext::optArg(int index) const
{
    if (index < 0 || index >= moptArgs.size())
        return PretexVariant();
    return moptArgs.at(index);
}

int ExecutionContext::optArgCount() const
{
    return moptArgs.size();
}

const QList<PretexVariant> &ExecutionContext::optArgs() const
{
    return moptArgs;
}

ExecutionContext *ExecutionContext::parent() const
{
    return mparent;
}

void ExecutionContext::restoreState(const QByteArray &state)
{
    clear();
    QVariantMap m = BeQt::deserialize(state).toMap();
    QVariantMap mm = m.value("variables").toMap();
    foreach (const QString &key, mm.keys())
        mvars.insert(key, mm.value(key).value<PretexVariant>());
    mm = m.value("arrays").toMap();
    foreach (const QString &key, mm.keys())
        marrays.insert(key, mm.value(key).value<PretexArray>());
    mm = m.value("functions").toMap();
    foreach (const QString &key, mm.keys())
        mfuncs.insert(key, mm.value(key).value<PretexFunction>());
}

PretexVariant ExecutionContext::returnValue() const
{
    return mretVal;
}

QByteArray ExecutionContext::saveState() const
{
    QVariantMap m;
    QVariantMap mm;
    foreach (const QString &key, mvars.keys())
        mm.insert(key, QVariant::fromValue(mvars.value(key)));
    m.insert("variables", mm);
    mm.clear();
    foreach (const QString &key, marrays.keys())
        mm.insert(key, QVariant::fromValue(marrays.value(key)));
    m.insert("arrays", mm);
    mm.clear();
    foreach (const QString &key, mfuncs.keys())
        mm.insert(key, QVariant::fromValue(mfuncs.value(key)));
    m.insert("functions", mm);
    return BeQt::serialize(m);
}

bool ExecutionContext::setArrayElement(const QString &name, const PretexArray::Indexes &indexes,
                                     const PretexVariant &value, QString *err)
{
    if (name.isEmpty())
        return bRet(err, tr("Empty array name", "error"), false);
    NameType t = UnknownName;
    if (!isNameOccupied(name, true, &t))
        return bRet(err, tr("No such array", "error"), false);
    if (ArrayName != t)
        return bRet(err, tr("Identifier is not an array", "error"), false);
    if (!marrays.contains(name))
        return mparent->setArrayElement(name, indexes, value, err);
    if (!marrays[name].areIndexesValid(indexes))
        return bRet(err, tr("Invalid indexes", "error"), false);
    marrays[name][indexes] = value;
    return bRet(err, QString(), true);
}

bool ExecutionContext::setFlag(PretexBuiltinFunction::SpecialFlag flag, QString *err)
{
    bool prop = false;
    if (!isFlagAccepted(flag, &prop)) {
        switch (flag) {
        case PretexBuiltinFunction::ReturnFlag:
            return bRet(err, tr("Can not use \"return\" outside function body", "error"), false);
        case PretexBuiltinFunction::BreakFlag:
            return bRet(err, tr("Can not use \"break\" outside loop", "error"), false);
        case PretexBuiltinFunction::ContinueFlag:
            return bRet(err, tr("Can not use \"continue\" outside loop", "error"), false);
        default:
            return bRet(err, tr("Unknown internal error (special flag)", "error"), false);
        }
    }
    mflag = flag;
    if (prop && mparent)
        mparent->setFlag(flag);
    return bRet(err, QString(), true);
}

bool ExecutionContext::setFunc(const QString &name, const Token &body, QString *err)
{
    if (name.isEmpty())
        return bRet(err, tr("Empty function name", "error"), false);
    NameType t = UnknownName;
    if (!isNameOccupied(name, true, &t))
        return bRet(err, tr("No such function", "error"), false);
    if (UserFunctionName != t)
        return bRet(err, tr("Identifier is not a function", "error"), false);
    if (!mfuncs.contains(name))
        return mparent->setFunc(name, body, err);
    if (body.type() != Token::Subprogram_Token)
        return bRet(err, tr("Invalid function body", "error"), false);
    mfuncs[name].setBody(body);
    return bRet(err, QString(), true);
}

void ExecutionContext::setReturnValue(const PretexVariant &v)
{
    mretVal = v;
}

void ExecutionContext::setReturnValue(const QString &s)
{
    mretVal = PretexVariant(s);
}

void ExecutionContext::setReturnValue(int i)
{
    mretVal = PretexVariant(i);
}

void ExecutionContext::setReturnValue(double d)
{
    mretVal = PretexVariant(d);
}

bool ExecutionContext::setVar(const QString &name, const PretexVariant &value, QString *err)
{
    if (name.isEmpty())
        return bRet(err, tr("Empty variable name", "error"), false);
    NameType t = UnknownName;
    if (!isNameOccupied(name, true, &t))
        return bRet(err, tr("No such variable", "error"), false);
    if (VariableName != t)
        return bRet(err, tr("Identifier is not a variable", "error"), false);
    if (!mvars.contains(name))
        return mparent->setVar(name, value, err);
    mvars[name] = value;
    return bRet(err, QString(), true);
}

Token ExecutionContext::specialArg(int index) const
{
    if (index < 0 || index >= mspecialArgs.size())
        return Token();
    return mspecialArgs.at(index);
}

int ExecutionContext::specialArgCount() const
{
    return mspecialArgs.size();
}

const QList<Token> &ExecutionContext::specialArgs() const
{
    return mspecialArgs;
}

bool ExecutionContext::undeclare(const QString &name, QString *err)
{
    if (name.isEmpty())
        return bRet(err, tr("Empty identifier", "error"), false);
    NameType t = UnknownName;
    if (!isNameOccupied(name, true, &t))
        return bRet(err, tr("No such identifier", "error"), false);
    if (!mvars.contains(name) && !marrays.contains(name) && !mfuncs.contains(name))
        return mparent->undeclare(name, err);
    switch (t) {
    case VariableName:
        mvars.remove(name);
        break;
    case ArrayName:
        marrays.remove(name);
        break;
    case UserFunctionName:
        mfuncs.remove(name);
        break;
    case BuiltinFunctionName:
    default:
        return bRet(err, tr("Attempt to delete builtin function", "error"), false);
    }
    return bRet(err, QString(), true);
}

PretexVariant ExecutionContext::variable(const QString &name) const
{
    if (mvars.contains(name))
        return mvars.value(name);
    return mparent ? mparent->variable(name) : PretexVariant();
}

/*============================== Private methods ===========================*/

bool ExecutionContext::isFlagAccepted(PretexBuiltinFunction::SpecialFlag flag, bool *propagate) const
{
    if (PretexBuiltinFunction::ReturnFlag == flag) {
        NameType t = UnknownName;
        if (isNameOccupied(mcaller, true, &t) && UserFunctionName == t)
            return bRet(propagate, false, true);
    }
    PretexBuiltinFunction *f = PretexBuiltinFunction::functionForName(mcaller);
    if (f && (f->acceptedFlags() & flag))
        return bRet(propagate, false, true);
    if (mparent && f && (f->flagsPropagateMask() & flag)) {
        bool b = mparent->isFlagAccepted(flag);
        return bRet(propagate, b, b);
    }
    if (!f && PretexBuiltinFunction::ReturnFlag == flag)
        return bRet(propagate, false, true);
    return bRet(propagate, false, false);
}
