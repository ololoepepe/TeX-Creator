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

#include "parser.h"

#include "token.h"
#include "tokendata.h"

#include <BeQtGlobal>
#include <BTextTools>

#include <QDebug>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

/*============================================================================
================================ TokenStack ==================================
============================================================================*/

class TokenStack : public QList< QPair<Token *, int> >
{
public:
    void append(Token *t, int s);
    void freeAll();
    int state() const;
    Token *takeLastToken();
};

/*============================================================================
================================ TokenStack ==================================
============================================================================*/

/*============================== Public methods ============================*/

void TokenStack::append(Token *t, int s)
{
    *this << qMakePair(t, s);
}

void TokenStack::freeAll()
{
    foreach (int i, bRangeR(size() - 1, 0))
        delete at(i).first;
    clear();
}

int TokenStack::state() const
{
    return !isEmpty() ? last().second : 0;
}

Token *TokenStack::takeLastToken()
{
    return !isEmpty() ? takeLast().first : 0;
}

/*============================================================================
================================ Parser ======================================
============================================================================*/

/*============================== Static private constants ==================*/

const int Parser::MaxState = 34;
const int Parser::MaxType = 21;
const char Parser::Table[][4] =
{
/*==============================================================================================================================================================
|    |  0   |  1   |  2   |  3   |  4   |  5   |  6   |  7   |  8   |  9   |  10  |  11  |  12  |  13  |  14  |  15  |  16  |  17  |  18  |  19  |  20  |  21  |
----------------------------------------------------------------------------------------------------------------------------------------------------------------
|    |  BS  |  FN  | SFN  | LBR  | RBR  | LBRK | RBRK | STR  | INT  | REAL | SHRP | EOF  | Prog | Func | ObLs | OpLs |OpLsN |ObArg |OpArg |Sprog | Stmt |ArgNo |
==============================================================================================================================================================*/
/*0*/  "S2 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "   ", "1  ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*1*/  "S2 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "R0 ", "3  ", "1  ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*2*/  "   ", "S4 ", "S5 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*3*/  "R1 ", "R1 ", "R1 ", "R1 ", "R1 ", "R1 ", "R1 ", "R1 ", "R1 ", "R1 ", "R1 ", "R1 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*4*/  "   ", "   ", "   ", "S8 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "6  ", "   ", "   ", "7  ", "   ", "   ", "   ", "   ",
/*5*/  "   ", "   ", "   ", "S9 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*6*/  "R6 ", "R6 ", "R6 ", "R6 ", "R6 ", "S13", "R6 ", "R6 ", "R6 ", "R6 ", "R6 ", "R6 ", "   ", "   ", "   ", "10 ", "11 ", "   ", "12 ", "   ", "   ", "   ",
/*7*/  "R4 ", "R4 ", "R4 ", "S8 ", "R4 ", "R4 ", "R4 ", "R4 ", "R4 ", "R4 ", "R4 ", "R4 ", "   ", "   ", "14 ", "   ", "   ", "7  ", "   ", "   ", "   ", "   ",
/*8*/  "S2 ", "R12", "R12", "R12", "R12", "R12", "R12", "S19", "S20", "S21", "S22", "R12", "   ", "17 ", "   ", "   ", "   ", "   ", "   ", "15 ", "16 ", "18 ",
/*9*/  "   ", "S23", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*10*/ "R2 ", "R2 ", "R2 ", "R2 ", "R2 ", "R2 ", "R2 ", "R2 ", "R2 ", "R2 ", "R2 ", "R2 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*11*/ "R7 ", "R7 ", "R7 ", "R7 ", "R7 ", "R7 ", "R7 ", "R7 ", "R7 ", "R7 ", "R7 ", "R7 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*12*/ "R8 ", "R8 ", "R8 ", "R8 ", "R8 ", "S13", "R8 ", "R8 ", "R8 ", "R8 ", "R8 ", "R8 ", "   ", "   ", "   ", "   ", "24 ", "   ", "12 ", "   ", "   ", "   ",
/*13*/ "S2 ", "R12", "R12", "R12", "R12", "R12", "R12", "S19", "S20", "S21", "S22", "R12", "   ", "17 ", "   ", "   ", "   ", "   ", "   ", "25 ", "16 ", "18 ",
/*14*/ "R5 ", "R5 ", "R5 ", "R5 ", "R5 ", "R5 ", "R5 ", "R5 ", "R5 ", "R5 ", "R5 ", "R5 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*15*/ "   ", "   ", "   ", "   ", "S26", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*16*/ "S2 ", "R12", "R12", "R12", "R12", "R12", "R12", "S19", "S20", "S21", "S22", "R12", "   ", "17 ", "   ", "   ", "   ", "   ", "   ", "27 ", "16 ", "18 ",
/*17*/ "R14", "R14", "R14", "R14", "R14", "R14", "R14", "R14", "R14", "R14", "R14", "R14", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*18*/ "R15", "R15", "R15", "R15", "R15", "R15", "R15", "R15", "R15", "R15", "R15", "R15", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*19*/ "R16", "R16", "R16", "R16", "R16", "R16", "R16", "R16", "R16", "R16", "R16", "R16", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*20*/ "R17", "R17", "R17", "R17", "R17", "R17", "R17", "R17", "R17", "R17", "R17", "R17", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*21*/ "R18", "R18", "R18", "R18", "R18", "R18", "R18", "R18", "R18", "R18", "R18", "R18", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*22*/ "S2 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "S28", "   ", "   ", "   ", "   ", "29 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*23*/ "   ", "   ", "   ", "   ", "S30", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*24*/ "R9 ", "R9 ", "R9 ", "R9 ", "R9 ", "R9 ", "R9 ", "R9 ", "R9 ", "R9 ", "R9 ", "R9 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*25*/ "   ", "   ", "   ", "   ", "   ", "   ", "S31", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*26*/ "R10", "R10", "R10", "R10", "R10", "R10", "R10", "R10", "R10", "R10", "R10", "R10", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*27*/ "R13", "R13", "R13", "R13", "R13", "R13", "R13", "R13", "R13", "R13", "R13", "R13", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*28*/ "R19", "R19", "R19", "R19", "R19", "R19", "R19", "R19", "R19", "R19", "R19", "R19", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*29*/ "R20", "R20", "R20", "R20", "R20", "R20", "R20", "R20", "R20", "R20", "R20", "R20", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*30*/ "R6 ", "R6 ", "R6 ", "S8 ", "R6 ", "S13", "R6 ", "R6 ", "R6 ", "R6 ", "R6 ", "R6 ", "   ", "   ", "32 ", "34 ", "11 ", "7  ", "12 ", "   ", "   ", "   ",
/*31*/ "R11", "R11", "R11", "R11", "R11", "R11", "R11", "R11", "R11", "R11", "R11", "R11", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*32*/ "R6 ", "R6 ", "R6 ", "R6 ", "R6 ", "S13", "R6 ", "R6 ", "R6 ", "R6 ", "R6 ", "R6 ", "   ", "   ", "   ", "33 ", "11 ", "   ", "12 ", "   ", "   ", "   ",
/*33*/ "R3 ", "R3 ", "R3 ", "R3 ", "R3 ", "R3 ", "R3 ", "R3 ", "R3 ", "R3 ", "R3 ", "R3 ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ",
/*34*/ "R21", "R21", "R21", "R21", "R21", "R21", "R21", "R21", "R21", "R21", "R21", "R21", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   "
};

