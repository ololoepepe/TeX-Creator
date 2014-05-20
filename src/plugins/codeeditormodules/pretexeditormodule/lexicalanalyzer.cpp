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

#include "lexicalanalyzer.h"
#include "token.h"
#include "tokendata.h"
#include "pretexbuiltinfunction.h"

#include <BeQtGlobal>
#include <BDirTools>
#include <BeQt>

#include <QString>
#include <QStringList>
#include <QList>
#include <QStack>
#include <QChar>
#include <QMap>
#include <QRegExp>
#include <QFileInfo>

#include <QDebug>

/*============================================================================
================================ Rule ========================================
============================================================================*/

class Rule
{
public:
    typedef bool (*MatchFunction)(const QString &, int &);
    typedef Token (*ParceFunction)(const QString &, int);
public:
    explicit Rule();
    explicit Rule(MatchFunction mf, ParceFunction pf);
public:
    bool match(const QString &s, int &matchedLength) const;
    Token parse(const QString &s, int pos = -1) const;
public:
    MatchFunction matchFunction;
    ParceFunction parceFunction;
};

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool checkTokenOrder(const QList<Token> &tokens, Token::Type current, QString *err = 0)
{
    Token::Type previous = !tokens.isEmpty() ? tokens.last().type() : Token::Unknown_Token;
    switch (previous)
    {
    case Token::Unknown_Token:
        if (Token::BACKSLASH_Token != current)
            return bRet(err, QString("Expected backslash"), false);
        break;
    case Token::STRING_Token:
    case Token::INTEGER_Token:
    case Token::REAL_Token:
        switch (current)
        {
        case Token::FUNC_NAME_Token:
        case Token::SPEC_FUNC_NAME_Token:
            return bRet(err, QString("Unexpected function name"), false);
        case Token::LBRACE_Token:
            return bRet(err, QString("Unexpected opening brace"), false);
        case Token::LBRACKET_Token:
            return bRet(err, QString("Unexpected opening bracket"), false);
        default:
            break;
        }
        break;
    case Token::SHARP_Token:
        if (Token::BACKSLASH_Token != current && Token::INTEGER_Token != current)
            return bRet(err, QString("Expected integer or backslash"), false);
        break;
    case Token::BACKSLASH_Token:
        if (Token::FUNC_NAME_Token != current && Token::SPEC_FUNC_NAME_Token != current)
            return bRet(err, QString("Expected function name"), false);
        break;
    case Token::FUNC_NAME_Token:
        if (Token::LBRACE_Token != current && Token::RBRACE_Token != current)
            return bRet(err, QString("Expected opening or closing brace"), false);
        break;
    case Token::SPEC_FUNC_NAME_Token:
        if (Token::LBRACE_Token != current)
            return bRet(err, QString("Expected opening brace"), false);
        break;
    case Token::LBRACE_Token:
        switch (current)
        {
        case Token::SPEC_FUNC_NAME_Token:
            return bRet(err, QString("Unexpected function name"), false);
        case Token::LBRACE_Token:
            return bRet(err, QString("Unexpected opening brace"), false);
        case Token::LBRACKET_Token:
            return bRet(err, QString("Unexpected opening bracket"), false);
        case Token::RBRACKET_Token:
            return bRet(err, QString("Unexpected closing bracket"), false);
        default:
            break;
        }
        break;
    case Token::RBRACE_Token:
        if (Token::FUNC_NAME_Token == current || Token::SPEC_FUNC_NAME_Token == current)
            return bRet(err, QString("Unexpected function name"), false);
        break;
    case Token::LBRACKET_Token:
        switch (current)
        {
        case Token::FUNC_NAME_Token:
        case Token::SPEC_FUNC_NAME_Token:
            return bRet(err, QString("Unexpected function name"), false);
        case Token::LBRACE_Token:
            return bRet(err, QString("Unexpected opening brace"), false);
        case Token::RBRACE_Token:
            return bRet(err, QString("Unexpected closing brace"), false);
        default:
            break;
        }
        break;
    case Token::RBRACKET_Token:
        switch (current)
        {
        case Token::FUNC_NAME_Token:
        case Token::SPEC_FUNC_NAME_Token:
            return bRet(err, QString("Unexpected function name"), false);
        case Token::LBRACE_Token:
            return bRet(err, QString("Unexpected opening brace"), false);
        default:
            break;
        }
        break;
    default:
        break;
    }
    return bRet(err, QString(), true);
}

