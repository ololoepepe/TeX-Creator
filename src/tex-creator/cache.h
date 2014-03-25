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