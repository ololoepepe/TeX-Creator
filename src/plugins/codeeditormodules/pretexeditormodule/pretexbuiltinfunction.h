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

#ifndef PRETEXBUILTINFUNCTION_H
#define PRETEXBUILTINFUNCTION_H

class ExecutionContext;
class Function_TokenData;

class QStringList;

#include "pretexvariant.h"

#include <QCoreApplication>
#include <QFlags>
#include <QList>
#include <QMap>
#include <QString>

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
private:
    static QList<PretexBuiltinFunction *> mlist;
    static QMap<QString, PretexBuiltinFunction *> mmap;
public:
    explicit PretexBuiltinFunction();
public:
    static void cleanup();
    static QStringList funcNames();
    static PretexBuiltinFunction *functionForName(const QString &name);
    static void init();
    static bool isBuiltinFunction(const QString &name);
    static QStringList normalFuncNames();
    static QStringList specFuncNames();
public:
    virtual SpecialFlags acceptedFlags() const;
    virtual bool execute(ExecutionContext *context, Function_TokenData *f, QString *err = 0);
    virtual SpecialFlags flagsPropagateMask() const;
    int maxArgCount() const;
    virtual QString name() const = 0;
    virtual int obligatoryArgumentCount() const = 0;
    virtual int optionalArgumentCount() const = 0;
protected:
    virtual bool execute(ExecutionContext *context, QString *err = 0) = 0;
    bool standardCheck(Function_TokenData *f, QString *err = 0) const;
private:
    static inline void addFunc(PretexBuiltinFunction *f, const QString &name1, const QString &name2 = QString());
};

#endif // PRETEXBUILTINFUNCTION_H
