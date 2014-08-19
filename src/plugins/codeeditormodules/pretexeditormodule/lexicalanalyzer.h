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

#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H

class QChar;
class QTextCodec;

#include "token.h"

#include <QChar>
#include <QCoreApplication>
#include <QList>
#include <QStack>
#include <QString>

/*============================================================================
================================ LexicalAnalyzer =============================
============================================================================*/

class LexicalAnalyzer
{
    Q_DECLARE_TR_FUNCTIONS(LexicalAnalyzer)
private:
    QTextCodec *mcodec;
    QString mfileName;
    QString msource;
public:
    explicit LexicalAnalyzer(const QString &source, const QString &fileName = QString(), QTextCodec *codec = 0);
public:
    static bool isEscaped(const QString &s, int pos, const QChar &symbol);
    static bool matchFuncName(const QString &s, int &matchedLength, bool *builtin = 0);
    static bool matchInteger(const QString &s, int &matchedLength);
    static bool matchReal(const QString &s, int &matchedLength);
    static bool matchSpecFuncName(const QString &s, int &matchedLength);
    static bool matchString(const QString &s, int &matchedLength);
public:
    QList<Token> analyze(bool *ok = 0, QString *err = 0, int *pos = 0, QString *fn = 0);
    QTextCodec *codec() const;
    QString codecName() const;
    QString fileName() const;
    void setCodec(QTextCodec *codec);
    void setCodecName(const QString &codecName);
    void setFileName(const QString &fileName);
    QString source() const;
    void setSource(const QString source);
private:
    static QList<Token> analyze(QString s, const QString &fileName, QTextCodec *codec, bool *ok = 0, QString *err = 0,
                                int *pos = 0, QString *fn = 0);
    static bool checkTokenOrder(const QList<Token> &tokens, Token::Type current, QString *err = 0);
    static bool matchBackslash(const QString &s, int &matchedLength);
    static bool matchFuncName2Args(const QString &s, int &matchedLength);
    static bool matchInput(const QString &s, QString &path, int &matchedLength);
    static bool matchLeftBrace(const QString &s, int &matchedLength);
    static bool matchLeftBracket(const QString &s, int &matchedLength);
    static bool matchRightBrace(const QString &s, int &matchedLength);
    static bool matchRightBracket(const QString &s, int &matchedLength);
    static bool matchSharp(const QString &s, int &matchedLength);
    static Token parceFuncName(const QString &s, int pos);
    static Token parceInteger(const QString &s, int pos);
    static Token parceReal(const QString &s, int pos);
    static Token parceSpecFuncName(const QString &s, int pos);
    static Token parceSpecialSymbol(const QString &s, int pos);
    static Token parceString(const QString &s, int pos);
    static bool removeNonterminals(QString &s, int &matchedLength, QString *err = 0, int *pos = 0);
    static QString replaceEscaped(QString s);
};

#endif // LEXICALANALYZER_H