static QString replaceEscaped(QString s)
{
    int i = 0;
    bool escMod = false;
    while (i < s.length())
    {
        typedef QMap<QChar, QChar> CharMap;
        init_once(CharMap, charMap, CharMap())
        {
            charMap.insert('\\', '\\');
            charMap.insert('%', '%');
            charMap.insert('\"', '\"');
            charMap.insert('s', ' ');
            charMap.insert('t', '\t');
            charMap.insert('n', '\n');
        }
        if (charMap.contains(s.at(i)))
        {
            if (LexicalAnalyzer::isEscaped(s, i, s.at(i).toAscii()) ^ escMod)
            {
                if ('\\' == s.at(i))
                    escMod = !escMod;
                else
                    escMod = false;
                s.replace(i - 1, 2, charMap.value(s.at(i)));
                --i;
            }
        }
        else
        {
            escMod = false;
        }
        ++i;
    }
    return s;
}

static bool matchInput(const QString &s, QString &path, int &matchedLength)
{
    path.clear();
    matchedLength = 0;
    if (s.isEmpty())
        return false;
    if (s.at(0) != '\\')
        return false;
    int i = 1;
    while (i < s.length() && s.at(i).isSpace())
        ++i;
    if (s.length() == i)
        return false;
    if (s.mid(i, 5) != "input")
        return false;
    i += 5;
    while (i < s.length() && s.at(i).isSpace())
        ++i;
    if (s.length() == i)
        return false;
    if (s.at(i) != '{')
        return false;
    ++i;
    while (i < s.length() && s.at(i).isSpace())
        ++i;
    if (s.length() == i)
        return false;
    if (s.at(i) != '\"')
        return false;
    ++i;
    int start = i;
    while (i < s.length() && (s.at(i) != '\"' || LexicalAnalyzer::isEscaped(s, i, '\"')))
        ++i;
    if (s.length() == i || s.at(i) != '\"')
        return false;
    ++i;
    while (i < s.length() && s.at(i).isSpace())
        ++i;
    if (s.length() == i)
        return false;
    if (s.at(i) != '}')
        return false;
    path = s.mid(start, i - start - 1);
    matchedLength = i + 1;
    return true;
}

static bool matchFuncName2Args(const QString &s, int &matchedLength)
{
    return LexicalAnalyzer::matchFuncName(s, matchedLength);
}

static bool matchSharp(const QString &s, int &matchedLength)
{
    matchedLength = s.startsWith('#');
    return matchedLength;
}

static bool matchBackslash(const QString &s, int &matchedLength)
{
    matchedLength = s.startsWith('\\');
    return matchedLength;
}

static bool matchLeftBrace(const QString &s, int &matchedLength)
{
    matchedLength = s.startsWith('{');
    return matchedLength;
}

static bool matchRightBrace(const QString &s, int &matchedLength)
{
    matchedLength = s.startsWith('}');
    return matchedLength;
}

static bool matchLeftBracket(const QString &s, int &matchedLength)
{
    matchedLength = s.startsWith('[');
    return matchedLength;
}

static bool matchRightBracket(const QString &s, int &matchedLength)
{
    matchedLength = s.startsWith(']');
    return matchedLength;
}

static Token parceString(const QString &s, int pos)
{
    Token t(Token::STRING_Token, pos);
    static_cast<String_TokenData *>(t.data())->setValue(replaceEscaped(s.mid(1, s.length() - 2)));
    return t;
}

static Token parceFuncName(const QString &s, int pos)
{
    if (s.isEmpty())
        return Token();
    Token t(Token::FUNC_NAME_Token, pos);
    static_cast<String_TokenData *>(t.data())->setValue(s);
    return t;
}

