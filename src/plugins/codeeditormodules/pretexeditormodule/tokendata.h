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

#ifndef TOKENDATA_H
#define TOKENDATA_H

class Statement_TokenData;
class ArgList_TokenData;
class Subprogram_TokenData;
class Function_TokenData;
class ArgumentNo_TokenData;

#include "token.h"

#include <QString>
#include <QList>

#define DATA_CAST(Type, Token) (static_cast<Type##_TokenData *>(Token->data()))

/*============================================================================
================================ TokenData ===================================
============================================================================*/

class TokenData
{
public:
    virtual ~TokenData();
protected:
    explicit TokenData(Token::Type type);
public:
    Token::Type type() const;
    virtual TokenData *clone() const;
    virtual QString toString() const;
private:
    const Token::Type Type;
private:
    friend class Token;
};

/*============================================================================
================================ Program_TokenData ===========================
============================================================================*/

class Program_TokenData : public TokenData
{
public:
    ~Program_TokenData();
private:
    explicit Program_TokenData();
public:
    TokenData *clone() const;
    QString toString() const;
    void prependFunction(Function_TokenData *f);
    void appendFunction(Function_TokenData *f);
    Function_TokenData *function(int index) const;
    int functionCount() const;
private:
    QList<Function_TokenData *> mfunctions;
private:
    friend class Token;
};

/*============================================================================
================================ String_TokenData ============================
============================================================================*/

class String_TokenData : public TokenData
{
private:
    explicit String_TokenData(Token::Type type);
public:
    TokenData *clone() const;
    QString toString() const;
    void setValue(const QString &s);
    QString value() const;
private:
    QString mvalue;
private:
    friend class Token;
};

/*============================================================================
================================ Integer_TokenData ===========================
============================================================================*/

class Integer_TokenData : public TokenData
{
private:
    explicit Integer_TokenData();
public:
    TokenData *clone() const;
    QString toString() const;
    void setValue(int v);
    int value() const;
private:
    int mvalue;
private:
    friend class Token;
};

/*============================================================================
================================ Real_TokenData ==============================
============================================================================*/

class Real_TokenData : public TokenData
{
private:
    explicit Real_TokenData();
public:
    TokenData *clone() const;
    QString toString() const;
    void setValue(double v);
    double value() const;
private:
    double mvalue;
private:
    friend class Token;
};

/*============================================================================
================================ Function_TokenData ==========================
============================================================================*/

class Function_TokenData : public TokenData
{
public:
    ~Function_TokenData();
private:
    explicit Function_TokenData();
public:
    TokenData *clone() const;
    QString toString() const;
    void setName(const QString &s);
    void setName(String_TokenData *d);
    void setObligatoryArguments(ArgList_TokenData *a);
    void setOptionalArguments(ArgList_TokenData *a);
    QString name() const;
    ArgList_TokenData *obligatoryArguments() const;
    ArgList_TokenData *optionalArguments() const;
    Subprogram_TokenData *obligatoryArgument(int index) const;
    Subprogram_TokenData *optionalArgument(int index) const;
    int obligatoryArgumentCount() const;
    int optionalArgumentCount() const;
private:
    QString mname;
    ArgList_TokenData *mobligArguments;
    ArgList_TokenData *moptArguments;
private:
    friend class Token;
};

/*============================================================================
================================ ArgList_TokenData ===========================
============================================================================*/

class ArgList_TokenData : public TokenData
{
public:
    ~ArgList_TokenData();
private:
    explicit ArgList_TokenData(Token::Type type);
public:
    TokenData *clone() const;
    QString toString() const;
    void copyArguments(ArgList_TokenData *other);
    void prependArgument(Subprogram_TokenData *p);
    void appendArgument(Subprogram_TokenData *p);
    Subprogram_TokenData *argument(int index) const;
    int argumentCount() const;
private:
    QList<Subprogram_TokenData *> marguments;
private:
    friend class Token;
};

/*============================================================================
================================ Subprogram_TokenData ========================
============================================================================*/

class Subprogram_TokenData : public TokenData
{
public:
    ~Subprogram_TokenData();
private:
    explicit Subprogram_TokenData(Token::Type type);
public:
    TokenData *clone() const;
    QString toString() const;
    void copyStatements(Subprogram_TokenData *other);
    void prependStatement(Statement_TokenData *s);
    void appendStatement(Statement_TokenData *s);
    Statement_TokenData *statement(int index) const;
    int statementCount() const;
private:
    QList<Statement_TokenData *> mstatements;
private:
    friend class Token;
};

/*============================================================================
================================ Statement_TokenData =========================
============================================================================*/

class Statement_TokenData : public TokenData
{
public:
    enum StatementType
    {
        FunctionStatement,
        StringStatement,
        IntegerStatement,
        RealStatement,
        ArgumentNoStatement
    };
public:
    ~Statement_TokenData();
private:
    explicit Statement_TokenData();
public:
    TokenData *clone() const;
    QString toString() const;
    void setFunction(Function_TokenData *f);
    void setString(const QString &s);
    void setInteger(int v);
    void setReal(double v);
    void setArgumentNo(ArgumentNo_TokenData *a);
    Function_TokenData *function() const;
    QString string() const;
    int integer() const;
    double real() const;
    ArgumentNo_TokenData *argumentNo() const;
    StatementType statementType() const;
    bool isFunction() const;
    bool isString() const;
    bool isInteger() const;
    bool isReal() const;
    bool isArgumentNo() const;
private:
    void clear();
private:
    Function_TokenData *mfunction;
    QString mstring;
    int minteger;
    double mreal;
    ArgumentNo_TokenData *margumentNo;
    StatementType mtype;
private:
    friend class Token;
};

/*============================================================================
================================ ArgumentNo_TokenData ========================
============================================================================*/

class ArgumentNo_TokenData : public TokenData
{
public:
    enum ArgumentNoType
    {
        IntegerArgumentNo,
        FunctionArgumentNo
    };
public:
    ~ArgumentNo_TokenData();
private:
    explicit ArgumentNo_TokenData();
public:
    TokenData *clone() const;
    QString toString() const;
    void setInteger(int v);
    void setFunction(Function_TokenData *f);
    int integer() const;
    Function_TokenData *function() const;
    ArgumentNoType argumentNoType() const;
    bool isInteger() const;
    bool isFunction() const;
private:
    int minteger;
    Function_TokenData *mfunction;
private:
    friend class Token;
};

#endif // TOKENDATA_H
