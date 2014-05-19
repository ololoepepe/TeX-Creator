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

#ifndef PRETEXFUNCTION_H
#define PRETEXFUNCTION_H

class ExecutionStack;

class QDataStream;
class QDebug;

#include "pretexstatement.h"
#include "pretexvariant.h"

#include <QMetaType>
#include <QList>
#include <QString>
#include <QCoreApplication>

/*============================================================================
================================ PretexFunction ==============================
============================================================================*/

class PretexFunction
{
    Q_DECLARE_TR_FUNCTIONS(PretexFunction)
public:
    explicit PretexFunction();
    explicit PretexFunction(const QString &name, int obligatoryArgumentCount, int optionalArgumentCount,
                            const QList<PretexStatement> &body);
    PretexFunction(const PretexFunction &other);
public:
    const QList<PretexStatement> &body() const;
    void clear();
    bool execute(ExecutionStack *stack, QString *err = 0);
    bool isEmpty() const;
    bool isValid() const;
    QString name() const;
    int obligatoryArgumentCount();
    int optionalArgumentCount();
    void setBody(const QList<PretexStatement> &list);
public:
    bool operator!=(const PretexFunction &other) const;
    PretexFunction &operator= (const PretexFunction &other);
    bool operator== (const PretexFunction &other) const;
public:
    friend QDataStream &operator<< (QDataStream &s, const PretexFunction &f);
    friend QDataStream &operator>> (QDataStream &s, PretexFunction &f);
    friend QDebug operator<< (QDebug dbg, const PretexFunction &f);
private:
    QString mname;
    int mobligArgCount;
    int moptArgCount;
    QList<PretexStatement> mbody;
};

Q_DECLARE_METATYPE(PretexFunction)

#endif // PRETEXFUNCTION_H
