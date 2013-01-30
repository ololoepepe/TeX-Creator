#ifndef SAMPLE_H
#define SAMPLE_H

#include <Qt>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QApplication>
#include <QVariantMap>

/*============================================================================
================================ Sample ======================================
============================================================================*/

class Sample
{
    Q_DECLARE_TR_FUNCTIONS(Sample)
public:
    enum Type
    {
        Unverified = 0,
        Approved,
        Rejected
    };
    enum TypeFormat
    {
        PortableFormat,
        LocalizedFormat
    };
public:
    explicit Sample();
    Sample(const Sample &other);
    virtual ~Sample();
public:
    static Sample fromVariantMap(const QVariantMap &m);
    static QString typeToLocalizedString(Type t, bool singular = true);
public:
    void setId(quint64 id);
    void setTitle(const QString &s);
    void setAuthor(const QString &s);
    void setType(Type t);
    void setTags(const QStringList &list);
    void setComment(const QString &s);
    void setAdminRemark(const QString &s);
    void setRating(int r);
    void setLastModified(const QDateTime &dt);
    quint64 id() const;
    QString title() const;
    QString author() const;
    Type type() const;
    QStringList tags() const;
    QString comment() const;
    QString adminRemark() const;
    int rating() const;
    QDateTime lastModified() const;
    QString idToString(int fixedLength = -1) const;
    QString typeToString(TypeFormat format = PortableFormat) const;
    QString ratingToString() const;
    QVariantMap toVariantMap() const;
    bool isValid() const;
    bool matchesKeywords(const QStringList &keywords, bool caseSensitive = false) const;
public:
    Sample &operator=(const Sample &other);
    bool operator==(const Sample &other) const;
private:
    static bool containsKeywords(const QString &string, const QStringList &keywords, bool caseSensitive = false);
private:
    quint64 mid;
    QString mtitle;
    QString mauthor;
    Type mtype;
    QStringList mtags;
    QString mcomment;
    QString mremark;
    int mrating;
    QDateTime mmodified;
};

#endif // SAMPLE_H
