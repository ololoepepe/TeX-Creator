/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
**
** This file is part of TeX Creator.
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

#include "dialog.h"

#include <BDialog>

#include <QByteArray>
#include <QCloseEvent>
#include <QObject>

/*============================================================================
================================ Dialog ======================================
============================================================================*/

/*============================== Public constructors =======================*/

Dialog::Dialog(StoreGeometryFunction f, QWidget *parent) :
    BDialog(parent), StoreGeometryFunc(f)
{
    //
}

/*============================== Protected methods =========================*/

void Dialog::closeEvent(QCloseEvent *event)
{
    if (StoreGeometryFunc)
        StoreGeometryFunc(saveGeometry());
    BDialog::closeEvent(event);
}
