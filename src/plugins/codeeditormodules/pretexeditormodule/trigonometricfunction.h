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

#ifndef TRIGONOMETRICFUNCTION_H
#define TRIGONOMETRICFUNCTION_H

class ExecutionStack;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QCoreApplication>
#include <QList>

#include <cmath>

/*============================================================================
================================ TrigonometricFunction =======================
============================================================================*/

class TrigonometricFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(TrigonometricFunction)
public:
    enum Type
    {
        AcosType,
        AcotType,
        AcscType,
        ArchType,
        ArcschType,
        ArcthType,
        ArschType,
        ArshType,
        ArthType,
        AsecType,
        AsinType,
        AtanType,
        ChType,
        CosType,
        CotType,
        CschType,
        CscType,
        CthType,
        SechType,
        SecType,
        ShType,
        SinType,
        TanType,
        ThType
    };
private:
    Type mtype;
public:
    explicit TrigonometricFunction(Type t);
public:
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
protected:
    bool execute(ExecutionStack *stack, QString *err = 0);
private:
    template<typename T> static double ctan(T t)
    {
        return 1.0 / std::tan(t);
    }
    template<typename T> static double sec(T t)
    {
        return 1.0 / std::sin(t);
    }
    template<typename T> static double csc(T t)
    {
        return 1.0 / std::cos(t);
    }
    template<typename T> static double acot(T t)
    {
        if (t >= 0.0)
            return std::asin(1.0 / std::sqrt(1.0 + std::pow(t, 2.0)));
        else
            return std::atan(1.0) * 4.0 - std::asin(1.0 / std::sqrt(1.0 + std::pow(t, 2.0)));
    }
    template<typename T> static double asec(T t)
    {
        return std::acos(1.0 / t);
    }
    template<typename T> static double acsc(T t)
    {
        return std::asin(1.0 / t);
    }
    template<typename T> static double cth(T t)
    {
        return 1.0 / std::tanh(t);
    }
    template<typename T> static double sech(T t)
    {
        return 1.0 / std::sinh(t);
    }
    template<typename T> static double csch(T t)
    {
        return 1.0 / std::cosh(t);
    }
    template<typename T> static double arsh(T t)
    {
        return std::log(t + std::sqrt(std::pow(t, 2.0) + 1.0));
    }
    template<typename T> static double arch(T t)
    {
        return std::log(t + std::sqrt(std::pow(t, 2.0) - 1.0));
    }
    template<typename T> static double arth(T t)
    {
        return 0.5 * std::log((1.0 + t) / (1.0 - t));
    }
    template<typename T> static double arcth(T t)
    {
        return 0.5 * std::log((t + 1.0) / (t - 1.0));
    }
    template<typename T> static double arsch(T t)
    {
        return 1.0 / std::sinh(t);
    }
    template<typename T> static double arcsch(T t)
    {
        return 1.0 / std::cosh(t);
    }
private:
    static bool unaryAcos(ExecutionStack *stack, QString *err = 0);
    static bool unaryAcot(ExecutionStack *stack, QString *err = 0);
    static bool unaryAcsc(ExecutionStack *stack, QString *err = 0);
    static bool unaryArch(ExecutionStack *stack, QString *err = 0);
    static bool unaryArcsch(ExecutionStack *stack, QString *err = 0);
    static bool unaryArcth(ExecutionStack *stack, QString *err = 0);
    static bool unaryArsch(ExecutionStack *stack, QString *err = 0);
    static bool unaryArsh(ExecutionStack *stack, QString *err = 0);
    static bool unaryArth(ExecutionStack *stack, QString *err = 0);
    static bool unaryAsec(ExecutionStack *stack, QString *err = 0);
    static bool unaryAsin(ExecutionStack *stack, QString *err = 0);
    static bool unaryAtan(ExecutionStack *stack, QString *err = 0);
    static bool unaryCh(ExecutionStack *stack, QString *err = 0);
    static bool unaryCos(ExecutionStack *stack, QString *err = 0);
    static bool unaryCot(ExecutionStack *stack, QString *err = 0);
    static bool unaryCsc(ExecutionStack *stack, QString *err = 0);
    static bool unaryCsch(ExecutionStack *stack, QString *err = 0);
    static bool unaryCth(ExecutionStack *stack, QString *err = 0);
    static bool unarySec(ExecutionStack *stack, QString *err = 0);
    static bool unarySech(ExecutionStack *stack, QString *err = 0);
    static bool unarySh(ExecutionStack *stack, QString *err = 0);
    static bool unarySin(ExecutionStack *stack, QString *err = 0);
    static bool unaryTan(ExecutionStack *stack, QString *err = 0);
    static bool unaryTh(ExecutionStack *stack, QString *err = 0);
private:
    Q_DISABLE_COPY(TrigonometricFunction)
};

#endif // TRIGONOMETRICFUNCTION_H
