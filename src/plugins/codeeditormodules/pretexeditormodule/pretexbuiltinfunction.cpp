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

#include <BeQtGlobal>

#include <QString>
#include <QStringList>
#include <QMap>

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
    static const QStringList names = QStringList() << "set" << "renewFunc" << "delete"
    << "newVar" << "newLocalVar" << "newGlobalVar" << "tryNewVar" << "tryNewLocalVar" << "tryNewGlobalVar"
    << "newFunc" << "newLocalFunc" << "newGlobalFunc" << "tryNewFunc" << "tryNewLocalFunc" << "tryNewGlobalFunc"
    << "newArray" << "newLocalArray" << "newGlobalArray" << "tryNewArray" << "tryNewLocalArray" << "tryNewGlobalArray";
    return names;
}

QStringList PretexBuiltinFunction::normalFuncNames()
{
    static const QStringList names = QStringList() << "+" << "-" << "*" << "sum";
    return names;
}

QStringList PretexBuiltinFunction::funcNames()
{
    init_once(QStringList, names, QStringList())
        names << normalFuncNames() << specFuncNames();
    return names;
}

void PretexBuiltinFunction::init()
{
    addFunc(new MathFunction(MathFunction::SumType), "sum", "+");
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

/*============================== Static private methods ====================*/

void PretexBuiltinFunction::addFunc(PretexBuiltinFunction *f, const QString &name1, const QString &name2)
{
    //No checks to gain maximum speed
    mmap.insert(name1, f);
    if (!name2.isEmpty())
        mmap.insert(name2, f);
    mlist << f;
}

QMap<QString, PretexBuiltinFunction *> PretexBuiltinFunction::mmap = QMap<QString, PretexBuiltinFunction *>();
QList<PretexBuiltinFunction *> PretexBuiltinFunction::mlist = QList<PretexBuiltinFunction *>();
