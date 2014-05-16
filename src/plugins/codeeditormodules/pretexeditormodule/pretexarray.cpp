#include "pretexarray.h"
#include "pretexarray.h"

#include <BeQtGlobal>

#include <QList>
#include <QDataStream>
#include <QDebug>

static int product(const PretexArray::Dimensions &dim, int current)
{
    if (current < 0 || current >= dim.size())
        return -1;
    if (dim.size() - 1 == current)
        return 1;
    int n = 1;
    foreach (int i, bRangeD(current + 1, dim.size() - 1))
        n *= dim.at(i);
    return n;
}

static int index(const PretexArray::Dimensions &dim, const PretexArray::Indexes &indexes)
{
    if (dim.isEmpty() || dim.size() != indexes.size())
        return -1;
    int n = 0;
    foreach (int i, bRangeD(0, dim.size() - 1))
    {
        int ind = indexes.at(i);
        if (ind < 0 || ind >= dim.at(i))
            return -1;
        int p = product(dim, i);
        if (p <= 0)
            return -1;
        n += ind * p;
    }
    return n;
}

/*============================================================================
================================ PretexArray =================================
============================================================================*/

/*============================== Public constructors =======================*/

PretexArray::PretexArray(const Dimensions &dimensions)
{
    if (dimensions.isEmpty())
        return;
    int n = 1;
    foreach (int dim, dimensions)
    {
        if (dim < 1)
            return;
        else
            n *= dim;
    }
    mdim = dimensions;
    foreach (int i, bRangeD(1, n))
    {
        Q_UNUSED(i)
        mdata << PretexVariant();
    }
}

PretexArray::PretexArray(const PretexArray &other)
{
    *this = other;
}

/*============================== Public methods ============================*/

const PretexVariant &PretexArray::at(const Indexes &indexes) const
{
    return mdata.at(index(mdim, indexes));
}

void PretexArray::clear()
{
    foreach (int i, bRangeD(0, mdata.size() - 1))
        mdata[i].clear();
}

int PretexArray::dimension(int indexNo)
{
    if (indexNo < 0 || indexNo >= mdim.size())
        return 0;
    return mdim.at(indexNo);
}

int PretexArray::dimensionCount() const
{
    return mdim.size();
}

PretexArray::Dimensions PretexArray::dimensions() const
{
    return mdim;
}

int PretexArray::elementCount() const
{
    if (!isValid())
        return 0;
    int n = 1;
    foreach (int dim, mdim)
        n *= dim;
    return n;
}

bool PretexArray::isValid() const
{
    return !mdim.isEmpty();
}

PretexVariant PretexArray::value(const Indexes &indexes) const
{
    return mdata.value(index(mdim, indexes));
}

/*============================== Public operators ==========================*/

bool PretexArray::operator!=(const PretexArray &other) const
{
    return !(*this == other);
}

PretexArray &PretexArray::operator= (const PretexArray &other)
{
    mdim = other.mdim;
    mdata = other.mdata;
    return *this;
}

bool PretexArray::operator== (const PretexArray &other) const
{
    return mdim == other.mdim && mdata == other.mdata;
}

PretexVariant &PretexArray::operator[] (const Indexes &indexes)
{
    return mdata[index(mdim, indexes)];
}

const PretexVariant &PretexArray::operator[] (const Indexes &indexes) const
{
    return mdata[index(mdim, indexes)];
}

/*============================== Public friend operators ===================*/

QDataStream &operator<< (QDataStream &s, const PretexArray &a)
{
    s << a.mdim;
    s << a.mdata;
    return s;
}

QDataStream &operator>> (QDataStream &s, PretexArray &a)
{
    PretexArray::Dimensions dim;
    QList<PretexVariant> data;
    s >> dim;
    s >> data;
    PretexArray aa(dim);
    if (!aa.isValid() || data.size() != aa.elementCount())
        return s;
    a = aa;
    return s;
}

QDebug operator<< (QDebug dbg, const PretexArray &a)
{
    if (a.isValid())
    {
        QString s = "[" + QString::number(a.mdim.first());
        foreach (int i, bRangeD(1, a.mdim.size() - 1))
            s += "x" + QString::number(a.mdim.at(i));
        s += "]";
        dbg.nospace() << "PretexArray(" << s.toLatin1().constData() << ")";
        //TODO
    }
    else
    {
        dbg.nospace() << "PretexArray()";
    }
    return dbg.space();
    return dbg.space();
}
