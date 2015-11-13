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

#include "pretexfiletype.h"

#include <BAbstractCodeEditorDocument>
#include <BAbstractFileType>
#include <BApplication>

#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QtAlgorithms>
#include <QTextCursor>

/*============================================================================
================================ PreTeXFileType ==============================
============================================================================*/

/*============================== Public constructors =======================*/

PreTeXFileType::PreTeXFileType()
{
    //
}

PreTeXFileType::~PreTeXFileType()
{
    //
}

/*============================== Public methods ============================*/

PreTeXFileType::BracketPairList PreTeXFileType::brackets() const
{
    BracketPairList list;
    list << createBracketPair("{", "}");
    list << createBracketPair("[", "]");
    list << createBracketPair("(", ")");
    return list;
}

QList<PreTeXFileType::AutocompletionItem> PreTeXFileType::createAutocompletionItemList(
        BAbstractCodeEditorDocument *doc, QTextCursor cursor)
{
    QList<AutocompletionItem> list;
    if (!doc || cursor.isNull())
        return list;
    cursor.select(QTextCursor::WordUnderCursor);
    if (!cursor.hasSelection())
        return list;
    QString word = cursor.selectedText();
    list << builtinFunctionAutocompletionItemListForWord(word);
    return list;
}

QString PreTeXFileType::description() const
{
    return tr("JavaScript files", "description");
}

QString PreTeXFileType::id() const
{
    return "JavaScript";
}

bool PreTeXFileType::matchesFileName(const QString &fileName) const
{
    return suffixes().contains(QFileInfo(fileName).suffix(), Qt::CaseInsensitive);
}

QString PreTeXFileType::name() const
{
    return tr("JavaScript", "name");
}

QStringList PreTeXFileType::suffixes() const
{
    return QStringList() << "js";
}

/*============================== Protected methods =========================*/

void PreTeXFileType::highlightBlock(const QString &text)
{
    /*int i = 0;
    int lastBSPos = -1;
    setCurrentBlockState(0);
    clearCurrentBlockSkipSegments();
    int lastState = previousBlockState();
    if (1 == lastState) {
        bool matched = false;
        while (i < text.length()) {
            if (text.at(i) == '%' && !LexicalAnalyzer::isEscaped(text, i, '%')
                    && text.length() > i + 1 && text.at(i + 1) == '%') {
                matched = true;
                break;
            }
            ++i;
        }
        if (matched) {
            i += 2;
            lastState = 0;
            setFormat(0, i, QColor(Qt::darkGray));
            addCurrentBlockSkipSegmentL(0, i);
        } else {
            setFormat(0, text.length(), QColor(Qt::darkGray));
            addCurrentBlockSkipSegment(0);
            lastState = 1;
        }
    }
    while (i < text.length()) {
        int ml = 0;
        bool builtin = false;
        bool matchedBS = false;
        QString s = text.mid(i);
        if (s.at(0) == '%' && !LexicalAnalyzer::isEscaped(text, i, '%')) {
            if (s.length() > 1 && s.at(1) == '%') {
                if (lastState == 1) {
                    setFormat(0, i + 1, QColor(Qt::darkGray));
                    addCurrentBlockSkipSegmentL(0, i + 1);
                    lastState = 0;
                } else {
                    int j = 2;
                    bool matched = false;
                    while (j < s.length()) {
                        if (s.at(j) == '%' && !LexicalAnalyzer::isEscaped(s, j, '%')
                                && s.length() > j + 1 && s.at(j + 1) == '%') {
                            matched = true;
                            break;
                        }
                        ++j;
                    }
                    if (matched) {
                        lastState = 0;
                        setFormat(i, j + 2, QColor(Qt::darkGray));
                        addCurrentBlockSkipSegmentL(i, j + 2);
                        i += j + 1;
                    } else {
                        setFormat(i, text.length() - i, QColor(Qt::darkGray));
                        addCurrentBlockSkipSegment(i);
                        lastState = 1;
                        break;
                    }
                }
            } else if (lastState != 1) {
                setFormat(i, text.length() - i, QColor(Qt::darkGray));
                addCurrentBlockSkipSegment(i);
                break;
            }
        } else if (LexicalAnalyzer::matchString(s, ml)) {
            setFormat(i, ml, QColor(51, 132, 43));
            addCurrentBlockSkipSegmentL(i, ml);
        } else if (LexicalAnalyzer::matchSpecFuncName(s, ml)) {
            if (lastBSPos >= 0)
                setFormat(lastBSPos, 1, QColor(180, 140, 30));
            setFormat(i, ml, QColor(180, 140, 30));
        } else if (LexicalAnalyzer::matchFuncName(s, ml, &builtin)) {
            if (builtin && lastBSPos >= 0)
                setFormat(lastBSPos, 1, QColor(180, 140, 30));
            setFormat(i, ml, builtin ? QColor(180, 140, 30) : QColor(Qt::red).lighter(70));
        } else if (LexicalAnalyzer::matchReal(s, ml) || LexicalAnalyzer::matchInteger(s, ml)) {
            setFormat(i, ml, QColor(0, 0, 136));
        } else if (s.at(0) == '#') {
            setFormat(i, 1, QColor(51, 132, 43));
        } else if (s.at(0) == '\\') {
            matchedBS = true;
            setFormat(i, 1, QColor(Qt::red).lighter(70));
        }
        lastBSPos = matchedBS ? i : -1;
        i += ml ? ml : 1;
    }
    setCurrentBlockState(lastState);*/
}

/*============================== Static private methods ====================*/

QList<BAbstractFileType::AutocompletionItem> PreTeXFileType::builtinFunctionAutocompletionItemListForWord(
        const QString &word)
{
    init_once(QStringList, builtinFunctions, QStringList()) {
        builtinFunctions.removeDuplicates();
        qSort(builtinFunctions.begin(), builtinFunctions.end(), &stringLessThan);
    }
    QList<AutocompletionItem> list;
    if (word.length() < 4 || !word.startsWith('\\'))
        return list;
    foreach (const QString &s, builtinFunctions) {
        if ((word.length() - 1) >= s.length() || !s.startsWith(word.mid(1), Qt::CaseInsensitive))
            continue;
        list << createAutocompletionItem("\\" + s + "{", s, "", BApplication::icon("function"));
    }
    return list;
}

bool PreTeXFileType::stringLessThan(const QString &s1, const QString &s2)
{
    return (s1.compare(s1, s2, Qt::CaseInsensitive) < 0);
}
