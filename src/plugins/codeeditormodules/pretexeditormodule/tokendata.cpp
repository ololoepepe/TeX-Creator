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

#include "tokendata.h"
#include "token.h"

#include <QList>
#include <QString>

#include <QDebug>

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

Token::Type TokenData::type() const
{
    return Type;
}

TokenData *TokenData::clone() const
{
    return new TokenData(Type);
}

QString TokenData::toString() const
{
    return QString();
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

TokenData *Program_TokenData::clone() const
{
    Program_TokenData *nd = new Program_TokenData;
    foreach (Function_TokenData *f, mfunctions)
        nd->appendFunction(f);
    return nd;
}

QString Program_TokenData::toString() const
{
    return "function_count=" + QString::number(mfunctions.size());
}

void Program_TokenData::prependFunction(Function_TokenData *f)
{
    if (!f || mfunctions.contains(f))
        return;
    mfunctions.prepend(static_cast<Function_TokenData *>(f->clone()));
}

void Program_TokenData::appendFunction(Function_TokenData *f)
{
    if (!f || mfunctions.contains(f))
        return;
    mfunctions << static_cast<Function_TokenData *>(f->clone());
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

QString String_TokenData::toString() const
{
    return "\"" + mvalue + "\"";
}

void String_TokenData::setValue(const QString &s)
{
    mvalue = s;
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

QString Integer_TokenData::toString() const
{
    return QString::number(mvalue);
}

void Integer_TokenData::setValue(int v)
{
    mvalue = v;
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

QString Real_TokenData::toString() const
{
    return QString::number(mvalue);
}

void Real_TokenData::setValue(double v)
{
    mvalue = v;
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

QString Function_TokenData::toString() const
{
    return "name=" + mname;
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

QString Function_TokenData::name() const
{
    return mname;
}

ArgList_TokenData *Function_TokenData::obligatoryArguments() const
{
    return mobligArguments;
}

ArgList_TokenData *Function_TokenData::optionalArguments() const
{
    return moptArguments;
}

Subprogram_TokenData *Function_TokenData::obligatoryArgument(int index) const
{
    return mobligArguments ? mobligArguments->argument(index) : 0;
}

Subprogram_TokenData *Function_TokenData::optionalArgument(int index) const
{
    return moptArguments ? moptArguments->argument(index) : 0;
}

int Function_TokenData::obligatoryArgumentCount() const
{
    return mobligArguments ? mobligArguments->argumentCount() : 0;
}

int Function_TokenData::optionalArgumentCount() const
{
    return moptArguments ? moptArguments->argumentCount() : 0;
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

TokenData *ArgList_TokenData::clone() const
{
    ArgList_TokenData *nd = new ArgList_TokenData(type());
    foreach (Subprogram_TokenData *p, marguments)
        nd->appendArgument(p);
    return nd;
}

QString ArgList_TokenData::toString() const
{
    return "argument_count=" + QString::number(marguments.size());
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

void ArgList_TokenData::prependArgument(Subprogram_TokenData *p)
{
    if (!p || marguments.contains(p))
        return;
    marguments.prepend(static_cast<Subprogram_TokenData *>(p->clone()));
}

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

TokenData *Subprogram_TokenData::clone() const
{
    Subprogram_TokenData *nd = new Subprogram_TokenData(type());
    foreach (Statement_TokenData *s, mstatements)
        nd->appendStatement(s);
    return nd;
}

QString Subprogram_TokenData::toString() const
{
    return "statement_count=" + QString::number(mstatements.size());
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

void Subprogram_TokenData::prependStatement(Statement_TokenData *s)
{
    if (!s || mstatements.contains(s))
        return;
    mstatements.prepend(static_cast<Statement_TokenData *>(s->clone()));
}

void Subprogram_TokenData::appendStatement(Statement_TokenData *s)
{
    if (!s || mstatements.contains(s))
        return;
    mstatements << static_cast<Statement_TokenData *>(s->clone());
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

TokenData *Statement_TokenData::clone() const
{
    Statement_TokenData *nd = new Statement_TokenData;
    switch (mtype)
    {
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

QString Statement_TokenData::toString() const
{
    //TODO
    return QString();
}

void Statement_TokenData::setFunction(Function_TokenData *f)
{
    clear();
    mtype = FunctionStatement;
    mfunction = f ? static_cast<Function_TokenData *>(f->clone()) : 0;
}

void Statement_TokenData::setString(const QString &s)
{
    clear();
    mtype = StringStatement;
    mstring = s;
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

void Statement_TokenData::setArgumentNo(ArgumentNo_TokenData *a)
{
    clear();
    mtype = ArgumentNoStatement;
    margumentNo = a ? static_cast<ArgumentNo_TokenData *>(a->clone()) : 0;
}

Function_TokenData *Statement_TokenData::function() const
{
    return mfunction;
}

QString Statement_TokenData::string() const
{
    return mstring;
}

int Statement_TokenData::integer() const
{
    return minteger;
}

double Statement_TokenData::real() const
{
    return mreal;
}

ArgumentNo_TokenData *Statement_TokenData::argumentNo() const
{
    return margumentNo;
}

Statement_TokenData::StatementType Statement_TokenData::statementType() const
{
    return mtype;
}

bool Statement_TokenData::isFunction() const
{
    return FunctionStatement == mtype;
}

bool Statement_TokenData::isString() const
{
    return StringStatement == mtype;
}

bool Statement_TokenData::isInteger() const
{
    return IntegerStatement == mtype;
}

bool Statement_TokenData::isReal() const
{
    return RealStatement == mtype;
}

bool Statement_TokenData::isArgumentNo() const
{
    return ArgumentNoStatement == mtype;
}

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

TokenData *ArgumentNo_TokenData::clone() const
{
    ArgumentNo_TokenData *nd = new ArgumentNo_TokenData;
    nd->minteger = minteger;
    nd->mfunction = mfunction ? static_cast<Function_TokenData *>(mfunction->clone()) : 0;
    return nd;
}

QString ArgumentNo_TokenData::toString() const
{
    //TODO
    return QString();
}

void ArgumentNo_TokenData::setInteger(int v)
{
    delete mfunction;
    minteger = -1;
    mfunction = 0;
    minteger = (v >= 0) ? v : -1;
}

void ArgumentNo_TokenData::setFunction(Function_TokenData *f)
{
    delete mfunction;
    minteger = -1;
    mfunction = f ? static_cast<Function_TokenData *>(f->clone()) : 0;
}

int ArgumentNo_TokenData::integer() const
{
    return minteger;
}

Function_TokenData *ArgumentNo_TokenData::function() const
{
    return mfunction;
}

ArgumentNo_TokenData::ArgumentNoType ArgumentNo_TokenData::argumentNoType() const
{
    return mfunction ? FunctionArgumentNo : IntegerArgumentNo;
}

bool ArgumentNo_TokenData::isInteger() const
{
    return !mfunction;
}

bool ArgumentNo_TokenData::isFunction() const
{
    return mfunction;
}
