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

#include "executionstack.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "pretexarray.h"
#include "pretexfunction.h"

#include <BeQt>
#include <BeQtGlobal>

#include <QMap>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QVariantMap>
#include <QVariantList>
#include <QVariant>

#include <QDebug>

/*============================================================================
================================ ExecutionStack ==============================
============================================================================*/

/*============================== Public constructors =======================*/

ExecutionStack::ExecutionStack(ExecutionStack *parent)
{
    mparent = parent;
    mdocument = 0;
    mtoken = 0;
    maccepedFlags = NoFlag;
    mflag = NoFlag;
}

ExecutionStack::ExecutionStack(BAbstractCodeEditorDocument *document, ExecutionStack *parent)
{
    mparent = parent;
    mdocument = document;
    mtoken = 0;
    maccepedFlags = NoFlag;
    mflag = NoFlag;
}

ExecutionStack::ExecutionStack(Token *token, const QList<PretexVariant> &obligatoryArguments,
                               const QList<PretexVariant> &optionalArguments, const QString &caller,
                               ExecutionStack *parent, SpecialFlags acceptedFlags)
{
    mparent = parent;
    mdocument = 0;
    mtoken = token;
    mobligArgs = obligatoryArguments;
    moptArgs = optionalArguments;
    maccepedFlags = acceptedFlags;
    if (parent)
        maccepedFlags |= parent->acceptedFlags();
    mflag = NoFlag;
    mcaller = caller;
}

ExecutionStack::ExecutionStack(Token *token, const QList<PretexVariant> &obligatoryArguments,
                               const QList<PretexVariant> &optionalArguments, const QList<Token> &specialArgs,
                               const QString &caller, ExecutionStack *parent, SpecialFlags acceptedFlags)
{
    mparent = parent;
    mdocument = 0;
    mtoken = token;
    mobligArgs = obligatoryArguments;
    moptArgs = optionalArguments;
    mspecialArgs = specialArgs;
    maccepedFlags = acceptedFlags;
    if (parent)
        maccepedFlags |= parent->acceptedFlags();
    mflag = NoFlag;
    mcaller = caller;
}

/*============================== Public methods ============================*/

bool ExecutionStack::declareVar(bool global, const QString &name, const PretexVariant &value, QString *err)
{
    if (name.isEmpty())
    {
        //TODO
        return false;
    }
    NameType t = UnknownName;
    if (isNameOccupied(name, global, &t))
    {
        //TODO
        return false;
    }
    if (global && mparent)
        return mparent->declareVar(global, name, value, err);
    mvars.insert(name, value);
    return bRet(err, QString(), true);
}

bool ExecutionStack::declareArray(bool global, const QString &name, const PretexArray::Dimensions &dimensions,
                                  QString *err)
{
    if (name.isEmpty())
    {
        //TODO
        return false;
    }
    NameType t = UnknownName;
    if (isNameOccupied(name, global, &t))
    {
        //TODO
        return false;
    }
    if (global && mparent)
        return mparent->declareArray(global, name, dimensions, err);
    PretexArray a(dimensions);
    //TODO
    marrays.insert(name, a);
    return bRet(err, QString(), true);
}

bool ExecutionStack::declareFunc(bool global, const QString &name, int obligatoryArgumentCount,
                                 int optionalAgrumentCount, const Token &body, QString *err)
{
    if (name.isEmpty())
    {
        //TODO
        return false;
    }
    NameType t = UnknownName;
    if (isNameOccupied(name, global, &t))
    {
        //TODO
        return false;
    }
    if (global && mparent)
        return mparent->declareFunc(global, name, obligatoryArgumentCount, optionalAgrumentCount, body, err);
    PretexFunction f(name, obligatoryArgumentCount, optionalAgrumentCount, body);
    //TODO
    mfuncs.insert(name, f);
    return bRet(err, QString(), true);
}

bool ExecutionStack::isNameOccupied(const QString &name, bool global, NameType *t) const
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

ExecutionStack *ExecutionStack::parent() const
{
    return mparent;
}

bool ExecutionStack::setVar(const QString &name, const PretexVariant &value, QString *err)
{
    if (name.isEmpty())
    {
        //TODO
        return false;
    }
    NameType t = UnknownName;
    if (!isNameOccupied(name, true, &t))
    {
        //TODO
        return false;
    }
    if (VariableName != t)
    {
        //TODO
        return false;
    }
    if (!mvars.contains(name))
        return mparent->setVar(name, value, err);
    mvars[name] = value;
    return bRet(err, QString(), true);
}

bool ExecutionStack::setArrayElement(const QString &name, const PretexArray::Indexes &indexes,
                                     const PretexVariant &value, QString *err)
{
    if (name.isEmpty())
    {
        //TODO
        return false;
    }
    NameType t = UnknownName;
    if (!isNameOccupied(name, true, &t))
    {
        //TODO
        return false;
    }
    if (ArrayName != t)
    {
        //TODO
        return false;
    }
    if (!marrays.contains(name))
        return mparent->setArrayElement(name, indexes, value, err);
    //TODO
    marrays[name][indexes] = value;
    return bRet(err, QString(), true);
}

