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

#ifndef PRETEXBUILTINFUNCTION_H
#define PRETEXBUILTINFUNCTION_H

class ExecutionStack;

#include "pretexvariant.h"

#include <QStringList>
#include <QList>
#include <QMap>
#include <QString>

/*============================================================================
================================ PretexBuiltinFunction =======================
============================================================================*/

class PretexBuiltinFunction
{
public:
    static PretexBuiltinFunction *functionForName(const QString &name);
    static bool isBuiltinFunction(const QString &name);
    static QStringList specFuncNames();
    static QStringList normalFuncNames();
    static QStringList funcNames();
    static void init();
    static void cleanup();
public:
    explicit PretexBuiltinFunction();
public:
    virtual QString name() const = 0;
    virtual int obligatoryArgumentCount() const = 0;
    virtual int optionalArgumentCount() const = 0;
    virtual bool execute(ExecutionStack *stack, const QList<PretexVariant> &obligatoryArguments,
                         const QList<PretexVariant> &optionalArguments, PretexVariant &result, QString *err = 0) = 0;
private:
    static inline void addFunc(PretexBuiltinFunction *f, const QString &name1, const QString &name2 = QString());
private:
    static QMap<QString, PretexBuiltinFunction *> mmap;
    static QList<PretexBuiltinFunction *> mlist;
};

#endif // PRETEXBUILTINFUNCTION_H
