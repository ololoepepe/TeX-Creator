#include "macroexecutionstack.h"

#include <BeQt>

#include <QMap>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

/*============================================================================
================================ MacroExecutionStack =========================
============================================================================*/

/*============================== Public constructors =======================*/

MacroExecutionStack::MacroExecutionStack(MacroExecutionStack *parent)
{
    mparent = parent;
}

/*============================== Public methods ============================*/

bool MacroExecutionStack::define(const QString &id, const QString &value, bool global)
{
    if (id.isEmpty() || isDefined(id))
        return false;
    if (global && mparent)
        mparent->define(id, value);
    else
        mmap.insert(id, value);
    return true;
}

bool MacroExecutionStack::undefine(const QString &id)
{
    if (id.isEmpty())
        return false;
    if (mmap.contains(id))
    {
        mmap.remove(id);
        return true;
    }
    else if (mparent)
    {
        return mparent->undefine(id);
    }
    else
    {
        return false;
    }
}

bool MacroExecutionStack::set(const QString &id, const QString &value)
{
    if (id.isEmpty())
        return false;
    if (mmap.contains(id))
    {
        mmap[id] = value;
        return true;
    }
    else if (mparent)
    {
        return mparent->set(id, value);
    }
    else
    {
        return false;
    }
}

bool MacroExecutionStack::get(const QString &id, QString &value) const
{
    if (id.isEmpty())
        return false;
    if (mmap.contains(id))
    {
        value = mmap.value(id);
        return true;
    }
    else if (mparent)
    {
        return mparent->get(id, value);
    }
    else
    {
        return false;
    }
}

bool MacroExecutionStack::isDefined(const QString &id) const
{
    if (id.isEmpty())
        return false;
    return mmap.contains(id) || (mparent && mparent->isDefined(id));
}

QByteArray MacroExecutionStack::save() const
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(BeQt::DataStreamVersion);
    out << mmap;
    return data;
}

void MacroExecutionStack::restore(const QByteArray &data)
{
    QDataStream in(data);
    in.setVersion(BeQt::DataStreamVersion);
    in >> mmap;
}
