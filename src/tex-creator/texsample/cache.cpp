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

#include "cache.h"

#include <TAbstractCache>
#include <TBinaryFile>
#include <TBinaryFileList>

#include <BDirTools>

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
    menabled = true;
}

/*============================== Static public methods =====================*/

bool Cache::saveSamplePreview(const QString &path, const TBinaryFile &mainFile, const TBinaryFileList &extraFiles)
{
    if (path.isEmpty() || !mainFile.isValid())
        return false;
    if (!mainFile.save(path)) {
        BDirTools::rmdir(path);
        return false;
    }
    foreach (const TBinaryFile &file, extraFiles) {
        if (!file.save(path)) {
            BDirTools::rmdir(path);
            return false;
        }
    }
    return true;
}

/*============================== Public methods ============================*/

QVariant Cache::data(const QString &operation, const QVariant &id) const
{
    if (!menabled)
        return QVariant();
    return QVariant();
}

bool Cache::isEnabled() const
{
    return menabled;
}

QDateTime Cache::lastRequestDateTime(const QString &operation, const QVariant &id) const
{
    if (!menabled)
        return QDateTime();
    return QDateTime();
}

void Cache::removeData(const QString &operation, const QVariant &id)
{
    if (menabled)
        return;
    //
}

void Cache::setData(const QString &operation, const QDateTime &requestDateTime, const QVariant &data,
                    const QVariant &id)
{
    if (!menabled)
        return;
    //
}

void Cache::setEnabled(bool enabled)
{
    menabled = enabled;
}

void Cache::showSamplePreview(quint64 sampleId)
{
    //
}