/*============================== Public constructors =======================*/

Parser::Parser(const QList<Token> &tokens)
{
    mtokens = tokens;
}

/*============================== Public methods ============================*/

Token *Parser::parse(bool *ok, QString *err, Token *token) const
{
    if (mtokens.isEmpty())
        return bRet(ok, true, err, QString(), token, Token(), new Token(Token::Program_Token));
    TokenStack stack;
    int i = 0;
    while (true) {
        if (mtokens.size() == i) {
            stack.freeAll();
            return bRet(ok, false, err, tr("Unexpected end of token list", "error"), token, Token(), (Token *) 0);
        }
        const Token &t = mtokens.at(i);
        int x = 0;
        ShiftReduceChoice choice = chooseShiftReduce(stack.state(), t.type(), &x);
        switch (choice) {
        case StateChangeChoice: {
            stack.append(new Token(t), x);
            ++i;
            break;
        }
        case ReduceChoice: {
            bool b = false;
            Token *nt = reduce(stack, x, &b, err);
            if (!b) {
                delete nt;
                stack.freeAll();
                return bRet(ok, b, token, t, (Token *) 0);
            }
            if (nt->type() == Token::Program_Token && stack.isEmpty()) {
                if (i < mtokens.size() - 1)
                    return bRet(ok, false, err, tr("Unexpected error", "error"), token, t, (Token *) 0);
                return bRet(ok, true, err, QString(), token, Token(), nt);
            }
            choice = chooseShiftReduce(stack.state(), nt->type(), &x);
            if (ShiftChoice != choice)
                return bRet(ok, false, err, tr("Failed to find shift rule", "error"), token, t, (Token *) 0);
            stack.append(nt, x);
            break;
        }
        case ShiftChoice: {
            stack.freeAll();
            return bRet(ok, false, err, tr("Unexpected shift rule", "error"), token, t, (Token *) 0);
        }
        case ErrorChoice:
        default: {
            stack.freeAll();
            return bRet(ok, false, err, tr("Failed to find shift or reduce rule", "error"), token, t, (Token *) 0);
        }
        }
    }
    return bRet(ok, false, err, tr("Failed to finish parsing", "error"), token, Token(), (Token *) 0);
}

