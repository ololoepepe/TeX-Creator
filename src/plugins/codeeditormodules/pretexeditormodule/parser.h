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

#ifndef PARSER_H
#define PARSER_H

class TokenStack;

class QString;

#include "token.h"

#include <QCoreApplication>
#include <QList>

/*============================================================================
================================ Parser ======================================
============================================================================*/

class Parser
{
    Q_DECLARE_TR_FUNCTIONS(Parser)
private:
    static const int MaxState;
    static const int MaxType;
    static const char Table[][4];
private:
    QList<Token> mtokens;
public:
    explicit Parser(const QList<Token> &tokens = QList<Token>());
public:
    Token *parse(bool *ok = 0, QString *err = 0, Token *token = 0) const;
    void setTokenList(const QList<Token> &tokens);
    QList<Token> tokenList() const;
private:
    enum ShiftReduceChoice
    {
        ErrorChoice = 0,
        StateChangeChoice,
        ShiftChoice,
        ReduceChoice
    };
private:
    static ShiftReduceChoice chooseShiftReduce(int state, Token::Type type, int *x = 0);
    static Token *reduceR0(TokenStack &stack);
    static Token *reduceR1(TokenStack &stack);
    static Token *reduceR2(TokenStack &stack);
    static Token *reduceR3(TokenStack &stack);
    static Token *reduceR4(TokenStack &stack);
    static Token *reduceR5(TokenStack &stack);
    static Token *reduceR6(TokenStack &stack);
    static Token *reduceR7(TokenStack &stack);
    static Token *reduceR8(TokenStack &stack);
    static Token *reduceR9(TokenStack &stack);
    static Token *reduceR10(TokenStack &stack);
    static Token *reduceR11(TokenStack &stack);
    static Token *reduceR12(TokenStack &stack);
    static Token *reduceR13(TokenStack &stack);
    static Token *reduceR14(TokenStack &stack);
    static Token *reduceR15(TokenStack &stack);
    static Token *reduceR16(TokenStack &stack);
    static Token *reduceR17(TokenStack &stack);
    static Token *reduceR18(TokenStack &stack);
    static Token *reduceR19(TokenStack &stack);
    static Token *reduceR20(TokenStack &stack);
    static Token *reduceR21(TokenStack &stack);
    static Token *reduce(TokenStack &stack, int rule, bool *ok = 0, QString *err = 0);
};

#endif // PARSER_H