bool ExecutionStack::setFunc(const QString &name, const Token &body, QString *err)
{
    if (name.isEmpty())
    {
        //TODO
        return false;
    }
    NameType t = UnknownName;
    if (!isNameOccupied(name, true, &t))
    {
        //TODO
        return false;
    }
    if (UserFunctionName != t)
    {
        //TODO
        return false;
    }
    if (!mfuncs.contains(name))
        return mparent->setFunc(name, body, err);
    mfuncs[name].setBody(body);
    return bRet(err, QString(), true);
}

bool ExecutionStack::undeclare(const QString &name, QString *err)
{
    if (name.isEmpty())
    {
        //TODO
        return false;
    }
    NameType t = UnknownName;
    if (!isNameOccupied(name, true, &t))
    {
        //TODO
        return false;
    }
    if (!mvars.contains(name) && !marrays.contains(name) && !mfuncs.contains(name))
        return mparent->undeclare(name, err);
    switch (t)
    {
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
        //TODO
        return false;
    }
    return bRet(err, QString(), true);
}

QByteArray ExecutionStack::saveState() const
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

void ExecutionStack::restoreState(const QByteArray &state)
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

void ExecutionStack::clear()
{
    mvars.clear();
    marrays.clear();
    mfuncs.clear();
}

PretexVariant ExecutionStack::variable(const QString &name) const
{
    if (mvars.contains(name))
        return mvars.value(name);
    return mparent ? mparent->variable(name) : PretexVariant();
}

PretexArray::Dimensions ExecutionStack::arrayDimensions(const QString &name) const
{
    if (marrays.contains(name))
        return marrays.value(name).dimensions();
    return mparent ? mparent->arrayDimensions(name) : PretexArray::Dimensions();
}

PretexVariant ExecutionStack::arrayElement(const QString &name, const PretexArray::Indexes &indexes) const
{
    if (marrays.contains(name))
        return marrays.value(name).value(indexes);
    return mparent ? mparent->arrayElement(name, indexes) : PretexVariant();
}

PretexFunction *ExecutionStack::function(const QString &name) const
{
    if (mfuncs.contains(name))
        return &const_cast<ExecutionStack *>(this)->mfuncs[name];
    return mparent ? mparent->function(name) : 0;
}

BAbstractCodeEditorDocument *ExecutionStack::doc() const
{
    if (mdocument)
        return mdocument;
    return mparent ? mparent->doc() : 0;
}

Token *ExecutionStack::token() const
{
    return mtoken;
}

const QList<PretexVariant> &ExecutionStack::obligArgs() const
{
    return mobligArgs;
}

const QList<PretexVariant> &ExecutionStack::optArgs() const
{
    return moptArgs;
}

const QList<Token> &ExecutionStack::specialArgs() const
{
    return mspecialArgs;
}

PretexVariant ExecutionStack::obligArg(int index) const
{
    if (index < 0 || index >= mobligArgs.size())
        return PretexVariant();
    return mobligArgs.at(index);
}

PretexVariant ExecutionStack::optArg(int index) const
{
    if (index < 0 || index >= moptArgs.size())
        return PretexVariant();
    return moptArgs.at(index);
}

Token ExecutionStack::specialArg(int index) const
{
    if (index < 0 || index >= mspecialArgs.size())
        return Token();
    return mspecialArgs.at(index);
}

int ExecutionStack::obligArgCount() const
{
    return mobligArgs.size();
}

int ExecutionStack::optArgCount() const
{
    return moptArgs.size();
}

int ExecutionStack::specialArgCount() const
{
    return mspecialArgs.size();
}

QString ExecutionStack::caller() const
{
    if (!mcaller.isEmpty())
        return mcaller;
    return mparent ? mparent->caller() : QString();
}

int ExecutionStack::maxArgCount() const
{
    QString c = caller();
    if (c.isEmpty())
        return 0;
    NameType t;
    if (!isNameOccupied(c, true, &t))
        return 0;
    switch (t)
    {
    case VariableName:
        return 0;
    case ArrayName:
        return arrayDimensions(c).size();
    case UserFunctionName:
        return function(c)->maxArgCount();
    case BuiltinFunctionName:
        return PretexBuiltinFunction::functionForName(c)->maxArgCount();
    default:
        return 0;
    }
}

void ExecutionStack::setReturnValue(const PretexVariant &v)
{
    mretVal = v;
}

void ExecutionStack::setReturnValue(const QString &s)
{
    mretVal = PretexVariant(s);
}

void ExecutionStack::setReturnValue(int i)
{
    mretVal = PretexVariant(i);
}

void ExecutionStack::setReturnValue(double d)
{
    mretVal = PretexVariant(d);
}

PretexVariant ExecutionStack::returnValue() const
{
    return mretVal;
}

ExecutionStack::SpecialFlags ExecutionStack::acceptedFlags() const
{
    return maccepedFlags;
}

bool ExecutionStack::isFlagAccepted(SpecialFlag flag)
{
    return (flag & maccepedFlags);
}

bool ExecutionStack::setFlag(SpecialFlag flag, QString *err)
{
    if (!isFlagAccepted(flag))
    {
        switch (flag)
        {
        case ReturnFlag:
            return bRet(err, tr("Can not use \"return\" outside function body", "error"), false);
        case BreakFlag:
            return bRet(err, tr("Can not use \"break\" outside loop", "error"), false);
        case ContinueFlag:
            return bRet(err, tr("Can not use \"continue\" outside loop", "error"), false);
        default:
            return bRet(err, tr("Unknown internal error (special flag)", "error"), false);
        }
    }
    mflag = flag;
    return bRet(err, QString(), true);
}