void Parser::setTokenList(const QList<Token> &tokens)
{
    mtokens = tokens;
}

QList<Token> Parser::tokenList() const
{
    return mtokens;
}

/*============================== Static private methods ====================*/

Parser::ShiftReduceChoice Parser::chooseShiftReduce(int state, Token::Type type, int *x)
{
    if (Token::Unknown_Token == type || state < 0 || state > MaxState)
        return bRet(x, -1, ErrorChoice);
    QString s = Table[state * (MaxType + 1) + type];
    BTextTools::removeTrailingSpaces(&s);
    if (s.isEmpty())
        return bRet(x, -1, ErrorChoice);
    ShiftReduceChoice c = ErrorChoice;
    if (s.startsWith("R"))
        c = ReduceChoice;
    else if (s.startsWith("S"))
        c = StateChangeChoice;
    else
        c = ShiftChoice;
    if (ShiftChoice != c) {
        s = s.mid(1);
        if (s.isEmpty())
            return bRet(x, -1, ErrorChoice);
    }
    bool b = false;
    int xx = s.toInt(&b);
    if (!b)
        return bRet(x, -1, ErrorChoice);
    return bRet(x, xx, c);
}

Token *Parser::reduceR0(TokenStack &)
{
    return new Token(Token::Program_Token);
}

Token *Parser::reduceR1(TokenStack &stack)
{
    if (stack.size() < 2)
        return 0;
    Token *program = stack.takeLastToken();
    Token *func = stack.takeLastToken();
    DATA_CAST(Program, program)->prependFunction(DATA_CAST(Function, func));
    delete func;
    return program;
}

Token *Parser::reduceR2(TokenStack &stack)
{
    if (stack.size() < 4)
        return 0;
    Token *optArgList = stack.takeLastToken();
    Token *obligArgList = stack.takeLastToken();
    Token *funcName = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *func = new Token(Token::Function_Token);
    Function_TokenData *data = DATA_CAST(Function, func);
    data->setName(DATA_CAST(String, funcName)->value());
    delete funcName;
    data->setObligatoryArguments(DATA_CAST(ArgList, obligArgList));
    delete obligArgList;
    data->setOptionalArguments(DATA_CAST(ArgList, optArgList));
    delete optArgList;
    return func;
}

Token *Parser::reduceR3(TokenStack &stack)
{
    if (stack.size() < 7)
        return 0;
    Token *optArgList = stack.takeLastToken();
    Token *obligArgList = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *funcName = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *specFuncName = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *func = new Token(Token::Function_Token);
    Function_TokenData *data = DATA_CAST(Function, func);
    data->setName(DATA_CAST(String, specFuncName)->value());
    delete specFuncName;
    Token *subprogram = new Token(Token::Subprogram_Token);
    Token *statement = new Token(Token::Statement_Token);
    DATA_CAST(Statement, statement)->setString(DATA_CAST(String, funcName)->value());
    delete funcName;
    DATA_CAST(Subprogram, subprogram)->prependStatement(DATA_CAST(Statement, statement));
    delete statement;
    DATA_CAST(ArgList, obligArgList)->prependArgument(DATA_CAST(Subprogram, subprogram));
    delete subprogram;
    data->setObligatoryArguments(DATA_CAST(ArgList, obligArgList));
    delete obligArgList;
    data->setOptionalArguments(DATA_CAST(ArgList, optArgList));
    delete optArgList;
    return func;
}

