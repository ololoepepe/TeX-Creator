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

#ifndef GENERALFUNCTION_H
#define GENERALFUNCTION_H

class ExecutionContext;
class Function_TokenData;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QCoreApplication>
#include <QList>

/*============================================================================
================================ GeneralFunction =============================
============================================================================*/

class GeneralFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(GeneralFunction)
public:
    enum Type
    {
        BreakType,
        ContinueType,
        DoUntilType,
        DoWhileType,
        FormatType,
        ForType,
        IfType,
        IsEmptyType,
        ReturnType,
        ToIntegerType,
        ToRealType,
        ToStringType,
        UntilType,
        WaitType,
        WhileType
    };
private:
    Type mtype;
public:
    explicit GeneralFunction(Type t);
public:
    SpecialFlags acceptedFlags() const;
    bool execute(ExecutionContext *context, Function_TokenData *f, QString *err = 0);
    SpecialFlags flagsPropagateMask() const;
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
protected:
    bool execute(ExecutionContext *context, QString *err = 0);
private:
    static bool doUntilLoop(ExecutionContext *context, QString *err = 0);
    static bool doWhileLoop(ExecutionContext *context, QString *err = 0);
    static bool flagFunction(ExecutionContext *context, SpecialFlag flag, QString *err = 0);
    static bool forLoop(ExecutionContext *context, QString *err = 0);
    static bool format(ExecutionContext *context, QString *err = 0);
    static bool ifCondition(ExecutionContext *context, QString *err = 0);
    static bool isEmpty(ExecutionContext *context, QString *err = 0);
    static bool predGeqD(const double &t1, const double &t2);
    static bool predGeqI(const int &t1, const int &t2);
    static bool predLeqD(const double &t1, const double &t2);
    static bool predLeqI(const int &t1, const int &t2);
    static bool toInteger(ExecutionContext *context, QString *err = 0);
    static bool toReal(ExecutionContext *context, QString *err = 0);
    static bool toString(ExecutionContext *context, QString *err = 0);
    static bool untilLoop(ExecutionContext *context, QString *err = 0);
    static bool waitFunction(ExecutionContext *context, QString *err = 0);
    static bool whileLoop(ExecutionContext *context, QString *err = 0);
private:
    Q_DISABLE_COPY(GeneralFunction)
};

#endif // GENERALFUNCTION_H
