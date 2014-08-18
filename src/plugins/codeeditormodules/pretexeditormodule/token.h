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

#ifndef TOKEN_H
#define TOKEN_H

class TokenData;

class QString;
class QByteArray;

/*============================================================================
================================ Token =======================================
============================================================================*/

class Token
{
public:
    enum Type
    {
        Unknown_Token = -1,
        BACKSLASH_Token = 0,
        FUNC_NAME_Token,
        SPEC_FUNC_NAME_Token,
        LBRACE_Token,
        RBRACE_Token,
        LBRACKET_Token,
        RBRACKET_Token,
        STRING_Token,
        INTEGER_Token,
        REAL_Token,
        SHARP_Token,
        EOF_Token,
        Program_Token,
        Function_Token,
        ObligArgList_Token,
        OptArgList_Token,
        OptArgListNE_Token,
        OblArg_Token,
        OptArg_Token,
        Subprogram_Token,
        Statement_Token,
        ArgumentNo_Token
    };
public:
    static QString typeToString(Type type, bool tokenWord = true);
public:
    explicit Token(Type type = Unknown_Token, int position = -1);
    Token(const Token &other);
    ~Token();
public:
    Type type() const;
    int position() const;
    TokenData *data() const;
    QString toString() const;
    QByteArray serialize() const;
    void deserialize(const QByteArray &data);
public:
    Token &operator= (const Token &other);
    bool operator== (const Token &other) const;
private:
    static TokenData *createData(Type type);
private:
    TokenData *mdata;
    int mpos;
};

#endif // TOKEN_H
