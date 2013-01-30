#include "sample.h"

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <QVariant>

/*============================================================================
================================ Sample ======================================
============================================================================*/

/*============================== Public constructors =======================*/

Sample::Sample()
{
    mid = 0;
    mtype = Unverified;
    mrating = 0;
    mmodified.setTimeSpec(Qt::UTC);
}

Sample::Sample(const Sample &other)
{
    *this = other;
}

Sample::~Sample()
{
    //
}

/*============================== Static public methods =====================*/

Sample Sample::fromVariantMap(const QVariantMap &m)
{
    Sample s;
    s.mid = m.value("id").toULongLong();
    s.mtitle = m.value("title").toString();
    s.mauthor = m.value("author").toString();
    s.mtype = static_cast<Type>( m.value("type").toInt() );
    s.mtags = m.value("tags").toStringList();
    s.mcomment = m.value("comment").toString();
    s.mremark = m.value("admin_remark").toString();
    s.setRating( m.value("rating").toInt() );
    s.mmodified.setMSecsSinceEpoch( m.value("modified_dt").toLongLong() );
    return s;
}

/*============================== Public methods ============================*/

void Sample::setId(quint64 id)
{
    mid = id;
}

void Sample::setTitle(const QString &s)
{
    mtitle = s;
}

void Sample::setAuthor(const QString &s)
{
    mauthor = s;
}

void Sample::setType(Type t)
{
    mtype = t;
}

void Sample::setTags(const QStringList &list)
{
    mtags = list;
}

void Sample::setComment(const QString &s)
{
    mcomment = s;
}

void Sample::setAdminRemark(const QString &s)
{
    mremark = s;
}

void Sample::setRating(int r)
{
    if (r < 0 || r > 100)
        return;
    mrating = r;
}

void Sample::setLastModified(const QDateTime &dt)
{
    mmodified = dt;
}

quint64 Sample::id() const
{
    return mid;
}

QString Sample::title() const
{
    return mtitle;
}

QString Sample::author() const
{
    return mauthor;
}

Sample::Type Sample::type() const
{
    return mtype;
}

QStringList Sample::tags() const
{
    return mtags;
}

QString Sample::comment() const
{
    return mcomment;
}

QString Sample::adminRemark() const
{
    return mremark;
}

int Sample::rating() const
{
    return mrating;
}

QDateTime Sample::lastModified() const
{
    return mmodified;
}

QString Sample::idToString(int fixedLength) const
{
    QString s = QString::number(mid);
    int dlen = fixedLength - s.length();
    if (dlen > 0)
        s.prepend( QString().fill('0', dlen) );
    return s;
}

QString Sample::typeToString(TypeFormat format) const
{
    switch (format)
    {
    case LocalizedFormat:
        switch ( type() )
        {
        case Approved:
            return tr("Approved", "type");
        case Rejected:
            return tr("Rejected", "type");
        case Unverified:
        default:
            return tr("Unverified", "type");
        }
    case PortableFormat:
    default:
        switch (mtype)
        {
        case Approved:
            return "approved";
        case Rejected:
            return "rejected";
        case Unverified:
        default:
            return "unverified";
        }
    }
}

QString Sample::ratingToString() const
{
    return QString::number(mrating);
}

QVariantMap Sample::toVariantMap() const
{
    QVariantMap m;
    m.insert("id", mid);
    m.insert("title", mtitle);
    m.insert("author", mauthor);
    m.insert("type", (int) mtype);
    m.insert("tags", mtags);
    m.insert("comment", mcomment);
    m.insert("admin_remark", mremark);
    m.insert("rating", mrating);
    m.insert( "modified_dt", mmodified.toMSecsSinceEpoch() );
    return m;
}

bool Sample::isValid() const
{
    return mid && !mtitle.isEmpty() && !mauthor.isEmpty();
}

bool Sample::matchesKeywords(const QStringList &keywords, bool caseSensitive) const
{
    if ( keywords.isEmpty() )
        return true;
    return containsKeywords(mtitle, keywords, caseSensitive) || containsKeywords(mauthor, keywords, caseSensitive);
}

/*============================== Public operators ==========================*/

Sample &Sample::operator=(const Sample &other)
{
    mid = other.mid;
    mtitle = other.mtitle;
    mauthor = other.mauthor;
    mtype = other.mtype;
    mtags = other.mtags;
    mcomment = other.mcomment;
    mremark = other.mremark;
    mrating = other.mrating;
    mmodified = other.mmodified;
    return *this;
}

bool Sample::operator==(const Sample &other) const
{
    return other.mid == mid;
}

/*============================== Static private methods ====================*/

bool Sample::containsKeywords(const QString &string, const QStringList &keywords, bool caseSensitive)
{
    foreach (const QString &kw, keywords)
        if ( string.contains(kw, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive) )
            return true;
    return false;
}
