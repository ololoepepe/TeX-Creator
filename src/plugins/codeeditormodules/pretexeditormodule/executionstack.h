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

#ifndef EXECUTIONSTACK_H
#define EXECUTIONSTACK_H

class QByteArray;

#include <QMap>
#include <QString>

/*============================================================================
================================ ExecutionStack ==============================
============================================================================*/

class ExecutionStack
{
public:
    explicit ExecutionStack(ExecutionStack *parent = 0);
public:
    bool define(const QString &id, const QString &value, bool global = false);
    bool defineF(const QString &id, const QString &value, bool global = true);
    bool undefine(const QString &id);
    bool set(const QString &id, const QString &value);
    bool setF(const QString &id, const QString &value);
    bool get(const QString &id, QString &value) const;
    bool getF(const QString &id, QString &value) const;
    bool isDefined(const QString &id) const;
    QByteArray save() const;
    void restore(const QByteArray &data);
    void clear();
private:
    ExecutionStack *mparent;
    QMap<QString, QString> mmap;
    QMap<QString, QString> mmapF;
};

#endif // EXECUTIONSTACK_H
