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

#ifndef CACHE_H
#define CACHE_H

class BCodeEdit;

class QStringList;
class QVariant;
class QByteArray;

#include "client.h"

#include <TSampleInfo>
#include <TProjectFile>
#include <TTexProject>
#include <TeXSample>
#include <TSampleInfoList>
#include <TIdList>

#include <QtGlobal>
#include <QSettings>
#include <QPointer>
#include <QString>
#include <QVariantMap>
#include <QList>
#include <QDateTime>
#include <QMap>

#define sCache Cache::instance()

/*============================================================================
================================ Cache =======================================
============================================================================*/

class Cache
{
public:
    explicit Cache();
    ~Cache();
public:
    static Cache *instance();
    static bool cacheExists();
public:
    void open();
    void close();
    void clear();
    void cacheSampleInfos(const TSampleInfoList &samples, const QDateTime &updateDT);
    void cacheSampleSource(quint64 id, const QDateTime &updateDT, const TTexProject &source = TTexProject());
    void cacheSamplePreview(quint64 id, const QDateTime &updateDT, const TProjectFile &preview = TProjectFile());
    void cacheUserInfo(const TUserInfo &info, const QDateTime &updateDT);
    void cacheUserInfo(quint64 id, const QDateTime &updateDT);
    void removeSample(quint64 id);
    void removeSamples(const TIdList &ids);
    void removeUserInfo(quint64 id);
    TSampleInfoList sampleInfos() const;
    TSampleInfo sampleInfo(quint64 id) const;
    TUserInfo userInfo(quint64 id) const;
    QDateTime sampleInfosUpdateDateTime(Qt::TimeSpec spec = Qt::UTC) const;
    QDateTime sampleSourceUpdateDateTime(quint64 id, Qt::TimeSpec spec = Qt::UTC) const;
    QDateTime samplePreviewUpdateDateTime(quint64 id, Qt::TimeSpec spec = Qt::UTC) const;
    QDateTime userInfoUpdateDateTime(quint64 id, Qt::TimeSpec spec = Qt::UTC) const;
    TTexProject sampleSource(quint64 id) const;
    QString samplePreviewFileName(quint64 id) const;
    bool isValid() const;
private:
    enum PathType
    {
        CachePath = 0,
        SamplesCachePath,
        UsersCachePath
    };
private:
    static inline QString sampleKey(const QString &subkey);
    static inline QString sampleKey(quint64 id, const QString &subkey = QString());
    static inline QString userKey(const QString &subkey);
    static inline QString userKey(quint64 id, const QString &subkey = QString());
    static QString cachePath(PathType type = CachePath, const QString &subpath = QString());
private:
    TIdList sampleInfosIds() const;
    void setValue(const QString &key, const QVariant &v);
    void remove(const QString &key);
    QVariant value(const QString &key) const;
    bool saveUserAvatar(quint64 id, const QByteArray &data) const;
    QByteArray loadUserAvatar(quint64 id, bool *ok = 0) const;
private:
    static Cache *minstance;
private:
    mutable QPointer<QSettings> msettings;
private:
    Q_DISABLE_COPY(Cache)
};

#endif // CACHE_H
