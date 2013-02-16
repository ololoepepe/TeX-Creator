#ifndef CACHE_H
#define CACHE_H

#include "sample.h"
#include "client.h"

class QDateTime;
class QStringList;

#include <QtGlobal>
#include <QSettings>
#include <QPointer>
#include <QString>
#include <QVariantMap>
#include <QList>

/*============================================================================
================================ Cache =======================================
============================================================================*/

class Cache
{
public:
    explicit Cache();
    ~Cache();
public:
    static void clearCache();
    static bool hasCache();
public:
    void setHost(const QString &host);
    void close();
    bool isValid() const;
    QDateTime samplesListUpdateDateTime() const;
    QList<Sample> samplesList() const;
    QDateTime sampleSourceUpdateDateTime(const quint64 id) const;
    QDateTime samplePreviewUpdateDateTime(const quint64 id) const;
    QVariantMap sampleSource(quint64 id) const;
    bool showSamplePreview(quint64 id) const;
    QDateTime userInfoUpdateDateTime(const QString &login) const;
    Client::UserInfo userInfo(const QString &login) const;
    bool setSamplesListUpdateDateTime(const QDateTime &dt);
    bool insertSamplesIntoList(const QList<Sample> &samples);
    bool removeSamplesFromList(const QList<quint64> &ids);
    bool setSampleSourceUpdateDateTime(quint64 id, const QDateTime &dt);
    bool setSamplePreviewUpdateDateTime(quint64 id, const QDateTime &dt);
    bool setSampleSource(quint64 id, const QVariantMap &sample);
    bool setSamplePreview(quint64 id, const QVariantMap &preview);
    bool setUserInfoUpdateDateTime(const QString &login, const QDateTime &dt);
    bool setUserInfo(const Client::UserInfo &info);
private:
    enum PathType
    {
        CachePath = 0,
        SamplesCachePath,
        UsersCachePath
    };
private:
    static inline QString idToString(quint64 id);
    static inline QString sampleKey( quint64 id, const QString &subkey = QString() );
    static inline QString userKey( const QString &login, const QString &subkey = QString() );
private:
    QString cachePath(PathType type = CachePath) const;
    QString sourceFileName(quint64 id) const;
    QString previewFileName(quint64 id) const;
    QStringList auxFileNames(quint64 id) const;
    void removeCache(quint64 id);
private:
    QPointer<QSettings> msettings;
    QString mhost;
private:
    Q_DISABLE_COPY(Cache)
};

#endif // CACHE_H
