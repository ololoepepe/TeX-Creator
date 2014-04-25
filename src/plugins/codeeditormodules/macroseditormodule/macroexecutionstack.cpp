/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
**
** This file is part of the MacrosEditorModule plugin of TeX Creator.
**
** TeX Creator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** TeX Creator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with TeX Creator.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

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
