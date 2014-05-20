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

class Function_TokenData;
class ExecutionStack;

#include "pretexvariant.h"

#include <QStringList>
#include <QList>
#include <QMap>
#include <QString>
#include <QCoreApplication>
#include <QFlags>

/*============================================================================
================================ PretexBuiltinFunction =======================
============================================================================*/

class PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(PretexBuiltinFunction)
public:
    enum SpecialFlag
    {
        NoFlag = 0x00,
        ReturnFlag = 0x01,
        BreakFlag = 0x02,
        ContinueFlag = 0x04
    };
    Q_DECLARE_FLAGS(SpecialFlags, SpecialFlag)
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
    virtual bool execute(ExecutionStack *stack, Function_TokenData *f, QString *err = 0);
    virtual SpecialFlags acceptedFlags() const;
    virtual SpecialFlags flagsPropagateMask() const;
    int maxArgCount() const;
protected:
    virtual bool execute(ExecutionStack *stack, QString *err = 0) = 0;
    bool standardCheck(Function_TokenData *f, QString *err = 0) const;
private:
    static inline void addFunc(PretexBuiltinFunction *f, const QString &name1, const QString &name2 = QString());
private:
    static QMap<QString, PretexBuiltinFunction *> mmap;
    static QList<PretexBuiltinFunction *> mlist;
};

#endif // PRETEXBUILTINFUNCTION_H
