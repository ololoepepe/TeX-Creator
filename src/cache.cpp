#include "cache.h"
#include "application.h"
#include "client.h"
#include "texsamplesettingstab.h"

#include <TUserInfo>
#include <TTexProject>
#include <TProjectFile>
#include <TeXSample>

#include <BDirTools>
#include <BCodeEdit>
#include <TServiceList>

#include <QString>
#include <QtGlobal>
#include <QSettings>
#include <QPointer>
#include <QDir>
#include <QDateTime>
#include <QVariant>
#include <QVariantMap>
#include <QStringList>
#include <QVariantList>
#include <QFileInfo>
#include <QList>
#include <QDebug>
#include <QImage>
#include <QByteArray>
#include <QBuffer>

/*============================================================================
================================ Cache =======================================
============================================================================*/

/*============================== Public constructors =======================*/

Cache::Cache()
{
    //
}

Cache::~Cache()
{
    close();
}

/*============================== Static public methods =====================*/

Cache *Cache::instance()
{
    if (!minstance)
    {
        minstance = new Cache;
        minstance->open();
    }
    return minstance;
}

bool Cache::cacheExists()
{
    QString path = cachePath();
    return !path.isEmpty() && !QDir(path).entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot).isEmpty();
}

/*============================== Public methods ============================*/

void Cache::open()
{
    if (!msettings.isNull())
        return;
    QString path = cachePath();
    if (!BDirTools::mkpath(path))
        return;
    msettings = new QSettings(path + "/cache.conf", QSettings::IniFormat);
    msettings->setIniCodec("UTF-8");
    if (!msettings->isWritable())
        return close();
}

void Cache::close()
{
    if (msettings.isNull())
        return;
    msettings->sync();
    delete msettings;
    msettings = 0;
}

