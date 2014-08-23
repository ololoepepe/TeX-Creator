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

#include "tokendata.h"

#include "token.h"

#include <BeQt>

#include <QByteArray>
#include <QDebug>
#include <QList>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

/*============================================================================
================================ TokenData ===================================
============================================================================*/

/*============================== Public constructors =======================*/

TokenData::~TokenData()
{
    //
}

/*============================== Protected constructors ====================*/

TokenData::TokenData(Token::Type type) : Type(type)
{
    //
}

/*============================== Public methods ============================*/

TokenData *TokenData::clone() const
{
    return new TokenData(Type);
}

bool TokenData::compare(const TokenData *) const
{
    return true;
}

void TokenData::deserialize(const QByteArray &)
{
    //
}

QByteArray TokenData::serialize() const
{
    return QByteArray();
}

QString TokenData::toString() const
{
    return QString();
}

Token::Type TokenData::type() const
{
    return Type;
}

/*============================================================================
================================ Program_TokenData ===========================
============================================================================*/

/*============================== Public constructors =======================*/

Program_TokenData::~Program_TokenData()
{
    foreach (Function_TokenData *f, mfunctions)
        delete f;
}

/*============================== Private constructors ======================*/

Program_TokenData::Program_TokenData() :
    TokenData(Token::Program_Token)
{
    //
}

/*============================== Public methods ============================*/

void Program_TokenData::appendFunction(Function_TokenData *f)
{
    if (!f || mfunctions.contains(f))
        return;
    mfunctions << static_cast<Function_TokenData *>(f->clone());
}

TokenData *Program_TokenData::clone() const
{
    Program_TokenData *nd = new Program_TokenData;
    foreach (Function_TokenData *f, mfunctions)
        nd->appendFunction(f);
    return nd;
}

bool Program_TokenData::compare(const TokenData *other) const
{
    const Program_TokenData *o = static_cast<const Program_TokenData *>(other);
    if (mfunctions.size() != o->mfunctions.size())
        return false;
    foreach (int i, bRangeD(0, mfunctions.size() - 1)) {
        if (!mfunctions.at(i)->compare(o->mfunctions.at(i)))
            return false;
    }
    return true;
}

Function_TokenData *Program_TokenData::function(int index) const
{
    if (index < 0 || index >= mfunctions.size())
        return 0;
    return mfunctions.at(index);
}

int Program_TokenData::functionCount() const
{
    return mfunctions.size();
}

void Program_TokenData::prependFunction(Function_TokenData *f)
{
    if (!f || mfunctions.contains(f))
        return;
    mfunctions.prepend(static_cast<Function_TokenData *>(f->clone()));
}

QString Program_TokenData::toString() const
{
    return "function_count=" + QString::number(mfunctions.size());
}

/*============================================================================
================================ STRING_TokenData ============================
============================================================================*/

/*============================== Private constructors ======================*/

String_TokenData::String_TokenData(Token::Type type) :
    TokenData(type)
{
    //
}

/*============================== Public methods ============================*/

TokenData *String_TokenData::clone() const
{
    String_TokenData *nd = new String_TokenData(type());
    nd->mvalue = mvalue;
    return nd;
}

bool String_TokenData::compare(const TokenData *other) const
{
    return mvalue == static_cast<const String_TokenData *>(other)->mvalue;
}

void String_TokenData::setValue(const QString &s)
{
    mvalue = s;
}

QString String_TokenData::toString() const
{
    return "\"" + mvalue + "\"";
}

QString String_TokenData::value() const
{
    return mvalue;
}

/*============================================================================
================================ Integer_TokenData ===========================
============================================================================*/

/*============================== Private constructors ======================*/

Integer_TokenData::Integer_TokenData() :
    TokenData(Token::INTEGER_Token)
{
    //
}

/*============================== Public methods ============================*/

TokenData *Integer_TokenData::clone() const
{
    Integer_TokenData *nd = new Integer_TokenData;
    nd->mvalue = mvalue;
    return nd;
}

