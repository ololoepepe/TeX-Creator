/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
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

#ifndef DIALOG_H
#define DIALOG_H

class QByteArray;
class QCloseEvent;
class QWidget;

#include <BDialog>

#include <QObject>

/*============================================================================
================================ Dialog ======================================
============================================================================*/

class Dialog : public BDialog
{
    Q_OBJECT
public:
    typedef void (*StoreGeometryFunction)(const QByteArray &geometry);
private:
    const StoreGeometryFunction StoreGeometryFunc;
public:
    explicit Dialog(StoreGeometryFunction f, QWidget *parent = 0);
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // DIALOG_H
