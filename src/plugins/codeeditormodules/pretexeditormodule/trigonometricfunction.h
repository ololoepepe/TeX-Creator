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

class ExecutionContext;

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
    bool execute(ExecutionContext *context, QString *err = 0);
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
    static bool unaryAcos(ExecutionContext *context, QString *err = 0);
    static bool unaryAcot(ExecutionContext *context, QString *err = 0);
    static bool unaryAcsc(ExecutionContext *context, QString *err = 0);
    static bool unaryArch(ExecutionContext *context, QString *err = 0);
    static bool unaryArcsch(ExecutionContext *context, QString *err = 0);
    static bool unaryArcth(ExecutionContext *context, QString *err = 0);
    static bool unaryArsch(ExecutionContext *context, QString *err = 0);
    static bool unaryArsh(ExecutionContext *context, QString *err = 0);
    static bool unaryArth(ExecutionContext *context, QString *err = 0);
    static bool unaryAsec(ExecutionContext *context, QString *err = 0);
    static bool unaryAsin(ExecutionContext *context, QString *err = 0);
    static bool unaryAtan(ExecutionContext *context, QString *err = 0);
    static bool unaryCh(ExecutionContext *context, QString *err = 0);
    static bool unaryCos(ExecutionContext *context, QString *err = 0);
    static bool unaryCot(ExecutionContext *context, QString *err = 0);
    static bool unaryCsc(ExecutionContext *context, QString *err = 0);
    static bool unaryCsch(ExecutionContext *context, QString *err = 0);
    static bool unaryCth(ExecutionContext *context, QString *err = 0);
    static bool unarySec(ExecutionContext *context, QString *err = 0);
    static bool unarySech(ExecutionContext *context, QString *err = 0);
    static bool unarySh(ExecutionContext *context, QString *err = 0);
    static bool unarySin(ExecutionContext *context, QString *err = 0);
    static bool unaryTan(ExecutionContext *context, QString *err = 0);
    static bool unaryTh(ExecutionContext *context, QString *err = 0);
private:
    Q_DISABLE_COPY(TrigonometricFunction)
};

#endif // TRIGONOMETRICFUNCTION_H
