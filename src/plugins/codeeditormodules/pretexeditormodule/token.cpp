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

#include "token.h"
#include "tokendata.h"

#include <QString>

#include <QDebug>

/*============================================================================
================================ Token =======================================
============================================================================*/

/*============================== Static public methods =====================*/

QString Token::typeToString(Type type, bool tokenWord)
{
    QString s;
    switch (type)
    {
    case STRING_Token:
        s = "STRING";
        break;
    case INTEGER_Token:
        s = "INTEGER";
        break;
    case REAL_Token:
        s = "REAL";
        break;
    case SHARP_Token:
        s = "SHARP";
        break;
    case FUNC_NAME_Token:
        s = "FUNC_NAME";
        break;
    case SPEC_FUNC_NAME_Token:
        s = "SPEC_FUNC_NAME";
        break;
    case BACKSLASH_Token:
        s = "BACKSLASH";
        break;
    case LBRACE_Token:
        s = "LBRACE";
        break;
    case RBRACE_Token:
        s = "RBRACE";
        break;
    case LBRACKET_Token:
        s = "LBRACKET";
        break;
    case RBRACKET_Token:
        s = "RBRACKET";
        break;
    case EOF_Token:
        s = "EOF";
        break;
    case Program_Token:
        s = "Program";
        break;
    case OblArg_Token:
        s = "ObligArg";
        break;
    case OptArg_Token:
        s = "OptArg";
        break;
    case Function_Token:
        s = "Function";
        break;
    case ObligArgList_Token:
        s = "ObligArgList";
        break;
    case OptArgList_Token:
        s = "OptArgList";
        break;
    case OptArgListNE_Token:
        s = "OptArgListNE";
        break;
    case Subprogram_Token:
        s = "Subprogram";
        break;
    case Statement_Token:
        s = "Statement";
        break;
    case ArgumentNo_Token:
        s = "ArgumentNo";
        break;
    case Unknown_Token:
    default:
        s = "Unknown";
        break;
    }
    if (tokenWord)
        s += " Token";
    return s;
}

/*============================== Public constructors =======================*/

Token::Token(Type type, int position)
{
    mdata = createData(type);
    mpos = (position >= 0) ? position : -1;
}

Token::Token(const Token &other)
{
    mdata = 0;
    *this = other;
}

Token::~Token()
{
    delete mdata;
}

/*============================== Public methods ============================*/

Token::Type Token::type() const
{
    return mdata ? mdata->type() : Unknown_Token;
}

int Token::position() const
{
    return mpos;
}

TokenData *Token::data() const
{
    return mdata;
}

QString Token::toString() const
{
    return typeToString(type()) + (data() ? (": " + data()->toString()) : QString());
}

/*============================== Public operators ==========================*/

Token &Token::operator= (const Token &other)
{
    delete mdata;
    mdata = other.data() ? other.data()->clone() : 0;
    mpos = other.mpos;
    return *this;
}

/*============================== Static private methods ====================*/

TokenData *Token::createData(Type type)
{
    switch (type)
    {
    case STRING_Token:
    case FUNC_NAME_Token:
        return new String_TokenData(type);
    case SPEC_FUNC_NAME_Token:
        return new String_TokenData(type);
    case INTEGER_Token:
        return new Integer_TokenData;
    case REAL_Token:
        return new Real_TokenData;
    case SHARP_Token:
    case BACKSLASH_Token:
    case LBRACE_Token:
    case RBRACE_Token:
    case LBRACKET_Token:
    case RBRACKET_Token:
    case EOF_Token:
        return new TokenData(type);
    case Program_Token:
        return new Program_TokenData;
    case Subprogram_Token:
    case OblArg_Token:
    case OptArg_Token:
        return new Subprogram_TokenData(type);
    case Statement_Token:
        return new Statement_TokenData;
    case Function_Token:
        return new Function_TokenData;
    case ObligArgList_Token:
    case OptArgList_Token:
    case OptArgListNE_Token:
        return new ArgList_TokenData(type);
    case ArgumentNo_Token:
        return new ArgumentNo_TokenData;
    case Token::Unknown_Token:
    default:
        return 0;
    }
}
