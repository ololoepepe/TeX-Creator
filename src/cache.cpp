#include "cache.h"
#include "sample.h"
#include "application.h"
#include "client.h"

#include <BDirTools>

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

void Cache::clearCache()
{
    QString path = BDirTools::findResource("texsample", BDirTools::UserOnly);
    if ( path.isEmpty() )
        return;
    foreach ( const QString &p, QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot) )
        BDirTools::rmdir(path + "/" + p);
}

bool Cache::hasCache()
{
    QString path = BDirTools::findResource("texsample", BDirTools::UserOnly);
    return !path.isEmpty() && !QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot).isEmpty();
}

/*============================== Public methods ============================*/

void Cache::setHost(const QString &host)
{
    if (host.isEmpty() || host == mhost)
        return;
    close();
    mhost = host;
    QString path = cachePath();
    if ( !BDirTools::mkpath(path) )
        return;
    msettings = new QSettings(path + "/cache.conf", QSettings::IniFormat);
    if ( !msettings->isWritable() )
        return close();
    msettings->setIniCodec("UTF-8");
}

void Cache::close()
{
    if ( msettings.isNull() )
        return;
    msettings->sync();
    delete msettings;
}

bool Cache::isValid() const
{
    return !msettings.isNull() && msettings->isWritable();
}

QDateTime Cache::samplesListUpdateDateTime() const
{
    if ( !isValid() )
        return QDateTime();
    return msettings->value("Samples/list_update_dt").toDateTime();
}

QList<Sample> Cache::samplesList() const
{
    QList<Sample> list;
    if ( !isValid() )
        return list;
    msettings->beginGroup("Samples");
    QStringList keys = msettings->childGroups();
    foreach (const QString &key, keys)
        list << Sample::fromVariantMap( msettings->value(key + "/info").toMap() );
    msettings->endGroup();
    return list;
}

QDateTime Cache::sampleSourceUpdateDateTime(const quint64 id) const
{
    if ( !id || !isValid() )
        return QDateTime();
    return msettings->value( sampleKey(id, "source_update_dt") ).toDateTime();
}

QDateTime Cache::samplePreviewUpdateDateTime(const quint64 id) const
{
    if ( !id || !isValid() )
        return QDateTime();
    return msettings->value( sampleKey(id, "preview_update_dt") ).toDateTime();
}

QVariantMap Cache::sampleSource(quint64 id) const
{
    QVariantMap m;
    if ( !id || !isValid() )
        return m;
    QString sfn = sourceFileName(id);
    if ( sfn.isEmpty() )
        return m;
    bool ok = false;
    QString txt = BDirTools::readTextFile(sfn, "UTF-8", &ok);
    if (!ok)
        return m;
    QVariantList aux;
    foreach ( const QString &fn, auxFileNames(id) )
    {
        bool ok = false;
        QByteArray ba = BDirTools::readFile(fn, -1, &ok);
        if (!ok)
            return m;
        QVariantMap a;
        a.insert( "file_name", QFileInfo(fn).fileName() );
        a.insert("data", ba);
        aux << a;
    }
    m.insert( "file_name", QFileInfo(sfn).fileName() );
    m.insert("text", txt);
    m.insert("aux_files", aux);
    return m;
}

bool Cache::showSamplePreview(quint64 id) const
{
    return id && isValid() && bApp->openLocalFile( previewFileName(id) );
}

QDateTime Cache::userInfoUpdateDateTime(const QString &login) const
{
    if ( login.isEmpty() || !isValid() )
        return QDateTime();
    return msettings->value( userKey(login, "update_dt") ).toDateTime();
}

Client::UserInfo Cache::userInfo(const QString &login) const
{
    Client::UserInfo info;
    if ( login.isEmpty() || !isValid() )
        return info;
    info.login = login;
    info.accessLevel = static_cast<Client::AccessLevel>( msettings->value( userKey(login, "access_level") ).toInt() );
    info.realName = msettings->value( userKey(login, "real_name") ).toString();
    QString path = cachePath(UsersCachePath);
    if ( path.isEmpty() || !QDir(path).exists() )
        return info;
    info.avatar = BDirTools::readFile(path + "/" + login);
    return info;
}

bool Cache::setSamplesListUpdateDateTime(const QDateTime &dt)
{
    if ( !isValid() )
        return false;
    msettings->setValue("Samples/list_update_dt", dt);
    return true;
}

bool Cache::insertSamplesIntoList(const QList<Sample> &samples)
{
    if ( samples.isEmpty() )
        return true;
    if ( !isValid() )
        return false;
    foreach (const Sample &s, samples)
    {
        if ( !s.isValid() )
            continue;
        msettings->setValue( sampleKey(s.id(), "info"), s.toVariantMap() );
        removeCache( s.id() );
    }
    return true;
}

bool Cache::removeSamplesFromList(const QList<quint64> &ids)
{
    if ( ids.isEmpty() )
        return true;
    if ( !isValid() )
        return false;
    foreach (quint64 id, ids)
    {
        if (!id)
            continue;
        msettings->remove( sampleKey(id) );
        removeCache(id);
    }
    return true;
}

bool Cache::setSampleSourceUpdateDateTime(quint64 id, const QDateTime &dt)
{
    if ( !id || !isValid() )
        return false;
    msettings->setValue(sampleKey(id, "source_update_dt"), dt);
    return true;
}

bool Cache::setSamplePreviewUpdateDateTime(quint64 id, const QDateTime &dt)
{
    if ( !id || !isValid() )
        return false;
    msettings->setValue(sampleKey(id, "preview_update_dt"), dt);
    return true;
}

