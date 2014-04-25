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

#ifndef MACROCOMMANDARGUMENT_H
#define MACROCOMMANDARGUMENT_H

class AbstractMacroCommand;
class MacroExecutionStack;

class BAbstractCodeEditorDocument;

#include <QString>

/*============================================================================
================================ MacroCommandArgument ========================
============================================================================*/

class MacroCommandArgument
{
public:
    static bool isCommand(QString txt);
    static bool isText(QString txt);
public:
    explicit MacroCommandArgument();
    explicit MacroCommandArgument(const QString &txt);
    MacroCommandArgument(const MacroCommandArgument &other);
    ~MacroCommandArgument();
public:
    void clear();
    bool fromText(const QString &txt);
    QString toText() const;
    QString toText(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    bool isCommand() const;
    bool isText() const;
    bool isValid() const;
    QString error() const;
public:
    MacroCommandArgument &operator =(const MacroCommandArgument &other);
    bool operator ==(const MacroCommandArgument &other) const;
    bool operator !=(const MacroCommandArgument &other) const;
private:
    void init();
private:
    QString text;
    AbstractMacroCommand *command;
    QString err;
};

#endif // MACROCOMMANDARGUMENT_H
