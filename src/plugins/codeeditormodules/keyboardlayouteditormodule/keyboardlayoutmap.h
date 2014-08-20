/****************************************************************************
**
** Copyright (C) 2014 Andrey Bogdanov
**
** This file is part of the Kayboard Layout Editor Module plugin
** of TeX Creator.
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

#ifndef KEYBOARDLAYOUTMAP_H
#define KEYBOARDLAYOUTMAP_H

class QString;

#include <QChar>
#include <QList>
#include <QMap>

/*============================================================================
================================ KeyboardLayoutMap ===========================
============================================================================*/

class KeyboardLayoutMap
{
public:
    explicit KeyboardLayoutMap();
    explicit KeyboardLayoutMap(const QString &fileName);
    KeyboardLayoutMap(const KeyboardLayoutMap &other);
public:
    bool isValid() const;
    bool load(const QString &fileName);
    bool switchLayout(QString &text) const;
public:
    KeyboardLayoutMap &operator=(const KeyboardLayoutMap &other);
private:
    QMap<QChar, QChar> mdirect;
    QList<QChar> mdirectUnique;
    QMap<QChar, QChar> mreverse;
    QList<QChar> mreverseUnique;
};

#endif // KEYBOARDLAYOUTMAP_H