bool Integer_TokenData::compare(const TokenData *other) const
{
    return mvalue == static_cast<const Integer_TokenData *>(other)->mvalue;
}

void Integer_TokenData::setValue(int v)
{
    mvalue = v;
}

QString Integer_TokenData::toString() const
{
    return QString::number(mvalue);
}

int Integer_TokenData::value() const
{
    return mvalue;
}

/*============================================================================
================================ Real_TokenData ==============================
============================================================================*/

/*============================== Private constructors ======================*/

Real_TokenData::Real_TokenData() :
    TokenData(Token::REAL_Token)
{
    mvalue = 0.0;
}

/*============================== Public methods ============================*/

TokenData *Real_TokenData::clone() const
{
    Real_TokenData *nd = new Real_TokenData;
    nd->mvalue = mvalue;
    return nd;
}

bool Real_TokenData::compare(const TokenData *other) const
{
    return mvalue == static_cast<const Real_TokenData *>(other)->mvalue;
}

void Real_TokenData::setValue(double v)
{
    mvalue = v;
}

QString Real_TokenData::toString() const
{
    return QString::number(mvalue);
}

double Real_TokenData::value() const
{
    return mvalue;
}

/*============================================================================
================================ Function_TokenData ==========================
============================================================================*/

/*============================== Public constructors =======================*/

Function_TokenData::~Function_TokenData()
{
    delete mobligArguments;
    delete moptArguments;
}

/*============================== Private constructors ======================*/

Function_TokenData::Function_TokenData() :
    TokenData(Token::Function_Token)
{
    mobligArguments = 0;
    moptArguments = 0;
}

/*============================== Public methods ============================*/

TokenData *Function_TokenData::clone() const
{
    Function_TokenData *nd = new Function_TokenData;
    nd->mname = mname;
    nd->setObligatoryArguments(mobligArguments);
    nd->setOptionalArguments(moptArguments);
    return nd;
}

bool Function_TokenData::compare(const TokenData *other) const
{
    const Function_TokenData *o = static_cast<const Function_TokenData *>(other);
    if (mname != o->mname || obligatoryArgumentCount() != o->obligatoryArgumentCount()
            || optionalArgumentCount() != o->optionalArgumentCount()) {
        return false;
    }
    foreach (int i, bRangeD(0, obligatoryArgumentCount() - 1)) {
        if (!obligatoryArgument(i)->compare(o->obligatoryArgument(i)))
            return false;
    }
    foreach (int i, bRangeD(0, optionalArgumentCount() - 1)) {
        if (!optionalArgument(i)->compare(o->optionalArgument(i)))
            return false;
    }
    return true;
}

void Function_TokenData::deserialize(const QByteArray &data)
{
    QVariantMap m = BeQt::deserialize(data).toMap();
    mname.clear();
    delete mobligArguments;
    mobligArguments = 0;
    delete moptArguments;
    moptArguments = 0;
    mname = m.value("name").toString();
    ArgList_TokenData obla(Token::ObligArgList_Token);
    obla.deserialize(m.value("obl_args").toByteArray());
    setObligatoryArguments(&obla);
    ArgList_TokenData opta(Token::OptArgList_Token);
    opta.deserialize(m.value("opt_args").toByteArray());
    setOptionalArguments(&opta);
}

QString Function_TokenData::name() const
{
    return mname;
}

Subprogram_TokenData *Function_TokenData::obligatoryArgument(int index) const
{
    return mobligArguments ? mobligArguments->argument(index) : 0;
}

int Function_TokenData::obligatoryArgumentCount() const
{
    return mobligArguments ? mobligArguments->argumentCount() : 0;
}

ArgList_TokenData *Function_TokenData::obligatoryArguments() const
{
    return mobligArguments;
}

Subprogram_TokenData *Function_TokenData::optionalArgument(int index) const
{
    return moptArguments ? moptArguments->argument(index) : 0;
}

