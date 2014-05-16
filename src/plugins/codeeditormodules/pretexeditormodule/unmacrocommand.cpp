/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
**
** This file is part of the MacrosEditorModule plugin of TeX Creator.
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

#include "unmacrocommand.h"
#include "macrocommand.h"
#include "macrocommandargument.h"
#include "executionstack.h"
#include "global.h"

#include <BeQtGlobal>
#include <BAbstractCodeEditorDocument>

#include <QList>
#include <QMap>
#include <QString>

#include <cmath>

/*============================================================================
================================ Template functions ==========================
============================================================================*/

template<typename T> T ctan(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::tan(t);
}

template<typename T> T sec(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::sin(t);
}

template<typename T> T csc(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::cos(t);
}

template<typename T> T acot(T t)
{
    static const T t0 = (T) 0;
    static const T t1 = (T) 1;
    static const T t2 = (T) 2;
    static const T t4 = (T) 4;
    if (t >= t0)
        return std::asin(t1 / std::sqrt(t1 + std::pow(t, t2)));
    else
        return std::atan(t1) * t4 - std::asin(t1 / std::sqrt(t1 + std::pow(t, t2)));
}

template<typename T> T asec(T t)
{
    static const T t1 = (T) 1;
    return std::acos(t1 / t);
}

template<typename T> T acsc(T t)
{
    static const T t1 = (T) 1;
    return std::asin(t1 / t);
}

template<typename T> T cth(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::tanh(t);
}

template<typename T> T sech(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::sinh(t);
}

template<typename T> T csch(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::cosh(t);
}

template<typename T> T arsh(T t)
{
    static const T t1 = (T) 1;
    static const T t2 = (T) 2;
    return std::log(t + std::sqrt(std::pow(t, t2) + t1));
}

template<typename T> T arch(T t)
{
    static const T t1 = (T) 1;
    static const T t2 = (T) 2;
    return std::log(t + std::sqrt(std::pow(t, t2) - t1));
}

template<typename T> T arth(T t)
{
    static const T t1 = (T) 1;
    return 0.5 * (double) std::log((t1 + t) / (t1 - t));
}

template<typename T> T arcth(T t)
{
    static const T t1 = (T) 1;
    return 0.5 * (double) std::log((t + t1) / (t - t1));
}

template<typename T> T arsch(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::sinh(t);
}

template<typename T> T arcsch(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::cosh(t);
}

/*============================================================================
================================ Global static functions =====================
============================================================================*/

int fact(int i)
{
    if (i < 0)
        return -1;
    if (!i || 1 == i)
        return 1;
    return i * fact(i - 1);
}

