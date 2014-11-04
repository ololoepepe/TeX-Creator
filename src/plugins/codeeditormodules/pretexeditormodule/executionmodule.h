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

#ifndef EXECUTIONMODULE_H
#define EXECUTIONMODULE_H

class ExecutionContext;
class Function_TokenData;
class PretexVariant;
class Subprogram_TokenData;
class Token;

class BAbstractCodeEditorDocument;

class QString;

#include <QCoreApplication>

/*============================================================================
================================ ExecutionModule =============================
============================================================================*/

class ExecutionModule
{
    Q_DECLARE_TR_FUNCTIONS(ExecutionModule)
private:
    BAbstractCodeEditorDocument *mdoc;
    Token *mprog;
    ExecutionContext *mcontext;
public:
    explicit ExecutionModule();
    explicit ExecutionModule(Token *program, BAbstractCodeEditorDocument *doc, ExecutionContext *context);
public:
    static PretexVariant executeSubprogram(ExecutionContext *context, Subprogram_TokenData *a, const QString &caller,
                                           bool *ok = 0, QString *err = 0);
public:
    BAbstractCodeEditorDocument *document() const;
    bool execute(QString *err = 0);
    ExecutionContext *executionContext() const;
    Token *program() const;
    void setDocument(BAbstractCodeEditorDocument *doc);
    void setExecutionContext(ExecutionContext *context);
    void setProgram(Token *prog);
private:
    static PretexVariant executeFunction(ExecutionContext *context, Function_TokenData *f, bool *ok = 0,
                                         QString *err = 0);
};

#endif // EXECUTIONMODULE_H
