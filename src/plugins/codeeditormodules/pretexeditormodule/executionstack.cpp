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

#include <BeQt>
#include <BeQtGlobal>

#include <QMap>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#include <QDebug>

/*============================================================================
================================ ExecutionStack ==============================
============================================================================*/

/*============================== Public constructors =======================*/

ExecutionStack::ExecutionStack(ExecutionStack *parent)
{
    mparent = parent;
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
    if (isNameOccupied(name, &t))
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
    if (isNameOccupied(name, &t))
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
                                 int optionalAgrumentCount, const QList<PretexStatement> &body, QString *err)
{
    if (name.isEmpty())
    {
        //TODO
        return false;
    }
    NameType t = UnknownName;
    if (isNameOccupied(name, &t))
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

bool ExecutionStack::isNameOccupied(const QString &name, NameType *t) const
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
    return mparent ? mparent->isNameOccupied(name, t) : bRet(t, UnknownName, false);
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
    if (!isNameOccupied(name, &t))
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
    if (!isNameOccupied(name, &t))
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

bool ExecutionStack::setFunc(const QString &name, const QList<PretexStatement> &body, QString *err)
{
    if (name.isEmpty())
    {
        //TODO
        return false;
    }
    NameType t = UnknownName;
    if (!isNameOccupied(name, &t))
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
    if (!isNameOccupied(name, &t))
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







bool ExecutionStack::define(const QString &id, const QString &value, bool global)
{
    if (id.isEmpty() || isDefined(id))
        return false;
    if (global && mparent)
        return mparent->define(id, value);
    mmap.insert(id, value);
    return true;
}

bool ExecutionStack::defineF(const QString &id, const QString &value, bool global)
{
    if (id.isEmpty() || isDefined(id))
        return false;
    if (global && mparent)
        return mparent->defineF(id, value);
    mmapF.insert(id, value);
    return true;
}

bool ExecutionStack::undefine(const QString &id)
{
    if (id.isEmpty())
        return false;
    if (mmap.contains(id))
    {
        mmap.remove(id);
        return true;
    }
    else if (mmapF.contains(id))
    {
        mmapF.remove(id);
        return true;
    }
    else if (mparent)
    {
        return mparent->undefine(id);
    }
    else
    {
        return false;
    }
}

bool ExecutionStack::set(const QString &id, const QString &value)
{
    if (id.isEmpty())
        return false;
    if (mmap.contains(id))
    {
        mmap[id] = value;
        return true;
    }
    else if (mparent)
    {
        return mparent->set(id, value);
    }
    else
    {
        return false;
    }
}

bool ExecutionStack::setF(const QString &id, const QString &value)
{
    if (id.isEmpty())
        return false;
    if (mmapF.contains(id))
    {
        mmapF[id] = value;
        return true;
    }
    else if (mparent)
    {
        return mparent->setF(id, value);
    }
    else
    {
        return false;
    }
}

bool ExecutionStack::get(const QString &id, QString &value) const
{
    if (id.isEmpty())
        return false;
    if (mmap.contains(id))
    {
        value = mmap.value(id);
        return true;
    }
    else if (mparent)
    {
        return mparent->get(id, value);
    }
    else
    {
        return false;
    }
}

bool ExecutionStack::getF(const QString &id, QString &value) const
{
    if (id.isEmpty())
        return false;
    if (mmapF.contains(id))
    {
        value = mmapF.value(id);
        return true;
    }
    else if (mparent)
    {
        return mparent->getF(id, value);
    }
    else
    {
        return false;
    }
}

bool ExecutionStack::isDefined(const QString &id) const
{
    if (id.isEmpty())
        return false;
    return mmap.contains(id) || mmapF.contains(id) || (mparent && mparent->isDefined(id));
}

QByteArray ExecutionStack::save() const
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(BeQt::DataStreamVersion);
    out << mmap;
    out << mmapF;
    return data;
}

void ExecutionStack::restore(const QByteArray &data)
{
    QDataStream in(data);
    in.setVersion(BeQt::DataStreamVersion);
    in >> mmap;
    in >> mmapF;
}

void ExecutionStack::clear()
{
    mmap.clear();
    mmapF.clear();
}