static QString unaryMinus(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool b = false;
    QString err = Global::toInt(text, i, &b);
    if (!err.isEmpty())
        return err;
    if (b)
    {
        text = QString::number(-1 * i);
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(-1.0 * d, 'g', 15);
    return "";
}

static QString unaryNegation(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    bool b = false;
    QString err = Global::toBool(text, b);
    if (!err.isEmpty())
        return err;
    text = b ? "false" : "true";
    return "";
}

static QString unaryAbs(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(std::abs(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::abs(d), 'g', 15);
    return "";
}

static QString unaryFact(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (i < 0)
        return "Taking factorial of negative nunber";
    text = QString::number(fact(i));
    return "";
}

static QString unaryExp(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::exp(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::exp(d), 'g', 15);
    return "";
}

static QString unaryLn(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::log(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::log(d), 'g', 15);
    return "";
}

static QString unaryLg(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::log10(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::log10(d), 'g', 15);
    return "";
}

static QString unarySqrt(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (i < 0)
            return "Taking square root of negative number";
        text = QString::number((int) std::sqrt(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    if (d < 0.0)
        return "Taking square root of negative number";
    text = QString::number(std::sqrt(d), 'g', 15);
    return "";
}

static QString unarySin(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::sin(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::sin(d), 'g', 15);
    return "";
}

static QString unaryCos(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::cos(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::cos(d), 'g', 15);
    return "";
}

static QString unaryTan(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        //TODO: Check
        text = QString::number((int) std::tan(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::tan(d), 'g', 15);
    return "";
}

static QString unaryCot(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        //TODO: Check
        text = QString::number(ctan(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(ctan(d), 'g', 15);
    return "";
}

static QString unarySec(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        //TODO: Check
        text = QString::number(sec(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(csc(d), 'g', 15);
    return "";
}

static QString unaryCsc(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        //TODO: Check
        text = QString::number((int) (1 / std::cos(i)));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(1 / std::cos(d), 'g', 15);
    return "";
}

static QString unaryAsin(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::asin(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::asin(d), 'g', 15);
    return "";
}

static QString unaryAcos(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::acos(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::acos(d), 'g', 15);
    return "";
}

static QString unaryAtan(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::atan(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::atan(d), 'g', 15);
    return "";
}

static QString unaryAcot(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(acot(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(acot(d), 'g', 15);
    return "";
}

static QString unaryAsec(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (!i)
            return "Taking asec of zero";
        text = QString::number(asec(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(asec(d), 'g', 15);
    return "";
}

static QString unaryAcsc(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (!i)
            return "Taking acsc of zero";
        text = QString::number(acsc(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(acsc(d), 'g', 15);
    return "";
}

static QString unarySh(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::sinh(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::sinh(d), 'g', 15);
    return "";
}

static QString unaryCh(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::cosh(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::cosh(d), 'g', 15);
    return "";
}

static QString unaryTh(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::tanh(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::tanh(d), 'g', 15);
    return "";
}

static QString unaryCth(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(cth(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(cth(d), 'g', 15);
    return "";
}

static QString unarySech(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(sech(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(sech(d), 'g', 15);
    return "";
}

static QString unaryCsch(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(csch(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(csch(d), 'g', 15);
    return "";
}

static QString unaryArsh(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(arsh(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arsh(d), 'g', 15);
    return "";
}

static QString unaryArch(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (i < 1)
            return "Negative ln argument";
        text = QString::number(arch(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arch(d), 'g', 15);
    return "";
}

static QString unaryArth(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (1 == i)
            return "Division by zero";
        text = QString::number(arth(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arth(d), 'g', 15);
    return "";
}

static QString unaryArcth(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (1 == i)
            return "Division by zero";
        text = QString::number(arcth(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arcth(d), 'g', 15);
    return "";
}

static QString unaryArsch(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(arsch(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arsch(d), 'g', 15);
    return "";
}

static QString unaryArcsch(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = Global::toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(arcsch(i));
        return "";
    }
    double d = 0.0;
    err = Global::toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arcsch(d), 'g', 15);
    return "";
}

/*============================================================================
================================ UnMacroCommand ==============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *UnMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new UnMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

UnMacroCommand::UnMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString UnMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    typedef QMap<QString, QString (*)(QString &)> UnaryFuncMap;
    init_once(UnaryFuncMap, funcMap, UnaryFuncMap())
    {
        funcMap.insert("-", &unaryMinus);
        funcMap.insert("!", &unaryNegation);
        funcMap.insert("abs", &unaryAbs);
        funcMap.insert("fact", &unaryFact);
        funcMap.insert("exp", &unaryExp);
        funcMap.insert("ln", &unaryLn);
        funcMap.insert("lg", &unaryLg);
        funcMap.insert("sqrt", &unarySqrt);
        funcMap.insert("sin", &unarySin);
        funcMap.insert("cos", &unaryCos);
        funcMap.insert("tan", &unaryTan);
        funcMap.insert("cot", &unaryCot);
        funcMap.insert("sec", &unarySec);
        funcMap.insert("csc", &unaryCsc);
        funcMap.insert("asin", &unaryAsin);
        funcMap.insert("acos", &unaryAcos);
        funcMap.insert("atan", &unaryAtan);
        funcMap.insert("acot", &unaryAcot);
        funcMap.insert("asec", &unaryAsec);
        funcMap.insert("acsc", &unaryAcsc);
        funcMap.insert("sh", &unarySh);
        funcMap.insert("ch", &unaryCh);
        funcMap.insert("th", &unaryTh);
        funcMap.insert("cth", &unaryCth);
        funcMap.insert("sech", &unarySech);
        funcMap.insert("csch", &unaryCsch);
        funcMap.insert("arsh", &unaryArsh);
        funcMap.insert("arch", &unaryArch);
        funcMap.insert("arth", &unaryArth);
        funcMap.insert("arcth", &unaryArcth);
        funcMap.insert("arsch", &unaryArsch);
        funcMap.insert("arcsch", &unaryArcsch);
    }
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    if (!funcMap.contains(s))
        return bRet(error, QString("Unknown operator"), QString());
    QString v = margs.at(1).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString f;
    if (margs.size() == 3)
    {
        f = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    err = funcMap[s](v);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    err = Global::formatText(v, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), v);
}

QString UnMacroCommand::name() const
{
    return "un";
}

QString UnMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\un{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *UnMacroCommand::clone() const
{
    return new UnMacroCommand(margs);
}
