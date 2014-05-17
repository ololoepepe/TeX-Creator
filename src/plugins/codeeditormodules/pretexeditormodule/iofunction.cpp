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

#include "iofunction.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "executionstack.h"
#include "global.h"

#include <BeQtGlobal>
#include <BDirTools>

#include <QList>
#include <QString>

B_DECLARE_TRANSLATE_FUNCTION

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool readFile(const PretexVariant &obligatoryArgument, const PretexVariant &optionalArgument,
                     PretexVariant &result, QString *err)
{
    if (obligatoryArgument.type() != PretexVariant::String)
        return bRet(err, translate("readFile", "File name must be a string", "error"), false);
    QString fn = obligatoryArgument.toString();
    if (fn.isEmpty())
        return bRet(err, translate("readFile", "File name can not be empty", "error"), false);
    QString codec;
    if (!optionalArgument.isNull())
    {
        if (optionalArgument.type() != PretexVariant::String)
            return bRet(err, translate("readFile", "Encoding name must be a string", "error"), false);
        codec = optionalArgument.toString();
    }
    if (codec.isEmpty())
        codec = "UTF-8";
    bool ok = false;
    QString s = BDirTools::readTextFile(fn, codec, &ok);
    if (!ok)
        return bRet(err, translate("readFile", "Failed to read file", "error"), false);
    result = PretexVariant(s);
    return bRet(err, QString(), true);
}

/*============================================================================
================================ IOFunction ==================================
============================================================================*/

/*============================== Public constructors =======================*/

IOFunction::IOFunction(Type t)
{
    mtype = t;
}

/*============================== Public methods ============================*/

QString IOFunction::name() const
{
    switch (mtype)
    {
    case ReadFileType:
        return "readFile";
    default:
        break;
    }
    return QString();
}

int IOFunction::obligatoryArgumentCount() const
{
    switch (mtype)
    {
    case ReadFileType:
        return 1;
    default:
        break;
    }
    return 0;
}

int IOFunction::optionalArgumentCount() const
{
    switch (mtype)
    {
    case ReadFileType:
        return 1;
    default:
        break;
    }
    return 0;
}

bool IOFunction::execute(ExecutionStack *, const QList<PretexVariant> &obligatoryArguments,
                         const QList<PretexVariant> &optionalArguments, PretexVariant &result, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case ReadFileType:
        return readFile(obligatoryArguments.first(), Global::firstIfAny(optionalArguments), result, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
