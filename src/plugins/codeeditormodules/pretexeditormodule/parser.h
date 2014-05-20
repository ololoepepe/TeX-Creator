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

#ifndef PARSER_H
#define PARSER_H

class QString;

#include "token.h"

#include <QList>
#include <QCoreApplication>

/*============================================================================
================================ Parser ======================================
============================================================================*/

class Parser
{
    Q_DECLARE_TR_FUNCTIONS(Parser)
public:
    explicit Parser(const QList<Token> &tokens = QList<Token>());
public:
    void setTokenList(const QList<Token> &tokens);
    QList<Token> tokenList() const;
    Token *parse(bool *ok = 0, QString *err = 0, Token *token = 0) const;
private:
    QList<Token> mtokens;
};

#endif // PARSER_H