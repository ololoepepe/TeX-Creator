#include "pretexvariant.h"

#include <QString>
#include <QVariant>
#include <QDataStream>
#include <QDebug>

/*============================================================================
================================ PretexVariant ===============================
============================================================================*/

/*============================== Public constructors =======================*/

PretexVariant::PretexVariant()
{
    //
}

PretexVariant::PretexVariant(const PretexVariant &other)
{
    *this = other;
}

PretexVariant::PretexVariant(Type t)
{
    mvariant = QVariant(t);
}

PretexVariant::PretexVariant(const QString &s)
{
    mvariant = QVariant(s);
}

PretexVariant::PretexVariant(int i)
{
    mvariant = QVariant(i);
}

PretexVariant::PretexVariant(double d)
{
    mvariant = QVariant(d);
}

/*============================== Public methods ============================*/

bool PretexVariant::canConvert(Type t) const
{
    return mvariant.canConvert(static_cast<QVariant::Type>(t));
}

void PretexVariant::clear()
{
    return mvariant.clear();
}

bool PretexVariant::convert(Type t)
{
    return mvariant.convert(static_cast<QVariant::Type>(t));
}

bool PretexVariant::isNull() const
{
    return mvariant.isNull();
}

bool PretexVariant::isValid() const
{
    return mvariant.isValid();
}

QString PretexVariant::toString() const
{
    return mvariant.toString();
}

int PretexVariant::toInt(bool *ok) const
{
    return mvariant.toInt(ok);
}

double PretexVariant::toReal(bool *ok) const
{
    return mvariant.toDouble(ok);
}

PretexVariant::Type PretexVariant::type() const
{
    return static_cast<Type>(mvariant.type());
}

const char *PretexVariant::typeName() const
{
    return mvariant.typeName();
}

/*============================== Public operators ==========================*/

bool PretexVariant::operator!=(const PretexVariant &other) const
{
    return !(*this == other);
}

PretexVariant &PretexVariant::operator= (const PretexVariant &other)
{
    mvariant = other.mvariant;
    return *this;
}

bool PretexVariant::operator== (const PretexVariant &other) const
{
    return mvariant == other.mvariant;
}

/*============================== Public friend operators ===================*/

QDataStream &operator<< (QDataStream &s, const PretexVariant &v)
{
    s << v.mvariant;
    return s;
}

QDataStream &operator>> (QDataStream &s, PretexVariant &v)
{
    QVariant vv;
    s >> vv;
    switch (vv.type())
    {
    case QVariant::String:
    case QVariant::Int:
    case QVariant::Double:
        v.mvariant = vv;
        break;
    default:
        v.mvariant = QVariant();
        break;
    }
    return s;
}

QDebug operator<< (QDebug dbg, const PretexVariant &v)
{
    switch (v.type())
    {
    case PretexVariant::String:
        dbg.nospace() << "PretexVariant(" << v.typeName() << ", " << v.toString() << ")";
        break;
    case PretexVariant::Int:
        dbg.nospace() << "PretexVariant(" << v.typeName() << ", " << v.toInt() << ")";
        break;
    case PretexVariant::Real:
        dbg.nospace() << "PretexVariant(" << v.typeName() << ", " << v.toReal() << ")";
        break;
    default:
        dbg.nospace() << "PretexVariant()";
        break;
    }
    return dbg.space();
}
