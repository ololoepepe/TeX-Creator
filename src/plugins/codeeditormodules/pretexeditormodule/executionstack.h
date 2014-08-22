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

#ifndef EXECUTIONSTACK_H
#define EXECUTIONSTACK_H

class PretexEditorModule;

class BAbstractCodeEditorDocument;

class QByteArray;

#include "pretexarray.h"
#include "pretexbuiltinfunction.h"
#include "pretexfunction.h"
#include "pretexvariant.h"
#include "token.h"

#include <QCoreApplication>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

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
        ArrayName,
        BuiltinFunctionName,
        UserFunctionName,
        VariableName
    };
private:
    PretexEditorModule * const EditorModule;
private:
    QMap<QString, PretexArray> marrays;
    QString mcaller;
    BAbstractCodeEditorDocument *mdocument;
    PretexBuiltinFunction::SpecialFlag mflag;
    QMap<QString, PretexFunction> mfuncs;
    QList<PretexVariant> mobligArgs;
    QList<PretexVariant> moptArgs;
    ExecutionStack *mparent;
    PretexVariant mretVal;
    QList<Token> mspecialArgs;
    QMap<QString, PretexVariant> mvars;
public:
    explicit ExecutionStack(PretexEditorModule *editorModule);
    explicit ExecutionStack(ExecutionStack *parent);
    explicit ExecutionStack(BAbstractCodeEditorDocument *document, ExecutionStack *parent = 0);
    explicit ExecutionStack(const QList<PretexVariant> &obligatoryArguments,
                            const QList<PretexVariant> &optionalArguments, const QString &caller,
                            ExecutionStack *parent = 0);
    explicit ExecutionStack(const QList<PretexVariant> &obligatoryArguments,
                            const QList<PretexVariant> &optionalArguments, const QList<Token> &specialArgs,
                            const QString &caller, ExecutionStack *parent = 0);
public:
    PretexArray::Dimensions arrayDimensions(const QString &name) const;
    PretexVariant arrayElement(const QString &name, const PretexArray::Indexes &indexes) const;
    QString caller() const;
    void clear();
    bool declareArray(bool global, const QString &name, const PretexArray::Dimensions &dimensions, QString *err = 0);
    bool declareFunc(bool global, const QString &name, int obligatoryArgumentCount, int optionalAgrumentCount,
                     const Token &body, QString *err = 0);
    bool declareVar(bool global, const QString &name, const PretexVariant &value, QString *err = 0);
    BAbstractCodeEditorDocument *doc() const;
    PretexEditorModule *editorModule() const;
    PretexBuiltinFunction::SpecialFlag flag() const;
    PretexFunction *function(const QString &name) const;
    bool isNameOccupied(const QString &name, bool global, NameType *t = 0) const;
    int maxArgCount() const;
    PretexVariant obligArg(int index = 0) const;
    int obligArgCount() const;
    const QList<PretexVariant> &obligArgs() const;
    PretexVariant optArg(int index = 0) const;
    int optArgCount() const;
    const QList<PretexVariant> &optArgs() const;
    ExecutionStack *parent() const;
    void restoreState(const QByteArray &state);
    PretexVariant returnValue() const;
    QByteArray saveState() const;
    bool setArrayElement(const QString &name, const PretexArray::Indexes &indexes, const PretexVariant &value,
                         QString *err = 0);
    bool setFlag(PretexBuiltinFunction::SpecialFlag flag, QString *err = 0);
    bool setFunc(const QString &name, const Token &body, QString *err = 0);
    void setReturnValue(const PretexVariant &v);
    void setReturnValue(const QString &s);
    void setReturnValue(int i);
    void setReturnValue(double d);
    bool setVar(const QString &name, const PretexVariant &value, QString *err = 0);
    Token specialArg(int index = 0) const;
    int specialArgCount() const;
    const QList<Token> &specialArgs() const;
    bool undeclare(const QString &name, QString *err = 0);
    PretexVariant variable(const QString &name) const;
private:
    bool isFlagAccepted(PretexBuiltinFunction::SpecialFlag flag, bool *propagate = 0) const;
};

#endif // EXECUTIONSTACK_H
