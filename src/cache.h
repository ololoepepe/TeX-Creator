#ifndef CACHE_H
#define CACHE_H

class QDateTime;
class QStringList;

#include <QtGlobal>
#include <QSettings>
#include <QPointer>
#include <QString>
#include <QVariantMap>

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
    QDateTime sampleListUpdateDateTime() const;
    QDateTime sampleSourceUpdateDateTime(const quint64 id) const;
    QDateTime samplePreviewUpdateDateTime(const quint64 id) const;
    QVariantMap sampleSource(quint64 id) const;
    bool setSampleListUpdateDateTime(const QDateTime &dt);
    bool setSampleSourceUpdateDateTime(quint64 id, const QDateTime &dt);
    bool setSamplePreviewUpdateDateTime(quint64 id, const QDateTime &dt);
    bool setSampleSource(quint64 id, const QVariantMap &sample);
private:
    static inline QString idToString(quint64 id);
    static inline QString sampleKey( quint64 id, const QString &subkey = QString() );
private:
    QString cachePath() const;
    QString sourceFileName(quint64 id) const;
    QString previewFileName(quint64 id) const;
    QStringList auxFileNames(quint64 id) const;
private:
    QPointer<QSettings> msettings;
    QString mhost;
private:
    Q_DISABLE_COPY(Cache)
};

#endif // CACHE_H