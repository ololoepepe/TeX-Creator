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

#include "executionmodule.h"

#include <BeQtGlobal>

#include <QStringList>
#include <QString>

/*============================================================================
================================ ExecutionModule =============================
============================================================================*/

/*============================== Public constructors =======================*/

ExecutionModule::ExecutionModule()
{
    //
}

/*============================== Static public methods =====================*/

QStringList ExecutionModule::specFuncNames()
{
    static const QStringList names = QStringList() << "set" << "renewFunc" << "delete"
    << "newVar" << "newLocalVar" << "newGlobalVar" << "tryNewVar" << "tryNewLocalVar" << "tryNewGlobalVar"
    << "newFunc" << "newLocalFunc" << "newGlobalFunc" << "tryNewFunc" << "tryNewLocalFunc" << "tryNewGlobalFunc"
    << "newArray" << "newLocalArray" << "newGlobalArray" << "tryNewArray" << "tryNewLocalArray" << "tryNewGlobalArray";
    return names;
}

QStringList ExecutionModule::normalFuncNames()
{
    static const QStringList names = QStringList() << "+" << "-" << "*";
    return names;
}

QStringList ExecutionModule::funcNames()
{
    init_once(QStringList, names, QStringList())
        names << normalFuncNames() << specFuncNames();
    return names;
}
