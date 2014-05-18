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

class Token;

class BAbstractCodeEditorDocument;

class QByteArray;

#include "pretexvariant.h"
#include "pretexarray.h"
#include "pretexfunction.h"
#include "pretexstatement.h"

#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>
#include <QFlags>
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
    enum SpecialFlag
    {
        NoFlag = 0x00,
        ReturnFlag = 0x01,
        BreakFlag = 0x02,
        ContinueFlag = 0x04
    };
    Q_DECLARE_FLAGS(SpecialFlags, SpecialFlag)
public:
    explicit ExecutionStack(ExecutionStack *parent = 0, SpecialFlags acceptedFlags = NoFlag);
    explicit ExecutionStack(BAbstractCodeEditorDocument *document, Token *token,
                            const QList<PretexVariant> &obligatoryArguments,
                            const QList<PretexVariant> &optionalArguments, ExecutionStack *parent = 0,
                            SpecialFlags acceptedFlags = NoFlag);
    explicit ExecutionStack(BAbstractCodeEditorDocument *document, Token *token,
                            const QList<PretexVariant> &obligatoryArguments,
                            const QList<PretexVariant> &optionalArguments, const QList<PretexFunction> &specialArgs,
                            ExecutionStack *parent = 0, SpecialFlags acceptedFlags = NoFlag);
public:
    bool declareVar(bool global, const QString &name, const PretexVariant &value, QString *err = 0);
    bool declareArray(bool global, const QString &name, const PretexArray::Dimensions &dimensions, QString *err = 0);
    bool declareFunc(bool global, const QString &name, int obligatoryArgumentCount, int optionalAgrumentCount,
                     const QList<PretexStatement> &body, QString *err = 0);
    bool isNameOccupied(const QString &name, bool global, NameType *t = 0) const;
    ExecutionStack *parent() const;
    bool setVar(const QString &name, const PretexVariant &value, QString *err = 0);
    bool setArrayElement(const QString &name, const PretexArray::Indexes &indexes, const PretexVariant &value,
                         QString *err = 0);
    bool setFunc(const QString &name, const QList<PretexStatement> &body, QString *err = 0);
    bool undeclare(const QString &name, QString *err = 0);
    void clear();
    QByteArray saveState() const;
    void restoreState(const QByteArray &state);
    BAbstractCodeEditorDocument *doc() const;
    Token *token() const;
    const QList<PretexVariant> &obligArgs() const;
    const QList<PretexVariant> &optArgs() const;
    const QList<PretexFunction> &specialArgs() const;
    PretexVariant obligArg(int index = 0) const;
    PretexVariant optArg(int index = 0) const;
    PretexFunction specialArg(int index = 0) const;
    int obligArgCount() const;
    int optArgCount() const;
    int specialArgCount() const;
    void setReturnValue(const PretexVariant &v);
    void setReturnValue(const QString &s);
    void setReturnValue(int i);
    void setReturnValue(double d);
    PretexVariant returnValue() const;
    SpecialFlags acceptedFlags() const;
    bool isFlagAccepted(SpecialFlag flag);
    bool setFlag(SpecialFlag flag, QString *err = 0);
    /*bool define(const QString &id, const QString &value, bool global = false);
    bool defineF(const QString &id, const QString &value, bool global = true);
    bool undefine(const QString &id);
    bool set(const QString &id, const QString &value);
    bool setF(const QString &id, const QString &value);
    bool get(const QString &id, QString &value) const;
    bool getF(const QString &id, QString &value) const;
    bool isDefined(const QString &id) const;*/
private:
    ExecutionStack *mparent;
    BAbstractCodeEditorDocument *mdocument;
    Token *mtoken;
    QList<PretexVariant> mobligArgs;
    QList<PretexVariant> moptArgs;
    QList<PretexFunction> mspecialArgs;
    PretexVariant mretVal;
    SpecialFlags maccepedFlags;
    SpecialFlag mflag;
    QMap<QString, PretexVariant> mvars;
    QMap<QString, PretexArray> marrays;
    QMap<QString, PretexFunction> mfuncs;
    //
    //
    /*QMap<QString, QString> mmap;
    QMap<QString, QString> mmapF;*/
};

#endif // EXECUTIONSTACK_H