int Function_TokenData::optionalArgumentCount() const
{
    return moptArguments ? moptArguments->argumentCount() : 0;
}

ArgList_TokenData *Function_TokenData::optionalArguments() const
{
    return moptArguments;
}

QByteArray Function_TokenData::serialize() const
{
    QVariantMap m;
    m.insert("name", mname);
    m.insert("obl_args", mobligArguments->serialize());
    m.insert("opt_args", moptArguments->serialize());
    return BeQt::serialize(m);
}

void Function_TokenData::setName(const QString &s)
{
    mname = s;
}

void Function_TokenData::setName(String_TokenData *d)
{
    mname = d ? d->value() : QString();
}

void Function_TokenData::setObligatoryArguments(ArgList_TokenData *a)
{
    delete mobligArguments;
    mobligArguments = a ? static_cast<ArgList_TokenData *>(a->clone()) : 0;
}

void Function_TokenData::setOptionalArguments(ArgList_TokenData *a)
{
    delete moptArguments;
    moptArguments = a ? static_cast<ArgList_TokenData *>(a->clone()) : 0;
}

QString Function_TokenData::toString() const
{
    return "name=" + mname;
}

/*============================================================================
================================ ArgList_TokenData ===========================
============================================================================*/

/*============================== Public constructors =======================*/

ArgList_TokenData::~ArgList_TokenData()
{
    foreach (Subprogram_TokenData *p, marguments)
        delete p;
}

/*============================== Private constructors ======================*/

ArgList_TokenData::ArgList_TokenData(Token::Type type) :
    TokenData(type)
{
    //
}

/*============================== Public methods ============================*/

void ArgList_TokenData::appendArgument(Subprogram_TokenData *p)
{
    if (!p || marguments.contains(p))
        return;
    marguments << static_cast<Subprogram_TokenData *>(p->clone());
}

Subprogram_TokenData *ArgList_TokenData::argument(int index) const
{
    if (index < 0 || index >= marguments.size())
        return 0;
    return marguments.at(index);
}

int ArgList_TokenData::argumentCount() const
{
    return marguments.size();
}

TokenData *ArgList_TokenData::clone() const
{
    ArgList_TokenData *nd = new ArgList_TokenData(type());
    foreach (Subprogram_TokenData *p, marguments)
        nd->appendArgument(p);
    return nd;
}

bool ArgList_TokenData::compare(const TokenData *other) const
{
    const ArgList_TokenData *o = static_cast<const ArgList_TokenData *>(other);
    if (argumentCount() != o->argumentCount())
        return false;
    foreach (int i, bRangeD(0, argumentCount() - 1)) {
        if (!argument(i)->compare(o->argument(i)))
            return false;
    }
    return true;
}

void ArgList_TokenData::copyArguments(ArgList_TokenData *other)
{
    foreach (Subprogram_TokenData *p, marguments)
        delete p;
    marguments.clear();
    if (!other)
        return;
    foreach (Subprogram_TokenData *p, other->marguments)
        appendArgument(p);
}

void ArgList_TokenData::deserialize(const QByteArray &data)
{
    QVariantMap m = BeQt::deserialize(data).toMap();
    foreach (Subprogram_TokenData *p, marguments)
        delete p;
    marguments.clear();
    QVariantList list = m.value("arguments").toList();
    foreach (const QVariant &v, list) {
        Subprogram_TokenData sp(Token::Subprogram_Token);
        sp.deserialize(v.toByteArray());
        appendArgument(&sp);
    }
}

void ArgList_TokenData::prependArgument(Subprogram_TokenData *p)
{
    if (!p || marguments.contains(p))
        return;
    marguments.prepend(static_cast<Subprogram_TokenData *>(p->clone()));
}

QByteArray ArgList_TokenData::serialize() const
{
    QVariantMap m;
    QVariantList list;
    foreach (Subprogram_TokenData *sp, marguments)
        list << sp->serialize();
    m.insert("arguments", list);
    return BeQt::serialize(m);
}

