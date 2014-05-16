/****************************************************************************
**
** Copyright (C) 2014 TeXSample Team
**
** This file is part of the PreTeX Editor Module plugin of TeX Creator.
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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

/*============================================================================
================================ Global ======================================
============================================================================*/

namespace Global
{

QString toDouble(const QString &text, double &d, bool *native = 0);
QString toInt(const QString &text, int &i, bool *native = 0);
QString toBool(const QString &text, bool &b, bool *native = 0);
QString formatText(QString &text, const QString &format = QString());
QString toRawText(QString s);
QString toVisibleText(QString s);
int indexOfHelper(const QString &text, const QString &what, int from = 0);

}

#endif // GLOBAL_H
