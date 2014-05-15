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

#ifndef MACROCOMMAND_H
#define MACROCOMMAND_H

class MacroExecutionStack;

class BAbstractCodeEditorDocument;

class QString;
class QKeyEvent;

#include "macrocommandargument.h"

#include <QList>

/*============================================================================
================================ AbstractMacroCommand ========================
============================================================================*/

class AbstractMacroCommand
{
public:
    static AbstractMacroCommand *fromText(QString text, QString *error = 0);
    static AbstractMacroCommand *fromKeyPressEvent(const QKeyEvent *e, AbstractMacroCommand *previousCommand = 0);
    static AbstractMacroCommand *fromKeyPressEvent(const QKeyEvent *e, QString *error,
                                                   AbstractMacroCommand *previousCommand = 0);
public:
    virtual ~AbstractMacroCommand();
protected:
    explicit AbstractMacroCommand(const QList<MacroCommandArgument> &args);
public:
    virtual void clear();
    virtual QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                            QString *error = 0) const = 0;
    virtual QString name() const = 0;
    virtual QString toText() const = 0;
    virtual bool isValid() const;
    virtual AbstractMacroCommand *clone() const = 0;
    bool compare(const AbstractMacroCommand *other) const;
protected:
    virtual bool compareInternal(const AbstractMacroCommand *other) const;
protected:
    QList<MacroCommandArgument> margs;
};

#endif // MACROCOMMAND_H