QString ArgList_TokenData::toString() const
{
    return "argument_count=" + QString::number(marguments.size());
}

/*============================================================================
================================ Subprogram_TokenData ========================
============================================================================*/

/*============================== Public constructors =======================*/

Subprogram_TokenData::~Subprogram_TokenData()
{
    foreach (Statement_TokenData *s, mstatements)
        delete s;
}

/*============================== Private constructors ======================*/

Subprogram_TokenData::Subprogram_TokenData(Token::Type type) :
    TokenData(type)
{
    //
}

/*============================== Public methods ============================*/

void Subprogram_TokenData::appendStatement(Statement_TokenData *s)
{
    if (!s || mstatements.contains(s))
        return;
    mstatements << static_cast<Statement_TokenData *>(s->clone());
}

TokenData *Subprogram_TokenData::clone() const
{
    Subprogram_TokenData *nd = new Subprogram_TokenData(type());
    foreach (Statement_TokenData *s, mstatements)
        nd->appendStatement(s);
    return nd;
}

bool Subprogram_TokenData::compare(const TokenData *other) const
{
    const Subprogram_TokenData *o = static_cast<const Subprogram_TokenData *>(other);
    if (statementCount() != o->statementCount())
        return false;
    foreach (int i, bRangeD(0, statementCount() - 1)) {
        if (!statement(i)->compare(o->statement(i)))
            return false;
    }
    return true;
}

void Subprogram_TokenData::copyStatements(Subprogram_TokenData *other)
{
    foreach (Statement_TokenData *s, mstatements)
        delete s;
    mstatements.clear();
    if (!other)
        return;
    foreach (Statement_TokenData *s, other->mstatements)
        appendStatement(s);
}

void Subprogram_TokenData::deserialize(const QByteArray &data)
{
    QVariantMap m = BeQt::deserialize(data).toMap();
    foreach (Statement_TokenData *st, mstatements)
        delete st;
    mstatements.clear();
    QVariantList list = m.value("statements").toList();
    foreach (const QVariant &v, list) {
        Statement_TokenData st;
        st.deserialize(v.toByteArray());
        appendStatement(&st);
    }
}

void Subprogram_TokenData::prependStatement(Statement_TokenData *s)
{
    if (!s || mstatements.contains(s))
        return;
    mstatements.prepend(static_cast<Statement_TokenData *>(s->clone()));
}

QByteArray Subprogram_TokenData::serialize() const
{
    QVariantMap m;
    QVariantList list;
    foreach (Statement_TokenData *st, mstatements)
        list << st->serialize();
    m.insert("statements", list);
    return BeQt::serialize(m);
}

Statement_TokenData *Subprogram_TokenData::statement(int index) const
{
    if (index < 0 || index >= mstatements.size())
        return 0;
    return mstatements.at(index);
}

int Subprogram_TokenData::statementCount() const
{
    return mstatements.size();
}

QString Subprogram_TokenData::toString() const
{
    return "statement_count=" + QString::number(mstatements.size());
}

/*============================================================================
================================ Statement_TokenData =========================
============================================================================*/

/*============================== Public constructors =======================*/

Statement_TokenData::~Statement_TokenData()
{
    delete mfunction;
    delete margumentNo;
}

/*============================== Private constructors ======================*/

Statement_TokenData::Statement_TokenData() :
    TokenData(Token::Statement_Token)
{
    mfunction = 0;
    margumentNo = 0;
    clear();
    mtype = StringStatement;
}

/*============================== Public methods ============================*/

ArgumentNo_TokenData *Statement_TokenData::argumentNo() const
{
    return margumentNo;
}

TokenData *Statement_TokenData::clone() const
{
    Statement_TokenData *nd = new Statement_TokenData;
    switch (mtype) {
    case FunctionStatement:
        nd->setFunction(mfunction);
        break;
    case StringStatement:
        nd->setString(mstring);
        break;
    case IntegerStatement:
        nd->setInteger(minteger);
        break;
    case RealStatement:
        nd->setReal(mreal);
        break;
    case ArgumentNoStatement:
        nd->setArgumentNo(margumentNo);
        break;
    default:
        break;
    }
    return nd;
}

