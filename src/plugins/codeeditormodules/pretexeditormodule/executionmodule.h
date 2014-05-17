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

#ifndef EXECUTIONMODULE_H
#define EXECUTIONMODULE_H

class ExecutionStack;
class Token;

class BAbstractCodeEditorDocument;

class QString;

/*============================================================================
================================ ExecutionModule =============================
============================================================================*/

class ExecutionModule
{
public:
    explicit ExecutionModule();
    explicit ExecutionModule(Token *program, BAbstractCodeEditorDocument *doc, ExecutionStack *stack);
public:
    void setProgram(Token *prog);
    void setDocument(BAbstractCodeEditorDocument *doc);
    void setExecutionStack(ExecutionStack *stack);
    Token *program() const;
    BAbstractCodeEditorDocument *document() const;
    ExecutionStack *executionStack() const;
    bool execute(QString *err = 0);
private:
    Token *mprog;
    BAbstractCodeEditorDocument *mdoc;
    ExecutionStack *mstack;
};

#endif // EXECUTIONMODULE_H
