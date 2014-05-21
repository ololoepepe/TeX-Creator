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

#ifndef EXECUTIONSTACK_H
#define EXECUTIONSTACK_H

class BAbstractCodeEditorDocument;

class QByteArray;

#include "pretexvariant.h"
#include "pretexarray.h"
#include "pretexfunction.h"
#include "token.h"
#include "pretexbuiltinfunction.h"

#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>
#include <QCoreApplication>

/*============================================================================
================================ ExecutionStack ==============================
============================================================================*/

class ExecutionStack
{
    Q_DECLARE_TR_FUNCTIONS(ExecutionStack)
public:
    enum NameType
    {
        UnknownName = 0,
        VariableName,
        ArrayName,
        UserFunctionName,
        BuiltinFunctionName
    };
public:
    explicit ExecutionStack(ExecutionStack *parent = 0);
    explicit ExecutionStack(BAbstractCodeEditorDocument *document, ExecutionStack *parent = 0);
    explicit ExecutionStack(const QList<PretexVariant> &obligatoryArguments,
                            const QList<PretexVariant> &optionalArguments, const QString &caller,
                            ExecutionStack *parent = 0);
    explicit ExecutionStack(const QList<PretexVariant> &obligatoryArguments,
                            const QList<PretexVariant> &optionalArguments, const QList<Token> &specialArgs,
                            const QString &caller, ExecutionStack *parent = 0);
public:
    bool declareVar(bool global, const QString &name, const PretexVariant &value, QString *err = 0);
    bool declareArray(bool global, const QString &name, const PretexArray::Dimensions &dimensions, QString *err = 0);
    bool declareFunc(bool global, const QString &name, int obligatoryArgumentCount, int optionalAgrumentCount,
                     const Token &body, QString *err = 0);
    bool isNameOccupied(const QString &name, bool global, NameType *t = 0) const;
    ExecutionStack *parent() const;
    bool setVar(const QString &name, const PretexVariant &value, QString *err = 0);
    bool setArrayElement(const QString &name, const PretexArray::Indexes &indexes, const PretexVariant &value,
                         QString *err = 0);
    bool setFunc(const QString &name, const Token &body, QString *err = 0);
    bool undeclare(const QString &name, QString *err = 0);
    void clear();
    QByteArray saveState() const;
    void restoreState(const QByteArray &state);
    PretexVariant variable(const QString &name) const;
    PretexArray::Dimensions arrayDimensions(const QString &name) const;
    PretexVariant arrayElement(const QString &name, const PretexArray::Indexes &indexes) const;
    PretexFunction *function(const QString &name) const;
    BAbstractCodeEditorDocument *doc() const;
    const QList<PretexVariant> &obligArgs() const;
    const QList<PretexVariant> &optArgs() const;
    const QList<Token> &specialArgs() const;
    PretexVariant obligArg(int index = 0) const;
    PretexVariant optArg(int index = 0) const;
    Token specialArg(int index = 0) const;
    int obligArgCount() const;
    int optArgCount() const;
    int specialArgCount() const;
    QString caller() const;
    int maxArgCount() const;
    void setReturnValue(const PretexVariant &v);
    void setReturnValue(const QString &s);
    void setReturnValue(int i);
    void setReturnValue(double d);
    PretexVariant returnValue() const;
    bool setFlag(PretexBuiltinFunction::SpecialFlag flag, QString *err = 0);
    PretexBuiltinFunction::SpecialFlag flag() const;
private:
    bool isFlagAccepted(PretexBuiltinFunction::SpecialFlag flag, bool *propagate = 0) const;
private:
    ExecutionStack *mparent;
    BAbstractCodeEditorDocument *mdocument;
    QList<PretexVariant> mobligArgs;
    QList<PretexVariant> moptArgs;
    QList<Token> mspecialArgs;
    QString mcaller;
    PretexVariant mretVal;
    PretexBuiltinFunction::SpecialFlag mflag;
    QMap<QString, PretexVariant> mvars;
    QMap<QString, PretexArray> marrays;
    QMap<QString, PretexFunction> mfuncs;
};

#endif // EXECUTIONSTACK_H