static Token parceSpecFuncName(const QString &s, int pos)
{
    if (s.isEmpty())
        return Token();
    Token t(Token::SPEC_FUNC_NAME_Token, pos);
    static_cast<String_TokenData *>(t.data())->setValue(s);
    return t;
}

static Token parceSpecialSymbol(const QString &s, int pos)
{
    typedef QMap<QString, Token::Type> TokenTypeMap;
    init_once(TokenTypeMap, tokens, TokenTypeMap())
    {
        tokens.insert("#", Token::SHARP_Token);
        tokens.insert("\\", Token::BACKSLASH_Token);
        tokens.insert("{", Token::LBRACE_Token);
        tokens.insert("}", Token::RBRACE_Token);
        tokens.insert("[", Token::LBRACKET_Token);
        tokens.insert("]", Token::RBRACKET_Token);
    }
    return tokens.contains(s) ? Token(tokens.value(s), pos) : Token();
}

static Token parceReal(const QString &s, int pos)
{
    bool ok = false;
    double x = s.toDouble(&ok);
    if (!ok)
        return Token();
    Token t(Token::REAL_Token, pos);
    static_cast<Real_TokenData *>(t.data())->setValue(x);
    return t;
}

static Token parceInteger(const QString &s, int pos)
{
    bool ok = false;
    int x = s.toInt(&ok);
    if (!ok)
        return Token();
    Token t(Token::INTEGER_Token, pos);
    static_cast<Integer_TokenData *>(t.data())->setValue(x);
    return t;
}

/*============================================================================
================================ Rule ========================================
============================================================================*/

/*============================== Public constructors =======================*/

Rule::Rule()
{
    matchFunction = 0;
    parceFunction = 0;
}

Rule::Rule(MatchFunction mf, ParceFunction pf)
{
    matchFunction = mf;
    parceFunction = pf;
}

/*============================== Public methods ============================*/

bool Rule::match(const QString &s, int &matchedLength) const
{
    if (!matchFunction)
    {
        matchedLength = 0;
        return false;
    }
    return matchFunction(s, matchedLength);
}

Token Rule::parse(const QString &s, int pos) const
{
    return parceFunction ? parceFunction(s, pos) : Token();
}

/*============================================================================
================================ LexicalAnalyzer =============================
============================================================================*/

/*============================== Static public methods =====================*/

bool LexicalAnalyzer::isEscaped(const QString &s, int pos, const QChar &symbol)
{
    if (s.isEmpty() || pos < 0 || pos >= s.length())
        return false;
    if (s.at(pos) != symbol)
        return false;
    int n = 0;
    int i = pos - 1;
    while (i >= 0 && s.at(i) == '\\')
    {
        ++n;
        --i;
    }
    return (n % 2);
}

bool LexicalAnalyzer::matchString(const QString &s, int &matchedLength)
{
    matchedLength = 0;
    if (!s.startsWith('\"'))
        return false;
    int i = 1;
    while (i < s.length() && (s.at(i) != '\"' || isEscaped(s, i, '\"')))
        ++i;
    if (s.length() == i || s.at(i) != '\"')
        return false;
    matchedLength = i + 1;
    return true;
}

bool LexicalAnalyzer::matchFuncName(const QString &s, int &matchedLength, bool *builtin)
{
    matchedLength = 0;
    int i = -1;
    bool b = false;
    foreach (const QString &fn, PretexBuiltinFunction::normalFuncNames())
    {
        if (s.startsWith(fn))
        {
            b = true;
            i = fn.length();
            break;
        }
    }
    if (b)
    {
        static QList<QChar> chars = QList<QChar>() << '\\' << '#' << '{' << '}' << '[' << ']' << '\"';
        if (i < s.length() && !s.at(i).isSpace() && !chars.contains(s.at(i)))
            return bRet(builtin, false, false);
        matchedLength = i;
        return bRet(builtin, true, true);
    }
    else
    {
        if (!s.at(0).isLetter() && s.at(0) != '_')
            return bRet(builtin, false, false);
        int i = 1;
        while (i < s.length() && (s.at(i).isLetterOrNumber() || s.at(i) == '_'))
            ++i;
        matchedLength = i;
        return bRet(builtin, false, true);
    }
}

