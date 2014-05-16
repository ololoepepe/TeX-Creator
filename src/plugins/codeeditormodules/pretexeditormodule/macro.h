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

#ifndef MACRO_H
#define MACRO_H

class ExecutionStack;
class AbstractMacroCommand;

class BAbstractCodeEditorDocument;
class BCodeEditor;

class QString;
class QKeyEvent;

#include <QList>

/*============================================================================
================================ Macro =======================================
============================================================================*/

class Macro
{
public:
    explicit Macro();
    explicit Macro(const QString &fileName);
    Macro(const Macro &other);
    ~Macro();
public:
    void clear();
    void execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, BCodeEditor *edtr, QString *error = 0) const;
    bool recordKeyPress(const QKeyEvent *e, QString *error = 0);
    bool fromText(const QString &text, QString *error = 0);
    bool fromFile(const QString &fileName, QString *error = 0);
    QString toText(QString *error = 0) const;
    bool toFile(const QString &fileName, QString *error = 0) const;
    bool isValid() const;
    bool isEmpty() const;
public:
    Macro &operator=(const Macro &other);
private:
    QList<AbstractMacroCommand *> mcommands;
};

#endif // MACRO_H
