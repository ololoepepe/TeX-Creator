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

#include "global.h"
#include "application.h"

#include <BCodeEdit>
#include <BCodeEditor>
#include <BGuiTools>
#include <BPassword>
#include <BPasswordWidget>

#include <QFont>
#include <QString>
#include <QTextCodec>
#include <QStringList>
#include <QByteArray>
#include <QVariant>
#include <QSettings>

#include <QDebug>

namespace Global
{

QByteArray pwdState;

QStringList splitArguments(const QString &string)
{
    QStringList list = string.split(' ', QString::SkipEmptyParts);
    if (list.isEmpty())
        return list;
    int x = -1;
    foreach (int i, bRangeR(list.size() - 1, 0))
    {
        QString &arg = list[i];
        if (x < 0 && arg.at(arg.length() - 1) == '\"')
            x = i;
        if (x >= 0 && arg.at(0) == '\"')
        {
            if (x != i)
                foreach (int j, bRange(x, i + 1))
                    arg.append(" " + list.takeAt(j));
            x = -1;
        }
    }
    return list;
}

QString joinArguments(const QStringList &list)
{
    QStringList nlist = list;
    if ( !nlist.isEmpty() )
    {
        foreach (int i, bRangeD(0, list.size() - 1))
        {
            QString &arg = nlist[i];
            if (arg.contains(' ') && (arg.at(0) != '\"' || arg.at(arg.length() - 1) != '\"'))
                arg.prepend('\"').append('\"');
        }
    }
    return nlist.join(" ");
}

int indexOfHelper(const QString &text, const QString &what, int from)
{
    if (text.isEmpty() || what.isEmpty())
        return -1;
    int ind = text.indexOf(what, from);
    while (ind >= 0)
    {
        if (!ind || text.at(ind - 1) != '\\')
            return ind;
        ind = text.indexOf(what, ++from);
    }
    return -1;
}

//Console

/*void setCompilerParameters(const TCompilerParameters &param)
{
    bSettings->setValue("Console/compiler_parameters", param);
}

void setCompiler(int c)
{
    TCompilerParameters param = compilerParameters();
    param.setCompiler(c);
    setCompilerParameters(param);
}

void setCompilerOptions(const QStringList &list)
{
    TCompilerParameters param = compilerParameters();
    param.setOptions(list);
    setCompilerParameters(param);
}

void setCompilerOptions(const QString &string)
{
    setCompilerOptions(QStringList() << string);
}

void setCompilerCommands(const QStringList &list)
{
    TCompilerParameters param = compilerParameters();
    param.setCommands(list);
    setCompilerParameters(param);
}

void setCompilerCommands(const QString &string)
{
    setCompilerCommands(QStringList() << string);
}

void setMakeindexEnabled(bool enabled)
{
    TCompilerParameters param = compilerParameters();
    param.setMakeindexEnabled(enabled);
    setCompilerParameters(param);
}

void setDvipsEnabled(bool enabled)
{
    TCompilerParameters param = compilerParameters();
    param.setDvipsEnabled(enabled);
    setCompilerParameters(param);
}

void setUseRemoteCompiler(bool b)
{
    bSettings->setValue("Console/use_remote_compiler", b);
    Application::emitUseRemoteCompilerChanged();
}

void setFallbackToLocalCompiler(bool b)
{
    bSettings->setValue("Console/fallback_to_local_compiler", b);
}

void setAlwaysLatinEnabled(bool enabled)
{
    bSettings->setValue("Console/always_latin_enabled", enabled);
}*/

//Console

/*bool hasFallbackToLocalCompiler()
{
    return bSettings->contains("Console/fallback_to_local_compiler");
}

TCompilerParameters compilerParameters()
{
    return bSettings->value("Console/compiler_parameters").value<TCompilerParameters>();
}

TCompilerParameters::Compiler compiler()
{
    return compilerParameters().compiler();
}

QStringList compilerOptions()
{
    return compilerParameters().options();
}

QString compilerOptionsString(bool command)
{
    return compilerParameters().optionsString(command);
}

QStringList compilerCommands()
{
    return compilerParameters().commands();
}

QString compilerCommandsString(bool command)
{
    return compilerParameters().commandsString(command);
}

bool makeindexEnabled()
{
    return compilerParameters().makeindexEnabled();
}

bool dvipsEnabled()
{
    return compilerParameters().dvipsEnabled();
}

bool useRemoteCompiler()
{
    return bSettings->value("Console/use_remote_compiler").toBool();
}

bool fallbackToLocalCompiler()
{
    return bSettings->value("Console/fallback_to_local_compiler").toBool();
}

bool alwaysLatinEnabled()
{
    return bSettings->value("Console/always_latin_enabled", false).toBool();
}*/

}
