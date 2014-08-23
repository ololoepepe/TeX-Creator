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

#ifndef PRETEXFILETYPE_H
#define PRETEXFILETYPE_H

class BAbstractCodeEditorDocument;

class QString;
class QStringList;
class QTextCursor;

#include <BAbstractFileType>

#include <QCoreApplication>
#include <QList>

/*============================================================================
================================ PreTeXFileType ==============================
============================================================================*/

class PreTeXFileType : public BAbstractFileType
{
    Q_DECLARE_TR_FUNCTIONS(PreTeXFileType)
public:
    explicit PreTeXFileType();
    ~PreTeXFileType();
public:
    BracketPairList brackets() const;
    QList<AutocompletionItem> createAutocompletionItemList(BAbstractCodeEditorDocument *doc, QTextCursor cursor);
    QString description() const;
    QString id() const;
    bool matchesFileName(const QString &fileName) const;
    QString name() const;
    QStringList suffixes() const;
protected:
    void highlightBlock(const QString &text);
private:
    static QList<AutocompletionItem> builtinFunctionAutocompletionItemListForWord(const QString &word);
    static bool stringLessThan(const QString &s1, const QString &s2);
private:
    Q_DISABLE_COPY(PreTeXFileType)
};

#endif // PRETEXFILETYPE_H
