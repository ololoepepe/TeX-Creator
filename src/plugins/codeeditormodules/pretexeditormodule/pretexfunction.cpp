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

#include "pretexfunction.h"
#include "pretexstatement.h"
#include "executionstack.h"
#include "pretexbuiltinfunction.h"
#include "executionmodule.h"

#include <BeQtGlobal>

#include <QList>
#include <QString>
#include <QDataStream>
#include <QDebug>
#include <QVariantList>

/*============================================================================
================================ PretexFunction ==============================
============================================================================*/

/*============================== Public constructors =======================*/

PretexFunction::PretexFunction()
{
    mobligArgCount = 0;
    moptArgCount = 0;
}

PretexFunction::PretexFunction(const QString &name, int obligatoryArgumentCount, int optionalArgumentCount,
                               const QList<PretexStatement> &body)
{
    if (name.isEmpty() || PretexBuiltinFunction::isBuiltinFunction(name) || obligatoryArgumentCount <= 0)
    {
        mobligArgCount = 0;
        moptArgCount = 0;
        return;
    }
    mname = name;
    mobligArgCount = obligatoryArgumentCount;
    moptArgCount = optionalArgumentCount >= 0 ? optionalArgumentCount : -1;
    mbody = body;
}

PretexFunction::PretexFunction(const PretexFunction &other)
{
    *this = other;
}

/*============================== Public methods ============================*/

const QList<PretexStatement> &PretexFunction::body() const
{
    return mbody;
}

void PretexFunction::clear()
{
    mname.clear();
    mobligArgCount = 0;
    moptArgCount = 0;
    mbody.clear();
}

bool PretexFunction::execute(ExecutionStack *stack, QString *err)
{
    if (!isValid())
        return bRet(err, tr("Attempted to execute invalid function", "error"), false);
    if (stack->obligArgCount() != obligatoryArgumentCount())
        return bRet(err, tr("Obligatory argument count mismatch", "error"), false);
    if (optionalArgumentCount() >= 0 && stack->optArgCount() > optionalArgumentCount())
        return bRet(err, tr("Optional argument count mismatch", "error"), false);
    //TODO
    if (mbody.isEmpty())
        return bRet(err, QString(), true);
    QList<PretexVariant> list;
    foreach (const PretexStatement &st, mbody)
    {
        switch (st.type())
        {
        case PretexStatement::Value:
            list << st.value();
            break;
        case PretexStatement::BuiltinFunction:
        {
            //ExecutionStack s(0, oblArgs, optArgs, stack);
            //if (!st.builtinFunction()->execute(s, err))
            //    return false;
            //list << s.returnValue();
            break;
        }
        case PretexStatement::UserFunction:
        {
            //ExecutionStack s(0, oblArgs, optArgs, stack,
            //                 PretexBuiltinFunction::functionFlags(st.builtinFunctionName()));
            //if (!st.userFunction()->execute(s, err))
            //    return false;
            //list << s.returnValue();
            break;
        }
        }
    }
    return false;
}

bool PretexFunction::isEmpty() const
{
    return !isValid() || mbody.isEmpty();
}

bool PretexFunction::isValid() const
{
    return !mname.isEmpty() && mobligArgCount > 0;
}

QString PretexFunction::name() const
{
    return mname;
}

int PretexFunction::obligatoryArgumentCount()
{
    return mobligArgCount;
}

int PretexFunction::optionalArgumentCount()
{
    return moptArgCount;
}

void PretexFunction::setBody(const QList<PretexStatement> &list)
{
    mbody = list;
}

/*============================== Public operators ==========================*/

bool PretexFunction::operator!=(const PretexFunction &other) const
{
    return !(*this == other);
}

PretexFunction &PretexFunction::operator= (const PretexFunction &other)
{
    mname = other.mname;
    mobligArgCount = other.mobligArgCount;
    moptArgCount = other.moptArgCount;
    mbody = other.mbody;
    return *this;
}

bool PretexFunction::operator== (const PretexFunction &other) const
{
    return mname == other.mname && mobligArgCount == other.mobligArgCount && moptArgCount == other.moptArgCount
            && mbody == other.mbody;
}

/*============================== Public friend operators ===================*/

QDataStream &operator<< (QDataStream &s, const PretexFunction &f)
{
    QVariantMap m;
    m.insert("name", f.mname);
    m.insert("obligatory_argument_count", f.mobligArgCount);
    m.insert("optional_argument_count", f.moptArgCount);
    QVariantList list;
    foreach (const PretexStatement &st, f.mbody)
        list << QVariant::fromValue(st);
    m.insert("statement_list", list);
    s << m;
    return s;
}

QDataStream &operator>> (QDataStream &s, PretexFunction &f)
{
    QVariantMap m;
    s >> m;
    QList<PretexStatement> list;
    foreach (const QVariant &v, m.value("statement_list").toList()) list << v.value<PretexStatement>();
    f = PretexFunction(m.value("name").toString(), m.value("obligatory_argument_count").toInt(),
                       m.value("optional_argument_count").toInt(), list);
    return s;
}

QDebug operator<< (QDebug dbg, const PretexFunction &)
{
    //TODO
    dbg.nospace() << "PretexFunction(UserFunction)";
    return dbg.space();
}
