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

#include "binmacrocommands.h"
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

template<typename T> T anyLog(T base, T t)
{
    return std::log(t) / std::log(base);
}

template<typename T> T root(T base, T p)
{
    static const T t1 = (T) 1;
    return std::pow(base, t1 / p);
}

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static QString binaryAddition(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = text1 + text2;
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = text1 + text2;
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = text1 + text2;
        return "";
    }
    if (native1 && native2)
    {
        text1 = QString::number(i1 + i2);
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = QString::number(d1 + d2, 'g', 15);
    return "";
}

static QString binarySubtraction(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        text1 = QString::number(i1 - i2);
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = QString::number(d1 - d2, 'g', 15);
    return "";
}

static QString binaryMultiplication(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        text1 = QString::number(i1 * i2);
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = QString::number(d1 * d2, 'g', 15);
    return "";
}

static QString binaryDivision(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        if (!i2)
            return "Division by zero";
        text1 = QString::number(i1 / i2);
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = QString::number(d1 / d2, 'g', 15);
    return "";
}

static QString binaryModulo(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    QString err = Global::toInt(text1, i1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    err = Global::toInt(text2, i2);
    if (!err.isEmpty())
        return err;
    if (!i2)
        return "Division by zero";
    text1 = QString::number(i1 % i2);
    return "";
}

static QString binaryInvolution(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        if (!i1)
            return "Involution of zero";
        text1 = QString::number(std::pow(i1, i2));
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = QString::number(std::pow(d1, d2), 'g', 15);
    return "";
}

static QString binaryLog(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        if (i1 <= 0 || i2 <= 0)
            return "Invalid log base/power";
        text1 = QString::number(anyLog(i1, i2));
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    if (d1 <= 0 || d2 <= 0)
        return "Invalid log base/power";
    text1 = QString::number(anyLog(d1, d2), 'g', 15);
    return "";
}

static QString binaryRoot(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        if (i1 < 0 || i2 < 0)
            return "Invalid root base/power";
        text1 = QString::number(root(i1, i2));
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    if (d1 < 0 || d2 < 0)
        return "Invalid root base/power";
    text1 = QString::number(root(d1, d2), 'g', 15);
    return "";
}

static QString binaryEqual(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 == text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 == text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 == text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 == i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = (d1 == d2) ? "true" : "false";
    return "";
}

static QString binaryNotEqual(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 != text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 != text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 != text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 != i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = (d1 != d2) ? "true" : "false";
    return "";
}

static QString binaryLesser(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 < text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 < text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 < text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 < i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = (d1 < d2) ? "true" : "false";
    return "";
}

static QString binaryLesserOrEqual(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 <= text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 <= text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 <= text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 <= i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = (d1 <= d2) ? "true" : "false";
    return "";
}

static QString binaryGreater(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 > text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 > text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 > text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 > i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = (d1 > d2) ? "true" : "false";
    return "";
}

static QString binaryGreaterOrEqual(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 >= text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = Global::toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 >= text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = Global::toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 >= text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 >= i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = Global::toDouble(text1, d1);
    double d2 = 0.0;
    err = Global::toDouble(text2, d2);
    text1 = (d1 >= d2) ? "true" : "false";
    return "";
}

static QString binaryAnd(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    bool b1 = false;
    QString err = Global::toBool(text1, b1);
    if (!err.isEmpty())
        return err;
    bool b2 = false;
    err = Global::toBool(text2, b2);
    if (!err.isEmpty())
        return err;
    text1 = (b1 && b2) ? "true" : "false";
    return "";
}

static QString binaryOr(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    bool b1 = false;
    QString err = Global::toBool(text1, b1);
    if (!err.isEmpty())
        return err;
    bool b2 = false;
    err = Global::toBool(text2, b2);
    if (!err.isEmpty())
        return err;
    text1 = (b1 || b2) ? "true" : "false";
    return "";
}

/*============================================================================
================================ BinMacroCommand =============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *BinMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 3 && args.size() <= 4) ? new BinMacroCommand(args) : 0;
}

BinMacroCommand::BinaryFunction BinMacroCommand::binaryFunction(const QString &op)
{
    typedef QMap<QString, BinaryFunction> BinaryFuncMap;
    init_once(BinaryFuncMap, funcMap, BinaryFuncMap())
    {
        funcMap.insert("+", &binaryAddition);
        funcMap.insert("-", &binarySubtraction);
        funcMap.insert("*", &binaryMultiplication);
        funcMap.insert("/", &binaryDivision);
        funcMap.insert("mod", &binaryModulo);
        funcMap.insert("^", &binaryInvolution);
        funcMap.insert("log", &binaryLog);
        funcMap.insert("root", &binaryRoot);
        funcMap.insert("==", &binaryEqual);
        funcMap.insert("!=", &binaryNotEqual);
        funcMap.insert("<", &binaryLesser);
        funcMap.insert("<=", &binaryLesserOrEqual);
        funcMap.insert(">", &binaryGreater);
        funcMap.insert(">=", &binaryGreaterOrEqual);
        funcMap.insert("||", &binaryOr);
        funcMap.insert("&&", &binaryAnd);
    }
    return funcMap.value(op);
}

bool BinMacroCommand::hasBinaryFunction(const QString &op)
{
    return binaryFunction(op);
}

/*============================== Private constructors ======================*/

BinMacroCommand::BinMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString BinMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    if (!hasBinaryFunction(s))
        return bRet(error, QString("Unknown operator"), QString());
    QString v1 = margs.at(1).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString v2 = margs.at(2).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString f;
    if (margs.size() == 4)
    {
        f = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    err = binaryFunction(s)(v1, v2);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    err = Global::formatText(v1, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), v1);
}

QString BinMacroCommand::name() const
{
    return "bin";
}

QString BinMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\bin{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}{" + margs.at(2).toText() + "}";
    for (int i = 3; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *BinMacroCommand::clone() const
{
    return new BinMacroCommand(margs);
}

/*============================================================================
================================ BinMMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *BinMMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 3) ? new BinMMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

BinMMacroCommand::BinMMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString BinMMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    if (!BinMacroCommand::hasBinaryFunction(s))
        return bRet(error, QString("Unknown operator"), QString());
    QString v1 = margs.at(1).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    BinMacroCommand::BinaryFunction func = BinMacroCommand::binaryFunction(s);
    for (int i = 2; i < margs.size(); ++i)
    {
        QString v2 = margs.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
        err = func(v1, v2);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    return bRet(error, QString(), v1);
}

QString BinMMacroCommand::name() const
{
    return "binM";
}

QString BinMMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\binM{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}{" + margs.at(2).toText() + "}";
    for (int i = 3; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *BinMMacroCommand::clone() const
{
    return new BinMMacroCommand(margs);
}
