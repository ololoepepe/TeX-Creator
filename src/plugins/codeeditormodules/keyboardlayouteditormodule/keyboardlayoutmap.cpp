/****************************************************************************
**
** Copyright (C) 2014 TeXSample Team
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

#include "keyboardlayoutmap.h"

#include <BDirTools>

#include <QChar>
#include <QList>
#include <QMap>
#include <QString>

/*============================================================================
================================ KeyboardLayoutMap ===========================
============================================================================*/

/*============================== Public constructors =======================*/

KeyboardLayoutMap::KeyboardLayoutMap()
{
    //
}

KeyboardLayoutMap::KeyboardLayoutMap(const QString &fileName)
{
    load(fileName);
}

KeyboardLayoutMap::KeyboardLayoutMap(const KeyboardLayoutMap &other)
{
    *this = other;
}

/*============================== Public methods ============================*/

bool KeyboardLayoutMap::isValid() const
{
    return !mdirect.isEmpty() && !mreverse.isEmpty() && !mdirectUnique.isEmpty() && !mreverseUnique.isEmpty();
}

bool KeyboardLayoutMap::load(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;
    bool ok = false;
    QStringList sl = BDirTools::readTextFile(fileName, "UTF-8", &ok).split('\n', QString::SkipEmptyParts);
    if (!ok)
        return false;
    mdirect.clear();
    mreverse.clear();
    mdirectUnique.clear();
    mreverseUnique.clear();
    foreach (int i, bRangeR(sl.size() - 1, 0)) {
        if (sl.at(i).at(0) == '#')
            sl.removeAt(i);
    }
    if (sl.isEmpty())
        return true;
    foreach (const QString &s, sl) {
        QStringList sl = s.split(' ');
        if (sl.size() != 2 || sl.first().length() != 1 || sl.last().length() != 1)
            continue;
        const QChar &fc = sl.first().at(0);
        const QChar &sc = sl.last().at(0);
        if (mdirect.contains(fc) || mreverse.contains(sc))
            continue;
        mdirect.insert(fc, sc);
        mdirectUnique << fc;
        mreverse.insert(sc, fc);
        mreverseUnique << sc;
    }
    foreach (int i, bRangeR(mdirectUnique.size() - 1, 0)) {
        const QChar &c = mdirectUnique.at(i);
        if (mreverseUnique.contains(c)) {
            mdirectUnique.removeAll(c);
            mreverseUnique.removeAll(c);
        }
    }
    return true;
}

bool KeyboardLayoutMap::switchLayout(QString &text) const
{
    if (text.isEmpty() || !isValid())
        return false;
    int direct = 0;
    int reverse = 0;
    foreach (const QChar &c, mdirectUnique)
        direct += text.count(c);
    foreach (const QChar &c, mreverseUnique)
        reverse += text.count(c);
    if (direct == reverse)
        return false;
    const QMap<QChar, QChar> &m = (direct > reverse) ? mdirect : mreverse;
    QList<QChar> keys = m.keys();
    foreach (int i, bRangeD(0, text.length() - 1)) {
        if (keys.contains(text.at(i)))
            text[i] = m.value(text.at(i));
    }
    return true;
}

/*============================== Public operators ==========================*/

KeyboardLayoutMap &KeyboardLayoutMap::operator=(const KeyboardLayoutMap &other)
{
    mdirect = other.mdirect;
    mreverse = other.mreverse;
    mdirectUnique = other.mdirectUnique;
    mreverseUnique = other.mreverseUnique;
    return *this;
}
