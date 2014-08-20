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

#include "pretexfunction.h"

#include "executionmodule.h"
#include "executionstack.h"
#include "pretexbuiltinfunction.h"
#include "tokendata.h"
#include "token.h"

#include <BeQtGlobal>

#include <QDataStream>
#include <QDebug>
#include <QList>
#include <QString>
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
                               const Token &body)
{
    if (name.isEmpty() || PretexBuiltinFunction::isBuiltinFunction(name) || obligatoryArgumentCount <= 0
            || body.type() != Token::Subprogram_Token) {
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

const Token &PretexFunction::body() const
{
    return mbody;
}

void PretexFunction::clear()
{
    mname.clear();
    mobligArgCount = 0;
    moptArgCount = 0;
    mbody = Token();
}

bool PretexFunction::execute(ExecutionStack *stack, Function_TokenData *f, QString *err)
{
    if (!isValid())
        return bRet(err, tr("Attempted to execute invalid function", "error"), false);
    int oblArgCount = obligatoryArgumentCount();
    if (f->obligatoryArgumentCount() != oblArgCount)
        return bRet(err, tr("Argument count mismatch:", "error") + " " + name(), false);
    int optArgCount = optionalArgumentCount();
    if (optArgCount >= 0 && f->optionalArgumentCount() > optArgCount)
        return bRet(err, tr("Argument count mismatch:", "error") + " " + name(), false);
    QList<PretexVariant> oblArgs;
    foreach (int i, bRangeD(0, f->obligatoryArgumentCount() - 1))
    {
        bool b = false;
        PretexVariant a = ExecutionModule::executeSubprogram(stack, f->obligatoryArgument(i), "", &b, err);
        if (!b)
            return false;
        oblArgs << a;
    }
    QList<PretexVariant> optArgs;
    foreach (int i, bRangeD(0, f->optionalArgumentCount() - 1))
    {
        bool b = false;
        PretexVariant a = ExecutionModule::executeSubprogram(stack, f->optionalArgument(i), "", &b, err);
        if (!b)
            return false;
        optArgs << a;
    }
    ExecutionStack s(oblArgs, optArgs, name(), stack);
    bool b = false;
    PretexVariant v = ExecutionModule::executeSubprogram(&s, DATA_CAST(Subprogram, &mbody), f->name(), &b, err);
    if (!b)
        return false;
    stack->setReturnValue(v);
    return bRet(err, QString(), true);
}

bool PretexFunction::isEmpty() const
{
    return !isValid() || !DATA_CAST(Subprogram, &mbody)->statementCount();
}

bool PretexFunction::isValid() const
{
    return !mname.isEmpty() && mobligArgCount > 0 && mbody.type() == Token::Subprogram_Token;
}

int PretexFunction::maxArgCount() const
{
    return (moptArgCount >= 0) ? (mobligArgCount + moptArgCount) : -1;
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

void PretexFunction::setBody(const Token &t)
{
    mbody = (t.type() == Token::Subprogram_Token) ? t : Token();
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
    m.insert("body", f.mbody.serialize());
    s << m;
    return s;
}

QDataStream &operator>> (QDataStream &s, PretexFunction &f)
{
    QVariantMap m;
    s >> m;
    Token t(Token::Subprogram_Token);
    t.deserialize(m.value("body").toByteArray());
    f = PretexFunction(m.value("name").toString(), m.value("obligatory_argument_count").toInt(),
                       m.value("optional_argument_count").toInt(), t);
    return s;
}

QDebug operator<< (QDebug dbg, const PretexFunction &)
{
    //TODO
    dbg.nospace() << "PretexFunction(UserFunction)";
    return dbg.space();
}