bool Statement_TokenData::compare(const TokenData *other) const
{
    const Statement_TokenData *o = static_cast<const Statement_TokenData *>(other);
    if (mtype != o->mtype)
        return false;
    switch (mtype) {
    case FunctionStatement:
        if (!mfunction->compare(o->mfunction))
            return false;
    case StringStatement:
        if (mstring != o->mstring)
            return false;
    case IntegerStatement:
        if (minteger != o->minteger)
            return false;
    case RealStatement:
        if (mreal != o->mreal)
            return false;
    case ArgumentNoStatement:
        if (!margumentNo->compare(o->margumentNo))
            return false;
    default:
        break;
    }
    return true;
}

void Statement_TokenData::deserialize(const QByteArray &data)
{
    QVariantMap m = BeQt::deserialize(data).toMap();
    delete mfunction;
    delete margumentNo;
    mfunction = 0;
    margumentNo = 0;
    mstring.clear();
    minteger = 0;
    mreal = 0.0;
    if (m.contains("function")) {
        Function_TokenData f;
        f.deserialize(m.value("function").toByteArray());
        setFunction(&f);
    } else if (m.contains("argument_no")) {
        ArgumentNo_TokenData a;
        a.deserialize(m.value("argument_no").toByteArray());
        setArgumentNo(&a);
    } else if (m.contains("string")) {
        mstring = m.value("string").toString();
    } else if (m.contains("integer")) {
        minteger = m.value("integer").toInt();
    } else if (m.contains("real")) {
        mreal = m.value("real").toDouble();
    }
}

Function_TokenData *Statement_TokenData::function() const
{
    return mfunction;
}

int Statement_TokenData::integer() const
{
    return minteger;
}

bool Statement_TokenData::isArgumentNo() const
{
    return ArgumentNoStatement == mtype;
}

bool Statement_TokenData::isFunction() const
{
    return FunctionStatement == mtype;
}

bool Statement_TokenData::isInteger() const
{
    return IntegerStatement == mtype;
}

bool Statement_TokenData::isReal() const
{
    return RealStatement == mtype;
}

bool Statement_TokenData::isString() const
{
    return StringStatement == mtype;
}

double Statement_TokenData::real() const
{
    return mreal;
}

QByteArray Statement_TokenData::serialize() const
{
    QVariantMap m;
    switch (mtype) {
    case FunctionStatement:
        m.insert("function", mfunction->serialize());
        break;
    case StringStatement:
        m.insert("string", mstring);
        break;
    case IntegerStatement:
        m.insert("integer", minteger);
        break;
    case RealStatement:
        m.insert("real", mreal);
        break;
    case ArgumentNoStatement:
        m.insert("argument_no", margumentNo->serialize());
        break;
    default:
        break;
    }
    return BeQt::serialize(m);
}

void Statement_TokenData::setArgumentNo(ArgumentNo_TokenData *a)
{
    clear();
    mtype = ArgumentNoStatement;
    margumentNo = a ? static_cast<ArgumentNo_TokenData *>(a->clone()) : 0;
}

void Statement_TokenData::setFunction(Function_TokenData *f)
{
    clear();
    mtype = FunctionStatement;
    mfunction = f ? static_cast<Function_TokenData *>(f->clone()) : 0;
}

void Statement_TokenData::setInteger(int v)
{
    clear();
    mtype = IntegerStatement;
    minteger = v;
}

void Statement_TokenData::setReal(double v)
{
    clear();
    mtype = RealStatement;
    mreal = v;
}

void Statement_TokenData::setString(const QString &s)
{
    clear();
    mtype = StringStatement;
    mstring = s;
}

Statement_TokenData::StatementType Statement_TokenData::statementType() const
{
    return mtype;
}

QString Statement_TokenData::string() const
{
    return mstring;
}

