/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
**
** This file is part of TeX Creator.
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

#include "latexfiletype.h"

#include <BAbstractFileType>

#include <QColor>
#include <QFileInfo>
#include <QRegExp>
#include <QString>
#include <QStringList>

static int indexOf(const QString &text, const QString &what, int from = 0)
{
    if (text.isEmpty() || what.isEmpty())
        return -1;
    int ind = text.indexOf(what, from);
    while (ind >= 0) {
        if (!ind || text.at(ind - 1) != '\\')
            return ind;
        ind = text.indexOf(what, ++from);
    }
    return -1;
}

/*============================================================================
================================ LatexFileType ===============================
============================================================================*/

/*============================== Public constructors =======================*/

LatexFileType::LatexFileType()
{
    //
}

LatexFileType::~LatexFileType()
{
    //
}

/*============================== Public methods ============================*/

BAbstractFileType::BracketPairList LatexFileType::brackets() const
{
    BracketPairList list;
    list << createBracketPair("{", "}", "\\");
    return list;
}

QString LatexFileType::description() const
{
    return tr("LaTeX files", "description");
}

QString LatexFileType::id() const
{
    return "LaTeX";
}

bool LatexFileType::matchesFileName(const QString &fileName) const
{
    return suffixes().contains(QFileInfo(fileName).suffix(), Qt::CaseInsensitive);
}

QString LatexFileType::name() const
{
    return "LaTeX"; //No need to translate
}

QStringList LatexFileType::suffixes() const
{
    return QStringList() << "tex" << "inp" << "pic" << "sty";
}

/*============================== Protected methods =========================*/

void LatexFileType::highlightBlock(const QString &text)
{
    //comments
    int comInd = text.indexOf('%');
    while (comInd > 0 && text.at(comInd - 1) == '\\')
        comInd = text.indexOf('%', comInd + 1);
    clearCurrentBlockSkipSegments();
    addCurrentBlockSkipSegment(comInd);
    if (comInd >= 0)
        setFormat(comInd, text.length() - comInd, QColor(Qt::darkGray));
    QString ntext = text.left(comInd);
    //commands
    QRegExp rx("(\\\\[a-zA-Z]*|\\\\#|\\\\\\$|\\\\%|\\\\&|\\\\_|\\\\\\{|\\\\\\})+");
    int pos = rx.indexIn(ntext);
    while (pos >= 0) {
        int len = rx.matchedLength();
        setFormat(pos, len, QColor(Qt::red).lighter(70));
        pos = rx.indexIn(ntext, pos + len);
    }
    //multiline (math mode)
    setCurrentBlockState(!ntext.isEmpty() ? 0 : previousBlockState());
    int startIndex = 0;
    bool firstIsStart = false;
    if (previousBlockState() != 1) {
        startIndex = indexOf(ntext, "$");
        firstIsStart = true;
    }
    while (startIndex >= 0)
    {
        int endIndex = indexOf(ntext, "$", startIndex + (firstIsStart ? 1 : 0));
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = ntext.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + 1;
        }
        setFormat(startIndex, commentLength, QColor(Qt::darkGreen));
        startIndex = indexOf(ntext, "$", startIndex + commentLength);
    }
}
