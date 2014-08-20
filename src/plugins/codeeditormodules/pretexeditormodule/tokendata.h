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

#ifndef TOKENDATA_H
#define TOKENDATA_H

class ArgList_TokenData;
class ArgumentNo_TokenData;
class Function_TokenData;
class Statement_TokenData;
class Subprogram_TokenData;

class QByteArray;

#include "token.h"

#include <QList>
#include <QString>

#define DATA_CAST(Type, Token) (static_cast<Type##_TokenData *>((Token)->data()))

/*============================================================================
================================ TokenData ===================================
============================================================================*/

class TokenData
{
private:
    const Token::Type Type;
public:
    virtual ~TokenData();
protected:
    explicit TokenData(Token::Type type);
public:
    virtual TokenData *clone() const;
    virtual bool compare(const TokenData *other) const;
    virtual void deserialize(const QByteArray &data);
    virtual QByteArray serialize() const;
    virtual QString toString() const;
    Token::Type type() const;
private:
    friend class Token;
};

/*============================================================================
================================ Program_TokenData ===========================
============================================================================*/

class Program_TokenData : public TokenData
{
private:
    QList<Function_TokenData *> mfunctions;
public:
    ~Program_TokenData();
private:
    explicit Program_TokenData();
public:
    void appendFunction(Function_TokenData *f);
    TokenData *clone() const;
    bool compare(const TokenData *other) const;
    Function_TokenData *function(int index) const;
    int functionCount() const;
    void prependFunction(Function_TokenData *f);
    QString toString() const;
private:
    friend class Token;
};

/*============================================================================
================================ String_TokenData ============================
============================================================================*/

class String_TokenData : public TokenData
{
private:
    QString mvalue;
private:
    explicit String_TokenData(Token::Type type);
public:
    TokenData *clone() const;
    bool compare(const TokenData *other) const;
    void setValue(const QString &s);
    QString toString() const;
    QString value() const;
private:
    friend class Token;
};

/*============================================================================
================================ Integer_TokenData ===========================
============================================================================*/

class Integer_TokenData : public TokenData
{
private:
    int mvalue;
private:
    explicit Integer_TokenData();
public:
    TokenData *clone() const;
    bool compare(const TokenData *other) const;
    void setValue(int v);
    QString toString() const;
    int value() const;
private:
    friend class Token;
};

/*============================================================================
================================ Real_TokenData ==============================
============================================================================*/

class Real_TokenData : public TokenData
{
private:
    double mvalue;
private:
    explicit Real_TokenData();
public:
    TokenData *clone() const;
    bool compare(const TokenData *other) const;
    void setValue(double v);
    QString toString() const;
    double value() const;
private:
    friend class Token;
};

/*============================================================================
================================ Function_TokenData ==========================
============================================================================*/

class Function_TokenData : public TokenData
{
private:
    QString mname;
    ArgList_TokenData *mobligArguments;
    ArgList_TokenData *moptArguments;
public:
    ~Function_TokenData();
private:
    explicit Function_TokenData();
public:
    TokenData *clone() const;
    bool compare(const TokenData *other) const;
    void deserialize(const QByteArray &data);
    QByteArray serialize() const;
    void setName(const QString &s);
    void setName(String_TokenData *d);
    void setObligatoryArguments(ArgList_TokenData *a);
    void setOptionalArguments(ArgList_TokenData *a);
    QString toString() const;
    QString name() const;
    Subprogram_TokenData *obligatoryArgument(int index) const;
    int obligatoryArgumentCount() const;
    ArgList_TokenData *obligatoryArguments() const;
    Subprogram_TokenData *optionalArgument(int index) const;
    int optionalArgumentCount() const;
    ArgList_TokenData *optionalArguments() const;
private:
    friend class ArgumentNo_TokenData;
    friend class Statement_TokenData;
    friend class Token;
};

/*============================================================================
================================ ArgList_TokenData ===========================
============================================================================*/

class ArgList_TokenData : public TokenData
{
private:
    QList<Subprogram_TokenData *> marguments;
public:
    ~ArgList_TokenData();
private:
    explicit ArgList_TokenData(Token::Type type);
public:
    void appendArgument(Subprogram_TokenData *p);
    Subprogram_TokenData *argument(int index) const;
    int argumentCount() const;
    TokenData *clone() const;
    bool compare(const TokenData *other) const;
    void copyArguments(ArgList_TokenData *other);
    void deserialize(const QByteArray &data);
    void prependArgument(Subprogram_TokenData *p);
    QByteArray serialize() const;
    QString toString() const;
private:
    friend class Function_TokenData;
    friend class Token;
};

/*============================================================================
================================ Subprogram_TokenData ========================
============================================================================*/

class Subprogram_TokenData : public TokenData
{
private:
    QList<Statement_TokenData *> mstatements;
public:
    ~Subprogram_TokenData();
private:
    explicit Subprogram_TokenData(Token::Type type);
public:
    void appendStatement(Statement_TokenData *s);
    TokenData *clone() const;
    bool compare(const TokenData *other) const;
    void copyStatements(Subprogram_TokenData *other);
    void deserialize(const QByteArray &data);
    void prependStatement(Statement_TokenData *s);
    QByteArray serialize() const;
    Statement_TokenData *statement(int index) const;
    int statementCount() const;
    QString toString() const;
private:
    friend class ArgList_TokenData;
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
private:
    ArgumentNo_TokenData *margumentNo;
    Function_TokenData *mfunction;
    int minteger;
    double mreal;
    QString mstring;
    StatementType mtype;
public:
    ~Statement_TokenData();
private:
    explicit Statement_TokenData();
public:
    ArgumentNo_TokenData *argumentNo() const;
    TokenData *clone() const;
    bool compare(const TokenData *other) const;
    void deserialize(const QByteArray &data);
    Function_TokenData *function() const;
    int integer() const;
    bool isArgumentNo() const;
    bool isFunction() const;
    bool isInteger() const;
    bool isReal() const;
    bool isString() const;
    double real() const;
    QByteArray serialize() const;
    void setArgumentNo(ArgumentNo_TokenData *a);
    void setFunction(Function_TokenData *f);
    void setInteger(int v);
    void setReal(double v);
    void setString(const QString &s);
    StatementType statementType() const;
    QString string() const;
    QString toString() const;
private:
    void clear();
private:
    friend class Subprogram_TokenData;
    friend class Token;
};

/*============================================================================
================================ ArgumentNo_TokenData ========================
============================================================================*/

class ArgumentNo_TokenData : public TokenData
{
private:
    Function_TokenData *mfunction;
    int minteger;
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
    ArgumentNoType argumentNoType() const;
    TokenData *clone() const;
    bool compare(const TokenData *other) const;
    void deserialize(const QByteArray &data);
    Function_TokenData *function() const;
    int integer() const;
    bool isFunction() const;
    bool isInteger() const;
    QByteArray serialize() const;
    void setFunction(Function_TokenData *f);
    void setInteger(int v);
    QString toString() const;
private:
    friend class Statement_TokenData;
    friend class Token;
};

#endif // TOKENDATA_H
