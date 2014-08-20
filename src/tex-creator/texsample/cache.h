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

#ifndef CACHE_H
#define CACHE_H

class QDateTime;
class QString;

#include <TAbstractCache>

#include <QVariant>

/*============================================================================
================================ Cache =======================================
============================================================================*/

class Cache : public TAbstractCache
{
public:
    explicit Cache(const QString &location);
public:
    QVariant data(const QString &operation, const QVariant &id = QVariant()) const;
    QDateTime lastRequestDateTime(const QString &operation, const QVariant &id = QVariant()) const;
    void removeData(const QString &operation, const QVariant &id = QVariant());
    void setData(const QString &operation, const QDateTime &requestDateTime, const QVariant &data = QVariant(),
                 const QVariant &id = QVariant());
};

#endif // CACHE_H