QString Statement_TokenData::toString() const
{
    switch (mtype) {
    case FunctionStatement:
        return "type=function, value=(" + mfunction->toString() + ")";
    case StringStatement:
        return "type=strin, value=" + mstring;
    case IntegerStatement:
        return "type=integet, value=" + QString::number(minteger);
    case RealStatement:
        return "type=real, value=" + QString::number(mreal);
    case ArgumentNoStatement:
        return "type=argment_no, value=(" + margumentNo->toString() + ")";
    default:
        break;
    }
    return QString();
}

/*============================== Private methods ===========================*/

void Statement_TokenData::clear()
{
    delete mfunction;
    mfunction = 0;
    minteger = -1;
    mreal = 0.0;
    delete margumentNo;
    margumentNo = 0;
}

/*============================================================================
================================ ArgumentNo_TokenData ========================
============================================================================*/

/*============================== Public constructors =======================*/

ArgumentNo_TokenData::~ArgumentNo_TokenData()
{
    delete mfunction;
}

/*============================== Private constructors ======================*/

ArgumentNo_TokenData::ArgumentNo_TokenData() :
    TokenData(Token::ArgumentNo_Token)
{
    mfunction = 0;
    minteger = -1;
}

/*============================== Public methods ============================*/

ArgumentNo_TokenData::ArgumentNoType ArgumentNo_TokenData::argumentNoType() const
{
    return mfunction ? FunctionArgumentNo : IntegerArgumentNo;
}

TokenData *ArgumentNo_TokenData::clone() const
{
    ArgumentNo_TokenData *nd = new ArgumentNo_TokenData;
    nd->minteger = minteger;
    nd->mfunction = mfunction ? static_cast<Function_TokenData *>(mfunction->clone()) : 0;
    return nd;
}

bool ArgumentNo_TokenData::compare(const TokenData *other) const
{
    const ArgumentNo_TokenData *o = static_cast<const ArgumentNo_TokenData *>(other);
    if (argumentNoType() != o->argumentNoType())
        return false;
    switch (argumentNoType()) {
    case FunctionArgumentNo:
        if (!mfunction->compare(o->mfunction))
            return false;
    case IntegerArgumentNo:
        if (minteger != o->minteger)
            return false;
    default:
        break;
    }
    return true;
}

void ArgumentNo_TokenData::deserialize(const QByteArray &data)
{
    QVariantMap m = BeQt::deserialize(data).toMap();
    delete mfunction;
    mfunction = 0;
    minteger = -1;
    if (m.contains("function")) {
        Function_TokenData f;
        f.deserialize(m.value("function").toByteArray());
        setFunction(&f);
    } else if (m.contains("integer")) {
        setInteger(m.value("integer").toInt());
    }
}

Function_TokenData *ArgumentNo_TokenData::function() const
{
    return mfunction;
}

int ArgumentNo_TokenData::integer() const
{
    return minteger;
}

bool ArgumentNo_TokenData::isFunction() const
{
    return mfunction;
}

bool ArgumentNo_TokenData::isInteger() const
{
    return !mfunction;
}

QByteArray ArgumentNo_TokenData::serialize() const
{
    QVariantMap m;
    if (mfunction)
        m.insert("function", mfunction->serialize());
    else
        m.insert("integer", minteger);
    return BeQt::serialize(m);
}

void ArgumentNo_TokenData::setFunction(Function_TokenData *f)
{
    delete mfunction;
    minteger = -1;
    mfunction = f ? static_cast<Function_TokenData *>(f->clone()) : 0;
}

void ArgumentNo_TokenData::setInteger(int v)
{
    delete mfunction;
    minteger = -1;
    mfunction = 0;
    minteger = (v >= 0) ? v : -1;
}

QString ArgumentNo_TokenData::toString() const
{
    if (mfunction)
        return "type=function, value=(" + mfunction->toString() + ")";
    else
        return "type=integer, value=" + QString::number(minteger);
}
