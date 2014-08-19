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

#ifndef IOFUNCTION_H
#define IOFUNCTION_H

class ExecutionStack;

class QString;

#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"

#include <QCoreApplication>
#include <QInputDialog>
#include <QList>
#include <QMessageBox>
#include <QTextDocument>

/*============================================================================
================================ IOFunction ==================================
============================================================================*/

class IOFunction : public PretexBuiltinFunction
{
    Q_DECLARE_TR_FUNCTIONS(IOFunction)
public:
    enum Type
    {
        FindType,
        GetInputType,
        InsertType,
        PressType,
        ReadFileType,
        ReplaceType,
        RunDetachedType,
        RunType,
        ShowMessageType
    };
private:
    Type mtype;
public:
    explicit IOFunction(Type t);
public:
    bool execute(ExecutionStack *stack, QString *err = 0);
    QString name() const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
private:
    static bool find(ExecutionStack *stack, QString *err = 0);
    static bool getInput(ExecutionStack *stack, QString *err = 0);
    static bool icon(const PretexVariant &v, QMessageBox::Icon *icon, QString *err = 0);
    static bool inputMode(const PretexVariant &v, QInputDialog::InputMode *mode, QString *err = 0);
    static bool insert(ExecutionStack *stack, QString *err = 0);
    static bool press(ExecutionStack *stack, QString *err = 0);
    static bool readFile(ExecutionStack *stack, QString *err = 0);
    static bool replace(ExecutionStack *stack, QString *err = 0);
    static bool replaceOptions(const PretexVariant &v, Qt::CaseSensitivity *cs, QString *err = 0);
    static bool replaceScope(const PretexVariant &v, bool *selection, QString *err = 0);
    static bool run(ExecutionStack *stack, bool detached, QString *err = 0);
    static bool searchOptions(const PretexVariant &v, QTextDocument::FindFlags *flags, bool *cyclic, QString *err = 0);
    static bool showMessage(ExecutionStack *stack, QString *err = 0);
private:
    Q_DISABLE_COPY(IOFunction)
};

#endif // IOFUNCTION_H
