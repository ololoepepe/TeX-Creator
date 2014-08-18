/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
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

#include "cache.h"

#include <TAbstractCache>

#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QVariant>

/*============================================================================
================================ Cache =======================================
============================================================================*/

/*============================== Public constructors =======================*/

Cache::Cache(const QString &location)
{
    //
}

/*============================== Public methods ============================*/

QVariant Cache::data(const QString &operation, const QVariant &id) const
{
    return QVariant();
}

QDateTime Cache::lastRequestDateTime(const QString &operation, const QVariant &id) const
{
    return QDateTime();
}

void Cache::removeData(const QString &operation, const QVariant &id)
{
    //
}

void Cache::setData(const QString &operation, const QDateTime &requestDateTime, const QVariant &data)
{
    //
}