bool Cache::setSampleSource(quint64 id, const QVariantMap &sample)
{
    if ( !id || sample.isEmpty() || !isValid() )
        return false;
    QString fn = sample.value("file_name").toString();
    QString text = sample.value("text").toString();
    if ( fn.isEmpty() || text.isEmpty() )
        return false;
    QString path = cachePath(SamplesCachePath);
    if ( path.isEmpty() )
        return false;
    path += ( "/" + idToString(id) );
    QString pfn = previewFileName(id); //Preview file name
    QStringList excl = QStringList() << QFileInfo(pfn).fileName();
    if ( !BDirTools::mkpath(path) || !BDirTools::removeFilesInDir( path, QStringList(), excl) )
        return false;
    if ( !BDirTools::writeTextFile(path + "/" + QFileInfo(fn).fileName(), text, "UTF-8") )
        return false;
    foreach ( const QVariant &v, sample.value("aux_files").toList() )
    {
        QVariantMap m = v.toMap();
        if ( !BDirTools::writeFile( path + "/" + m.value("file_name").toString(), m.value("data").toByteArray() ) )
            return false;
    }
    if ( QFileInfo(pfn).baseName() != QFileInfo(fn).baseName() )
        QFile::rename(pfn, path + "/" + QFileInfo(fn).baseName() + ".pdf");
    return true;
}

bool Cache::setSamplePreview(quint64 id, const QVariantMap &preview)
{
    if ( !id || preview.isEmpty() || !isValid() )
        return false;
    QString fn = preview.value("file_name").toString();
    QByteArray ba = preview.value("data").toByteArray();
    if ( fn.isEmpty() || ba.isEmpty() )
        return false;
    QString path = cachePath(SamplesCachePath);
    if ( path.isEmpty() )
        return false;
    path += ( "/" + idToString(id) );
    if ( !BDirTools::mkpath(path) )
        return false;
    return BDirTools::writeFile(path + "/" + QFileInfo(fn).fileName(), ba);
}

bool Cache::setUserInfoUpdateDateTime(const QString &login, const QDateTime &dt)
{
    if ( login.isEmpty() || !isValid() )
        return false;
    msettings->setValue(userKey(login, "update_dt"), dt);
    return true;
}

bool Cache::setUserInfo(const Client::UserInfo &info)
{
    if ( info.login.isEmpty() || !isValid() )
        return false;
    msettings->setValue(userKey(info.login, "access_level"), info.accessLevel);
    msettings->setValue(userKey(info.login, "real_name"), info.realName);
    if ( info.avatar.isNull() )
        return true;
    QString path = cachePath(UsersCachePath);
    return BDirTools::mkpath(path) && BDirTools::writeFile(path + "/" + info.login, info.avatar);
}

/*============================== Static private methods ====================*/

QString Cache::idToString(quint64 id)
{
    return QString::number(id);
}

QString Cache::sampleKey(quint64 id, const QString &subkey)
{
    return "Samples/id_" + idToString(id) + ( !subkey.isEmpty() ? ("/" + subkey) : QString() );
}

QString Cache::userKey(const QString &login, const QString &subkey)
{
    return "Users/" + login + ( !subkey.isEmpty() ? ("/" + subkey) : QString() );
}

/*============================== Private methods ===========================*/

QString Cache::cachePath(PathType type) const
{
    if ( mhost.isEmpty() )
        return "";
    QString path = BDirTools::findResource("texsample", BDirTools::UserOnly);
    if ( path.isEmpty() )
        return "";
    path += "/" + mhost;
    switch (type)
    {
    case SamplesCachePath:
        return path + "/samples";
    case UsersCachePath:
        return path + "/users";
    case CachePath:
    default:
        return path;
    }
}

QString Cache::sourceFileName(quint64 id) const
{
    if (!id)
        return "";
    QString path = cachePath(SamplesCachePath);
    if ( path.isEmpty() )
        return "";
    QDir d( path + "/" + idToString(id) );
    if ( !d.exists() )
        return "";
    QStringList files = d.entryList(QStringList() << "*.tex", QDir::Files);
    return (files.size() == 1) ? d.absoluteFilePath( files.first() ) : QString();
}

QString Cache::previewFileName(quint64 id) const
{
    QString sfn = sourceFileName(id);
    if ( sfn.isEmpty() )
    {
        QString path = cachePath(SamplesCachePath);
        if ( path.isEmpty() )
            return "";
        QDir d( path + "/" + idToString(id) );
        if ( !d.exists() )
            return "";
        QStringList files = d.entryList(QStringList() << "*.pdf", QDir::Files);
        return (files.size() == 1) ? d.absoluteFilePath( files.first() ) : QString();
    }
    QFileInfo sfi(sfn);
    QFileInfo fi(sfi.path() + "/" + sfi.baseName() + ".pdf");
    return ( fi.exists() && fi.isFile() ) ? fi.filePath() : QString();
}

QStringList Cache::auxFileNames(quint64 id) const
{
    QStringList list;
    QString pfn = previewFileName(id);
    if ( pfn.isEmpty() )
        return list;
    QString sfn = sourceFileName(id);
    QDir d( QFileInfo(sfn).path() );
    foreach ( const QString &fn, d.entryList(QDir::Files) )
        list << d.absoluteFilePath(fn);
    list.removeAll(sfn);
    list.removeAll(pfn);
    return list;
}

void Cache::removeCache(quint64 id)
{
    if (!id)
        return;
    QString path = cachePath(SamplesCachePath);
    if ( path.isEmpty() )
        return;
    BDirTools::rmdir( path + "/" + idToString(id) );
}
