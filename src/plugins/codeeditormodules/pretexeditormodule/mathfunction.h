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

#ifndef MATHFUNCTION_H
#define MATHFUNCTION_H

class ExecutionContext;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QCoreApplication>
#include <QList>

#include <cmath>

/*============================================================================
================================ MathFunction ================================
============================================================================*/

class MathFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(MathFunction)
public:
    enum Type
    {
        AbsType,
        AddType,
        DivideType,
        ExpType,
        FactorialType,
        LgType,
        LnType,
        LogType,
        ModuloType,
        MultiplyType,
        NegativeType,
        PowerType,
        RandomType,
        RootType,
        RoundType,
        SqrtType,
        SubtractType
    };
private:
    Type mtype;
public:
    explicit MathFunction(Type t);
public:
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
protected:
    bool execute(ExecutionContext *context, QString *err = 0);
private:
    template<typename T> static T anyLog(T base, T t)
    {
        return std::log(t) / std::log(base);
    }
    template<typename T> static T anyRoot(T base, T p)
    {
        static const T t1 = (T) 1;
        return std::pow(base, t1 / p);
    }
private:
    static bool abs(ExecutionContext *context, QString *err = 0);
    static bool add(ExecutionContext *context, QString *err = 0);
    static bool divide(ExecutionContext *context, QString *err = 0);
    static bool exp(ExecutionContext *context, QString *err = 0);
    static int fact(int i = 0);
    static bool factorial(ExecutionContext *context, QString *err = 0);
    static bool lg(ExecutionContext *context, QString *err = 0);
    static bool ln(ExecutionContext *context, QString *err = 0);
    static bool log(ExecutionContext *context, QString *err = 0);
    static bool modulo(ExecutionContext *context, QString *err = 0);
    static bool multiply(ExecutionContext *context, QString *err = 0);
    static bool negative(ExecutionContext *context, QString *err = 0);
    static bool power(ExecutionContext *context, QString *err = 0);
    static bool random(ExecutionContext *context, QString *err = 0);
    static bool root(ExecutionContext *context, QString *err = 0);
    static bool round(ExecutionContext *context, QString *err = 0);
    static bool sqrt(ExecutionContext *context, QString *err = 0);
    static bool subtract(ExecutionContext *context, QString *err = 0);
private:
    Q_DISABLE_COPY(MathFunction)
};

#endif // MATHFUNCTION_H