void Cache::clear()
{
    QString path = cachePath();
    if (path.isEmpty())
        return;
    bool b = isValid();
    close();
    BDirTools::removeFilesInDir(path);
    foreach (const QString &p, QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        BDirTools::rmdir(path + "/" + p);
    if (b)
        open();
}

void Cache::cacheSampleInfos(const TSampleInfoList &samples, const QDateTime &updateDT)
{
    if (!isValid())
        return;
    foreach (const TSampleInfo &s, samples)
    {
        if (!s.isValid(TSampleInfo::GeneralContext))
            continue;
        removeSample(s.id());
        setValue(sampleKey(s.id(), "info"), s);
    }
    setValue(sampleKey("update_dt"), updateDT);
}

void Cache::cacheSampleSource(quint64 id, const QDateTime &updateDT, const TTexProject &source)
{
    if (!id || !isValid())
        return;
    if (source.isValid())
    {
        BDirTools::rmdir(cachePath(SamplesCachePath, QString::number(id)));
        source.save(cachePath(SamplesCachePath, QString::number(id)), "UTF-8");
    }
    setValue(sampleKey(id, "source_update_dt"), updateDT);
}

void Cache::cacheSamplePreview(quint64 id, const QDateTime &updateDT, const TProjectFile &preview)
{
    if (!id || !isValid())
        return;
    if (preview.isValid())
    {
        //BDirTools::rmdir(cachePath(SamplesCachePath, QString::number(id)));
        preview.save(cachePath(SamplesCachePath, QString::number(id)), "UTF-8");
    }
    setValue(sampleKey(id, "preview_update_dt"), updateDT);
}

void Cache::cacheUserInfo(const TUserInfo &info, const QDateTime &updateDT)
{
    if (!info.id() || !isValid())
        return;
    if (info.isValid())
    {
        removeUserInfo(info.id());
        setValue(userKey(info.id(), "login"), info.login());
        setValue(userKey(info.id(), "access_level"), info.accessLevel());
        setValue(userKey(info.id(), "services"), info.services());
        setValue(userKey(info.id(), "real_name"), info.realName());
        setValue(userKey(info.id(), "creation_dt"), info.creationDateTime());
        setValue(userKey(info.id(), "update_dt"), info.updateDateTime());
        saveUserAvatar(info.id(), info.avatar());
    }
    setValue(userKey(info.id(), "update_dt"), updateDT);
}

void Cache::cacheUserInfo(quint64 id, const QDateTime &updateDT)
{
    cacheUserInfo(TUserInfo(id), updateDT);
}

void Cache::removeSample(quint64 id)
{
    if (!id || !isValid())
        return;
    BDirTools::rmdir(cachePath(SamplesCachePath, QString::number(id)));
    remove(sampleKey(id));
}

void Cache::removeSamples(const TIdList &ids)
{
    if (!isValid())
        return;
    foreach (quint64 id, ids)
        removeSample(id);
}

void Cache::removeUserInfo(quint64 id)
{
    if (!id || !isValid())
        return;
    remove(userKey(id));
    BDirTools::rmdir(cachePath(UsersCachePath, QString::number(id)));
}

TSampleInfoList Cache::sampleInfos() const
{
    TSampleInfoList list;
    if (!isValid())
        return list;
    foreach (quint64 id, sampleInfosIds())
    {
        TSampleInfo info = sampleInfo(id);
        if (info.isValid())
            list << info;
    }
    return list;
}

TSampleInfo Cache::sampleInfo(quint64 id) const
{
    return (id && isValid()) ? value(sampleKey(id, "info")).value<TSampleInfo>() : TSampleInfo();
}

TUserInfo Cache::userInfo(quint64 id) const
{
    if (!id || !isValid())
        return TUserInfo();
    TUserInfo info;
    info.setId(id);
    info.setLogin(value(userKey(info.id(), "login")).toString());
    info.setAccessLevel(value(userKey(info.id(), "access_level")).value<TAccessLevel>());
    info.setServices(value(userKey(info.id(), "services")).value<TServiceList>());
    info.setRealName(value(userKey(info.id(), "real_name")).toString());
    info.setCreationDateTime(value(userKey(info.id(), "creation_dt")).toDateTime());
    info.setUpdateDateTime(value(userKey(info.id(), "update_dt")).toDateTime());
    info.setAvatar(loadUserAvatar(id));
    return info;
}

QDateTime Cache::sampleInfosUpdateDateTime(Qt::TimeSpec spec) const
{
    return value(sampleKey("update_dt")).toDateTime().toTimeSpec(spec);
}

QDateTime Cache::sampleSourceUpdateDateTime(quint64 id, Qt::TimeSpec spec) const
{
    return ((id && isValid()) ? value(sampleKey(id, "source_update_dt")).toDateTime() : QDateTime()).toTimeSpec(spec);
}

QDateTime Cache::samplePreviewUpdateDateTime(quint64 id, Qt::TimeSpec spec) const
{
    return ((id && isValid()) ? value(sampleKey(id, "preview_update_dt")).toDateTime() : QDateTime()).toTimeSpec(spec);
}

QDateTime Cache::userInfoUpdateDateTime(quint64 id, Qt::TimeSpec spec) const
{
    return value(userKey(id, "update_dt")).toDateTime().toTimeSpec(spec);
}

TTexProject Cache::sampleSource(quint64 id) const
{
    return TTexProject(cachePath(SamplesCachePath, QString::number(id) + "/" + sampleInfo(id).fileName()), "UTF-8");
}

QString Cache::samplePreviewFileName(quint64 id) const
{
    QString fn = QFileInfo(sampleInfo(id).fileName()).baseName() + ".pdf";
    return cachePath(SamplesCachePath, QString::number(id) + "/" + fn);
}

bool Cache::isValid() const
{
    return !msettings.isNull() && msettings->isWritable();
}

/*============================== Static private methods ====================*/

QString Cache::sampleKey(const QString &subkey)
{
    return sampleKey(0, subkey);
}

QString Cache::sampleKey(quint64 id, const QString &subkey)
{
    QString s = "Samples";
    if (id)
        s += "/id_" + QString::number(id);
    if (!subkey.isEmpty())
        s += "/" + subkey;
    return s;
}

QString Cache::userKey(const QString &subkey)
{
    return userKey(0, subkey);
}

QString Cache::userKey(quint64 id, const QString &subkey)
{
    QString s = "Users";
    if (id)
        s += "/id_" + QString::number(id);
    if (!subkey.isEmpty())
        s += "/" + subkey;
    return s;
}

QString Cache::cachePath(PathType type, const QString &subpath)
{
    QString path = BDirTools::findResource("texsample", BDirTools::UserOnly);
    if (path.isEmpty())
        return "";
    switch (type)
    {
    case SamplesCachePath:
        path += "/samples";
        break;
    case UsersCachePath:
        path += "/users";
        break;
    case CachePath:
    default:
        break;
    }
    if (path.isEmpty())
        return "";
    if (!subpath.isEmpty())
        path += "/" + subpath;
    return path;
}

/*============================== Private methods ===========================*/

TIdList Cache::sampleInfosIds() const
{
    TIdList list;
    if (!isValid())
        return list;
    msettings->beginGroup("Samples");
    QStringList keys = msettings->childGroups();
    msettings->endGroup();
    foreach (const QString &key, keys)
    {
        bool ok = false;
        quint64 id = key.mid(3).toULongLong(&ok);
        if (ok && id)
            list << id;
    }
    return list;
}

void Cache::setValue(const QString &key, const QVariant &v)
{
    if (key.isEmpty() || msettings.isNull())
        return;
    msettings->setValue(key, v);
}

void Cache::remove(const QString &key)
{
    if (key.isEmpty() || msettings.isNull())
        return;
    msettings->remove(key);
}

QVariant Cache::value(const QString &key) const
{
    return (!key.isEmpty() && !msettings.isNull()) ? msettings->value(key) : QVariant();
}

bool Cache::saveUserAvatar(quint64 id, const QByteArray &data) const
{
    if (!id)
        return false;
    return BDirTools::writeFile(cachePath(UsersCachePath, QString::number(id) + "/avatar.dat"), data);
}

QByteArray Cache::loadUserAvatar(quint64 id, bool *ok) const
{
    if (!id)
        bRet(ok, false, QByteArray());
    return BDirTools::readFile(cachePath(UsersCachePath, QString::number(id) + "/avatar.dat"), -1, ok);
}

/*============================== Static private members ====================*/

Cache *Cache::minstance = 0;
