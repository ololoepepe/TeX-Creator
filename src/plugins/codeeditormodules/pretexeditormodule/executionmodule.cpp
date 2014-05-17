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

#include "executionmodule.h"
#include "pretexeditormodule.h"
#include "executionstack.h"
#include "token.h"
#include "tokendata.h"

#include <BAbstractCodeEditorDocument>

#include <QString>

/*============================================================================
================================ ExecutionModule =============================
============================================================================*/

/*============================== Public constructors =======================*/

ExecutionModule::ExecutionModule()
{
    mprog = 0;
    mdoc = 0;
    mstack = 0;
}

ExecutionModule::ExecutionModule(Token *program, BAbstractCodeEditorDocument *doc, ExecutionStack *stack)
{
    mprog = program;
    mdoc = doc;
    mstack = stack;
}

void ExecutionModule::setProgram(Token *prog)
{
    mprog = prog;
}

void ExecutionModule::setDocument(BAbstractCodeEditorDocument *doc)
{
    mdoc = doc;
}

void ExecutionModule::setExecutionStack(ExecutionStack *stack)
{
    mstack = stack;
}

Token *ExecutionModule::program() const
{
    return mprog;
}

BAbstractCodeEditorDocument *ExecutionModule::document() const
{
    return mdoc;
}

ExecutionStack *ExecutionModule::executionStack() const
{
    return mstack;
}

bool ExecutionModule::execute(QString *err)
{
    //
    return bRet(err, QString(), true);
}
