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

#ifndef LATEXFILETYPE_H
#define LATEXFILETYPE_H

class QString;
class QStringList;

#include "application.h"

#include <BAbstractFileType>

/*============================================================================
================================ LatexFileType ===============================
============================================================================*/

class LatexFileType : public BAbstractFileType
{
    Q_DECLARE_TR_FUNCTIONS(LatexFileType)
public:
    LatexFileType();
    ~LatexFileType();
public:
    BracketPairList brackets() const;
    QString description() const;
    QString id() const;
    bool matchesFileName(const QString &fileName) const;
    QString name() const;
    QStringList suffixes() const;
protected:
    void highlightBlock(const QString &text);
private:
    Q_DISABLE_COPY(LatexFileType)
};

#endif // LATEXFILETYPE_H
