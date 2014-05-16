#ifndef PRETEXVARIANT_H
#define PRETEXVARIANT_H

class QString;
class QDataStream;
class QDebug;

#include <QVariant>
#include <QMetaType>

/*============================================================================
================================ PretexVariant ===============================
============================================================================*/

class PretexVariant
{
public:
    enum Type
    {
        Invalid = QVariant::Invalid,
        String = QVariant::String,
        Int = QVariant::Int,
        Real = QVariant::Double
    };
public:
    explicit PretexVariant();
    PretexVariant(const PretexVariant &other);
    explicit PretexVariant(Type t);
    explicit PretexVariant(const QString &s);
    explicit PretexVariant(int i);
    explicit PretexVariant(double d);
public:
    bool canConvert(Type t) const;
    void clear();
    bool convert(Type t);
    bool isNull() const;
    bool isValid() const;
    QString toString() const;
    int toInt(bool *ok = 0) const;
    double toReal(bool *ok = 0) const;
    Type type() const;
    const char *typeName() const;
public:
    bool operator!=(const PretexVariant &other) const;
    PretexVariant &operator= (const PretexVariant &other);
    bool operator== (const PretexVariant &other) const;
public:
    friend QDataStream &operator<< (QDataStream &s, const PretexVariant &v);
    friend QDataStream &operator>> (QDataStream &s, PretexVariant &v);
    friend QDebug operator<< (QDebug dbg, const PretexVariant &v);
private:
    QVariant mvariant;
};

Q_DECLARE_METATYPE(PretexVariant)

#endif // PRETEXVARIANT_H
