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

#include "lexicalanalyzer.h"

#include <BAbstractCodeEditorDocument>
#include <BAbstractFileType>
#include <BApplication>

#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
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
    list << createBracketPair("{", "}", "\\");
    list << createBracketPair("[", "]", "\\");
    return list;
}

QList<PreTeXFileType::AutocompletionItem> PreTeXFileType::createAutocompletionItemList(
        BAbstractCodeEditorDocument *doc, QTextCursor cursor)
{
    init_once(QStringList, functionNewList, QStringList()) {
        functionNewList << "\\newVar";
        functionNewList << "\\newLocalVar";
        functionNewList << "\\newGlobalVar";
        functionNewList << "\\newArray";
        functionNewList << "\\newLocalArray";
        functionNewList << "\\newGlobalArray";
        functionNewList << "\\newFunc";
        functionNewList << "\\newLocalFunc";
        functionNewList << "\\newGlobalFunc";
    }
    typedef QMap<QString, const QStringList *> StringListMap;
    init_once(StringListMap, listMap, StringListMap()) {
        listMap.insert("\\n", &functionNewList);
        listMap.insert("\\ne", &functionNewList);
        listMap.insert("\\new", &functionNewList);
    }
    typedef QMap<QString, QString> StringMap;
    init_once(StringMap, iconNameMap, StringMap()) {
        iconNameMap.insert("\\n", "function");
        iconNameMap.insert("\\ne", "function");
        iconNameMap.insert("\\new", "function");
    }
    QList<AutocompletionItem> list;
    if (!doc || cursor.isNull())
        return list;
    cursor.select(QTextCursor::WordUnderCursor);
    if (!cursor.hasSelection())
        return list;
    QString text = cursor.selectedText();
    const QStringList *sl = listMap.value(text);
    if (!sl)
        return list;
    QString iconName = iconNameMap.value(text);
    foreach (const QString &s, *sl)
        list << createAutocompletionItem(s + "{", s, "", BApplication::icon(iconName));
    return list;
}

QString PreTeXFileType::description() const
{
    return tr("PreTeX files", "description");
}

QString PreTeXFileType::id() const
{
    return "PreTeX";
}

bool PreTeXFileType::matchesFileName(const QString &fileName) const
{
    return suffixes().contains(QFileInfo(fileName).suffix(), Qt::CaseInsensitive);
}

QString PreTeXFileType::name() const
{
    return tr("PreTeX", "name");
}

QStringList PreTeXFileType::suffixes() const
{
    return QStringList() << "pretex";
}

/*============================== Protected methods =========================*/

void PreTeXFileType::highlightBlock(const QString &text)
{
    int i = 0;
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
    setCurrentBlockState(lastState);
}
