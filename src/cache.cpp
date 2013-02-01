#include "cache.h"

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

QDateTime Cache::sampleListUpdateDateTime() const
{
    if ( !isValid() )
        return QDateTime();
    return msettings->value("List/update_dt").toDateTime();
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

bool Cache::setSampleListUpdateDateTime(const QDateTime &dt)
{
    if ( !isValid() )
        return false;
    msettings->setValue("list_update_dt", dt);
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
    QString path = cachePath();
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
        if ( !BDirTools::writeFile( m.value("file_name").toString(), m.value("data").toByteArray() ) )
            return false;
    }
    if ( QFileInfo(pfn).baseName() != QFileInfo(fn).baseName() )
        QFile::rename(pfn, path + "/" + QFileInfo(fn).baseName() + ".pdf");
    return true;
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

/*============================== Private methods ===========================*/

QString Cache::cachePath() const
{
    if ( mhost.isEmpty() )
        return "";
    QString path = BDirTools::findResource("texsample", BDirTools::UserOnly);
    if ( path.isEmpty() )
        return "";
    return path + "/" + mhost;
}

QString Cache::sourceFileName(quint64 id) const
{
    if (!id)
        return "";
    QString path = cachePath();
    if ( path.isEmpty() )
        return "";
    QDir d( path + "/" + idToString(id) );
    if ( !d.exists() )
        return "";
    QStringList files = d.entryList(QStringList() << "*.tex", QDir::Files);
    return (files.size() == 1) ? files.first() : QString();
}

QString Cache::previewFileName(quint64 id) const
{
    QString sfn = sourceFileName(id);
    if ( sfn.isEmpty() )
        return "";
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
