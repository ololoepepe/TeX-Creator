#include "macroexecutionstack.h"

#include <BeQt>

#include <QMap>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#include <QDebug>

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
        return mparent->define(id, value);
    mmap.insert(id, value);
    return true;
}

bool MacroExecutionStack::defineF(const QString &id, const QString &value, bool global)
{
    if (id.isEmpty() || isDefined(id))
        return false;
    if (global && mparent)
        return mparent->defineF(id, value);
    mmapF.insert(id, value);
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
    else if (mmapF.contains(id))
    {
        mmapF.remove(id);
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

bool MacroExecutionStack::setF(const QString &id, const QString &value)
{
    if (id.isEmpty())
        return false;
    if (mmapF.contains(id))
    {
        mmapF[id] = value;
        return true;
    }
    else if (mparent)
    {
        return mparent->setF(id, value);
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

bool MacroExecutionStack::getF(const QString &id, QString &value) const
{
    if (id.isEmpty())
        return false;
    if (mmapF.contains(id))
    {
        value = mmapF.value(id);
        return true;
    }
    else if (mparent)
    {
        return mparent->getF(id, value);
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
    return mmap.contains(id) || mmapF.contains(id) || (mparent && mparent->isDefined(id));
}

QByteArray MacroExecutionStack::save() const
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(BeQt::DataStreamVersion);
    out << mmap;
    out << mmapF;
    return data;
}

void MacroExecutionStack::restore(const QByteArray &data)
{
    QDataStream in(data);
    in.setVersion(BeQt::DataStreamVersion);
    in >> mmap;
    in >> mmapF;
}

void MacroExecutionStack::clear()
{
    mmap.clear();
    mmapF.clear();
}