Token *Parser::reduceR4(TokenStack &stack)
{
    if (stack.size() < 1)
        return 0;
    Token *arg = stack.takeLastToken();
    Token *argList = new Token(Token::ObligArgList_Token);
    DATA_CAST(ArgList, argList)->appendArgument(DATA_CAST(Subprogram, arg));
    delete arg;
    return argList;
}

Token *Parser::reduceR5(TokenStack &stack)
{
    if (stack.size() < 2)
        return 0;
    Token *argList = stack.takeLastToken();
    Token *arg = stack.takeLastToken();
    DATA_CAST(ArgList, argList)->prependArgument(DATA_CAST(Subprogram, arg));
    delete arg;
    return argList;
}

Token *Parser::reduceR6(TokenStack &)
{
    return new Token(Token::OptArgList_Token);
}

Token *Parser::reduceR7(TokenStack &stack)
{
    if (stack.size() < 1)
        return 0;
    Token *argListNE = stack.takeLastToken();
    Token *argList = new Token(Token::OptArgList_Token);
    DATA_CAST(ArgList, argList)->copyArguments(DATA_CAST(ArgList, argListNE));
    delete argListNE;
    return argList;
}

Token *Parser::reduceR8(TokenStack &stack)
{
    if (stack.size() < 1)
        return 0;
    Token *arg = stack.takeLastToken();
    Token *argList = new Token(Token::OptArgListNE_Token);
    DATA_CAST(ArgList, argList)->appendArgument(DATA_CAST(Subprogram, arg));
    delete arg;
    return argList;
}

Token *Parser::reduceR9(TokenStack &stack)
{
    if (stack.size() < 2)
        return 0;
    Token *argList = stack.takeLastToken();
    Token *arg = stack.takeLastToken();
    DATA_CAST(ArgList, argList)->prependArgument(DATA_CAST(Subprogram, arg));
    delete arg;
    return argList;
}

Token *Parser::reduceR10(TokenStack &stack)
{
    if (stack.size() < 3)
        return 0;
    delete stack.takeLastToken();
    Token *p = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *arg = new Token(Token::OblArg_Token);
    DATA_CAST(Subprogram, arg)->copyStatements(DATA_CAST(Subprogram, p));
    delete p;
    return arg;
}

Token *Parser::reduceR11(TokenStack &stack)
{
    if (stack.size() < 3)
        return 0;
    delete stack.takeLastToken();
    Token *p = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *arg = new Token(Token::OptArg_Token);
    DATA_CAST(Subprogram, arg)->copyStatements(DATA_CAST(Subprogram, p));
    delete p;
    return arg;
}

Token *Parser::reduceR12(TokenStack &)
{
    return new Token(Token::Subprogram_Token);
}

Token *Parser::reduceR13(TokenStack &stack)
{
    if (stack.size() < 2)
        return 0;
    Token *p = stack.takeLastToken();
    Token *s = stack.takeLastToken();
    DATA_CAST(Subprogram, p)->prependStatement(DATA_CAST(Statement, s));
    delete s;
    return p;
}

Token *Parser::reduceR14(TokenStack &stack)
{
    if (stack.size() < 1)
        return 0;
    Token *f = stack.takeLastToken();
    Token *s = new Token(Token::Statement_Token);
    DATA_CAST(Statement, s)->setFunction(DATA_CAST(Function, f));
    delete f;
    return s;
}

Token *Parser::reduceR15(TokenStack &stack)
{
    if (stack.size() < 1)
        return 0;
    Token *an = stack.takeLastToken();
    Token *s = new Token(Token::Statement_Token);
    DATA_CAST(Statement, s)->setArgumentNo(DATA_CAST(ArgumentNo, an));
    delete an;
    return s;
}

Token *Parser::reduceR16(TokenStack &stack)
{
    if (stack.size() < 1)
        return 0;
    Token *string = stack.takeLastToken();
    Token *s = new Token(Token::Statement_Token);
    DATA_CAST(Statement, s)->setString(DATA_CAST(String, string)->value());
    delete string;
    return s;
}

