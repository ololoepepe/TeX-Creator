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

#ifndef PRETEXBUILTINFUNCTION_H
#define PRETEXBUILTINFUNCTION_H

class QString;

/*============================================================================
================================ PretexBuiltinFunction =======================
============================================================================*/

class PretexBuiltinFunction
{
public:
    static PretexBuiltinFunction *functionForName(const QString &name);
    static bool isBuiltinFunction(const QString &name);
public:
    explicit PretexBuiltinFunction();
public:
    virtual QString name() const = 0;
    virtual int obligatoryArgumentCount() const = 0;
    virtual int optionalArgumentCount() const = 0;
};

#endif // PRETEXBUILTINFUNCTION_H
