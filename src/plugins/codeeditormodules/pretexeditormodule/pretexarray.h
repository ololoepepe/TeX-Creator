#ifndef PRETEXARRAY_H
#define PRETEXARRAY_H

class QDataStream;
class QDebug;

#include "pretexvariant.h"

#include <QList>
#include <QMetaType>

/*============================================================================
================================ PretexArray =================================
============================================================================*/

class PretexArray
{
public:
    typedef QList<int> Dimensions;
    typedef QList<int> Indexes;
public:
    explicit PretexArray(const Dimensions &dimensions = Dimensions());
    PretexArray(const PretexArray &other);
public:
    const PretexVariant &at(const Indexes &indexes) const;
    void clear();
    int dimension(int indexNo);
    int dimensionCount() const;
    Dimensions dimensions() const;
    int elementCount() const;
    bool isValid() const;
    PretexVariant value(const Indexes &indexes) const;
public:
    bool operator!=(const PretexArray &other) const;
    PretexArray &operator= (const PretexArray &other);
    bool operator== (const PretexArray &other) const;
    PretexVariant &operator[] (const Indexes &indexes);
    const PretexVariant &operator[] (const Indexes &indexes) const;
public:
    friend QDataStream &operator<< (QDataStream &s, const PretexArray &a);
    friend QDataStream &operator>> (QDataStream &s, PretexArray &a);
    friend QDebug operator<< (QDebug dbg, const PretexArray &a);
private:
    Dimensions mdim;
    QList<PretexVariant> mdata;
};

Q_DECLARE_METATYPE(PretexArray)

#endif // PRETEXARRAY_H