Token *Parser::reduceR17(TokenStack &stack)
{
    if (stack.size() < 1)
        return 0;
    Token *integer = stack.takeLastToken();
    Token *s = new Token(Token::Statement_Token);
    DATA_CAST(Statement, s)->setInteger(DATA_CAST(Integer, integer)->value());
    delete integer;
    return s;
}

Token *Parser::reduceR18(TokenStack &stack)
{
    if (stack.size() < 1)
        return 0;
    Token *real = stack.takeLastToken();
    Token *s = new Token(Token::Statement_Token);
    DATA_CAST(Statement, s)->setReal(DATA_CAST(Real, real)->value());
    delete real;
    return s;
}

Token *Parser::reduceR19(TokenStack &stack)
{
    if (stack.size() < 2)
        return 0;
    Token *integer = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *an = new Token(Token::ArgumentNo_Token);
    DATA_CAST(ArgumentNo, an)->setInteger(DATA_CAST(Integer, integer)->value());
    delete integer;
    return an;
}

Token *Parser::reduceR20(TokenStack &stack)
{
    if (stack.size() < 2)
        return 0;
    Token *f = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *an = new Token(Token::ArgumentNo_Token);
    DATA_CAST(ArgumentNo, an)->setFunction(DATA_CAST(Function, f));
    delete f;
    return an;
}

Token *Parser::reduceR21(TokenStack &stack)
{
    if (stack.size() < 6)
        return 0;
    Token *optArgList = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *funcName = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *specFuncName = stack.takeLastToken();
    delete stack.takeLastToken();
    Token *func = new Token(Token::Function_Token);
    Function_TokenData *data = DATA_CAST(Function, func);
    data->setName(DATA_CAST(String, specFuncName)->value());
    delete specFuncName;
    Token *subprogram = new Token(Token::Subprogram_Token);
    Token *statement = new Token(Token::Statement_Token);
    DATA_CAST(Statement, statement)->setString(DATA_CAST(String, funcName)->value());
    delete funcName;
    DATA_CAST(Subprogram, subprogram)->prependStatement(DATA_CAST(Statement, statement));
    delete statement;
    Token *obligArgList = new Token(Token::ObligArgList_Token);
    DATA_CAST(ArgList, obligArgList)->prependArgument(DATA_CAST(Subprogram, subprogram));
    delete subprogram;
    data->setObligatoryArguments(DATA_CAST(ArgList, obligArgList));
    delete obligArgList;
    data->setOptionalArguments(DATA_CAST(ArgList, optArgList));
    delete optArgList;
    return func;
}

Token *Parser::reduce(TokenStack &stack, int rule, bool *ok, QString *err)
{
    typedef Token *(*ReduceFunc)(TokenStack &);
    typedef QMap<int, ReduceFunc> FuncMap;
    init_once(FuncMap, funcMap, FuncMap()) {
        funcMap.insert(0, &reduceR0);
        funcMap.insert(1, &reduceR1);
        funcMap.insert(2, &reduceR2);
        funcMap.insert(3, &reduceR3);
        funcMap.insert(4, &reduceR4);
        funcMap.insert(5, &reduceR5);
        funcMap.insert(6, &reduceR6);
        funcMap.insert(7, &reduceR7);
        funcMap.insert(8, &reduceR8);
        funcMap.insert(9, &reduceR9);
        funcMap.insert(10, &reduceR10);
        funcMap.insert(11, &reduceR11);
        funcMap.insert(12, &reduceR12);
        funcMap.insert(13, &reduceR13);
        funcMap.insert(14, &reduceR14);
        funcMap.insert(15, &reduceR15);
        funcMap.insert(16, &reduceR16);
        funcMap.insert(17, &reduceR17);
        funcMap.insert(18, &reduceR18);
        funcMap.insert(19, &reduceR19);
        funcMap.insert(20, &reduceR20);
        funcMap.insert(21, &reduceR21);
    }
    ReduceFunc f = funcMap.value(rule);
    return f ? bRet(ok, true, err, QString(), f(stack)) :
               bRet(ok, false, err, QString("Failed to find reduce rule"), (Token *)0);
}