bool LexicalAnalyzer::matchSpecFuncName(const QString &s, int &matchedLength)
{
    matchedLength = 0;
    int i = -1;
    bool b = false;
    foreach (const QString &fn, PretexBuiltinFunction::specFuncNames())
    {
        if (s.startsWith(fn))
        {
            b = true;
            i = fn.length();
            break;
        }
    }
    if (!b || (i < s.length() && (s.at(i).isLetterOrNumber() || s.at(i) == '_')))
        return false;
    matchedLength = i;
    return true;
}

bool LexicalAnalyzer::matchReal(const QString &s, int &matchedLength)
{
    matchedLength = 0;
    QRegExp rx("\\-?[0-9]+\\.[0-9]+");
    int ind = rx.indexIn(s);
    if (ind)
        return false;
    matchedLength = rx.matchedLength();
    return true;
}

bool LexicalAnalyzer::matchInteger(const QString &s, int &matchedLength)
{
    matchedLength = 0;
    QRegExp rx("\\-?[0-9]+");
    int ind = rx.indexIn(s);
    if (ind)
        return false;
    matchedLength = rx.matchedLength();
    return true;
}

/*============================== Public constructors =======================*/

LexicalAnalyzer::LexicalAnalyzer(const QString &source, const QString &fileName, QTextCodec *codec)
{
    msource = source;
    mfileName = fileName;
    mcodec = codec;
}

/*============================== Public methods ============================*/

void LexicalAnalyzer::setSource(const QString source)
{
    msource = source;
}

void LexicalAnalyzer::setFileName(const QString &fileName)
{
    mfileName = fileName;
}

void LexicalAnalyzer::setCodec(QTextCodec *codec)
{
    mcodec = codec;
}

void LexicalAnalyzer::setCodecName(const QString &codecName)
{
    mcodec = BeQt::codec(codecName);
}

QString LexicalAnalyzer::source() const
{
    return msource;
}

QString LexicalAnalyzer::fileName() const
{
    return mfileName;
}

QTextCodec *LexicalAnalyzer::codec() const
{
    return mcodec;
}

QString LexicalAnalyzer::codecName() const
{
    return BeQt::codecName(mcodec);
}

QList<Token> LexicalAnalyzer::analyze(bool *ok, QString *err, int *pos, QString *fn)
{
    return analyze(msource, mfileName, mcodec, ok, err, pos, fn);
}

/*============================== Static private methods ====================*/

