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

#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H

class QChar;
class QTextCodec;

#include "token.h"

#include <QString>
#include <QList>
#include <QStack>
#include <QChar>

/*============================================================================
================================ LexicalAnalyzer =============================
============================================================================*/

class LexicalAnalyzer
{
public:
    explicit LexicalAnalyzer(const QString &source, const QString &fileName = QString(), QTextCodec *codec = 0);
public:
    static bool isEscaped(const QString &s, int pos, const QChar &symbol);
    static bool matchString(const QString &s, int &matchedLength);
    static bool matchFuncName(const QString &s, int &matchedLength, bool *builtin = 0);
    static bool matchSpecFuncName(const QString &s, int &matchedLength);
    static bool matchReal(const QString &s, int &matchedLength);
    static bool matchInteger(const QString &s, int &matchedLength);
public:
    void setSource(const QString source);
    void setFileName(const QString &fileName);
    void setCodec(QTextCodec *codec);
    void setCodecName(const QString &codecName);
    QString source() const;
    QString fileName() const;
    QTextCodec *codec() const;
    QString codecName() const;
    QList<Token> analyze(bool *ok = 0, QString *err = 0, int *pos = 0, QString *fn = 0);
private:
    static QList<Token> analyze(QString s, const QString &fileName, QTextCodec *codec, bool *ok = 0, QString *err = 0,
                                int *pos = 0, QString *fn = 0);
    static bool removeNonterminals(QString &s, int &matchedLength, QString *err = 0, int *pos = 0);
private:
    QString msource;
    QString mfileName;
    QTextCodec *mcodec;
};

#endif // LEXICALANALYZER_H
