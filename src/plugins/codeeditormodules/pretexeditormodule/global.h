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

#include "pretexvariant.h"

#include <QString>
#include <QList>

/*============================================================================
================================ Global ======================================
============================================================================*/

namespace Global
{

PretexVariant::Type typeToCastTo(PretexVariant::Type preferredType, const QList<PretexVariant> &obligatoryArguments,
                                 const QList<PretexVariant> &optionalArguments = QList<PretexVariant>());

}

#endif // GLOBAL_H