QList<Token> LexicalAnalyzer::analyze(QString s, const QString &fileName, QTextCodec *codec, bool *ok, QString *err,
                                      int *pos, QString *fn)
{
    typedef QList<Rule> RuleList;
    init_once(RuleList, rules, RuleList())
    {
        //String literal, priority=0
        rules << Rule(&matchString, &parceString);
        //Function name, priority=1
        rules << Rule(&matchFuncName2Args, &parceFuncName);
        //Special function name, priority=2
        rules << Rule(&matchSpecFuncName, &parceSpecFuncName);
        //Special symbols, priority=3
        rules << Rule(&matchSharp, &parceSpecialSymbol);
        rules << Rule(&matchBackslash, &parceSpecialSymbol);
        rules << Rule(&matchLeftBrace, &parceSpecialSymbol);
        rules << Rule(&matchRightBrace, &parceSpecialSymbol);
        rules << Rule(&matchLeftBracket, &parceSpecialSymbol);
        rules << Rule(&matchRightBracket, &parceSpecialSymbol);
        //Real literal, priority=4
        rules << Rule(&matchReal, &parceReal);
        //String literal, priority=5
        rules << Rule(&matchInteger, &parceInteger);
    }
    QList<Token> tokens;
    int p = 0;
    while (!s.isEmpty())
    {
        int ml = 0;
        int pp = -1;
        if (!removeNonterminals(s, ml, err, &pp))
            return bRet(ok, false, fn, fileName, pos, p + pp, tokens);
        if (s.isEmpty())
        {
            tokens << Token(Token::EOF_Token, p);
            return bRet(ok, true, err, fileName, pos, -1, tokens);
        }
        p += ml;
        QString path;
        ml = 0;
        if (matchInput(s, path, ml))
        {
            if (!fileName.isEmpty() && QFileInfo(path).isRelative())
                path = QFileInfo(fileName).path() + "/" + path;
            bool b = false;
            QString sub = BDirTools::readTextFile(path, codec, &b);
            if (!b)
                return bRet(ok, false, err, QString("Failed to load file"), pos, p, fn, fileName, tokens);
            b = false;
            QList<Token> list = analyze(sub, path, codec, &b, err, pos, fn);
            if (!b)
                return bRet(ok ,false, tokens);
            if (!list.isEmpty() && !checkTokenOrder(tokens, list.first().type(), err))
                return bRet(ok, false, pos, p, tokens);
            if (!list.isEmpty())
                list.removeLast();
            tokens << list;
            s.remove(0, ml);
            p += ml;
            continue;
        }
        bool matched = false;
        for (int i = 0; i < rules.size(); ++i)
        {
            const Rule &r = rules.at(i);
            int ml = 0;
            if (r.match(s, ml))
            {
                QString t = s.left(ml);
                for (int j = i + 1; j < rules.size(); ++j)
                {
                    const Rule &r = rules.at(j);
                    int ml = 0;
                    if (r.match(t, ml) && t.length() == ml)
                    {
                        Token token = r.parse(t, p);
                        if (!checkTokenOrder(tokens, token.type(), err))
                            return bRet(ok, false, pos, p, fn, fileName, tokens);
                        tokens << token;
                        s.remove(0, ml);
                        p += ml;
                        matched = true;
                        break;
                    }
                }
                if (matched)
                    break;
                Token token = r.parse(t, p);
                if (!checkTokenOrder(tokens, token.type(), err))
                    return bRet(ok, false, pos, p, fn, fileName, tokens);
                tokens << token;
                s.remove(0, ml);
                p += ml;
                matched = true;
                break;
            }
        }
        if (!matched)
            return bRet(ok, false, err, QString("Unknown token"), pos, p, fn, fileName, tokens);
    }
    tokens << Token(Token::EOF_Token, p);
    return bRet(ok, true, err, QString(), pos, -1, fn, fileName, tokens);
}

bool LexicalAnalyzer::removeNonterminals(QString &s, int &matchedLength, QString *err, int *pos)
{
    matchedLength = 0;
    QRegExp rx("\\s+");
    int ind = rx.indexIn(s);
    if (!ind)
    {
        matchedLength = rx.matchedLength();
        s.remove(0, matchedLength);
    }
    if (s.isEmpty() || s.at(0) != '%')
        return bRet(err, QString(), pos, -1, true);
    if (s.length() > 1 && s.at(1) == '%')
    {
        int i = 2;
        bool b = false;
        while (i < s.length() - 1)
        {
            if (s.at(i) == '%' && !isEscaped(s, i, '%') && s.at(i + 1) != '%')
            {
                b = true;
                break;
            }
            ++i;
        }
        if (b)
        {
            matchedLength += i + 1;
            s.remove(0, i + 1);
        }
        if (!b)
        {
            matchedLength = 0;
            return bRet(err, QString("Unterminated comment"), pos, i, false);
        }
    }
    else
    {
        int i = 1;
        while (i < s.length() && s.at(i) != '\n')
            ++i;
        matchedLength += i + 1;
        s.remove(0, i + 1);
    }
    int ml = 0;
    int p = -1;
    if (!removeNonterminals(s, ml, err, &p))
    {
        ml = matchedLength;
        matchedLength = 0;
        return bRet(pos, ml + p, false);
    }
    matchedLength += ml;
    return bRet(err, QString(), pos, -1, true);
}
