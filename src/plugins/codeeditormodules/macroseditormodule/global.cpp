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

#include "global.h"

#include <BeQtGlobal>

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QMap>

#include <QDebug>

namespace Global
{

QString toDouble(const QString &text, double &d, bool *native)
{
    if (text.isEmpty())
        return bRet(native, false, QString("Failed to convert"));
    if (text.toLower() == "true")
    {
        d = 1.0;
        return bRet(native, false, QString());
    }
    else if (text.toLower() == "false")
    {
        d = 0.0;
        return bRet(native, false, QString());
    }
    if (text.contains('.'))
    {
        bool ok = false;
        double dd = text.toDouble(&ok);
        if (!ok)
            return bRet(native, true, QString("Failed to convert"));
        d = dd;
        return bRet(native, true, QString());
    }
    else
    {
        bool ok = false;
        int i = text.toInt(&ok);
        if (!ok)
            return bRet(native, false, QString("Failed to convert"));
        d = (double) i;
        return bRet(native, false, QString());
    }
}

QString toInt(const QString &text, int &i, bool *native)
{
    if (text.isEmpty())
        return bRet(native, false, QString("Failed to convert"));
    if (text.toLower() == "true")
    {
        i = 1;
        return bRet(native, false, QString());
    }
    else if (text.toLower() == "false")
    {
        i = 0;
        return bRet(native, false, QString());
    }
    if (text.contains('.'))
    {
        bool ok = false;
        double d = text.toDouble(&ok);
        if (!ok)
            return bRet(native, false, QString("Failed to convert"));
        i = (int) d;
        return bRet(native, false, QString());
    }
    else
    {
        bool ok = false;
        int ii = text.toInt(&ok);
        if (!ok)
            return bRet(native, true, QString("Failed to convert"));
        i = ii;
        return bRet(native, true, QString());
    }
}

QString toBool(const QString &text, bool &b, bool *native)
{
    if (text.isEmpty())
        return bRet(native, false, QString("Failed to convert"));
    if (text.toLower() == "true")
    {
        b = true;
        return bRet(native, true, QString());
    }
    else if (text.toLower() == "false")
    {
        b = false;
        return bRet(native, true, QString());
    }
    if (text.contains('.'))
    {
        bool ok = false;
        double d = text.toDouble(&ok);
        if (!ok)
            return bRet(native, false, QString("Failed to convert"));
        b = (bool) d;
        return bRet(native, false, QString());
    }
    else
    {
        bool ok = false;
        int i = text.toInt(&ok);
        if (!ok)
            return bRet(native, false, QString("Failed to convert"));
        b = (bool) i;
        return bRet(native, false, QString());
    }
}

QString formatText(QString &text, const QString &format)
{
    if (text.isEmpty() || format.isEmpty())
        return "";
    if (QRegExp("(([1-9][0-9]*)\\.)?i(\\.([1-9]|1[0-5]))?").exactMatch(format))
    {
        QStringList sl = format.split('.');
        int prep = 0;
        int prec = 0;
        if (sl.size() == 3)
        {
            prep = sl.first().toInt();
            prec = sl.last().toInt();
        }
        else if (sl.size() == 2)
        {
            if (sl.first() == "i")
                prec = sl.last().toInt();
            else
                prep = sl.first().toInt();
        }
        bool ok = false;
        int i = (text.contains('.') || "inf" == text) ? (int) text.toDouble(&ok) : text.toInt(&ok);
        if (!ok)
            return "Unable to convert";
        text = QString::number(i);
        if (prec)
            text += "." + QString().fill('0', prec);
        if (prep)
            text.prepend(QString().fill('0', prep - text.length()));
    }
    else if ("b" == format.toLower())
    {
        bool b = false;
        QString err = toBool(text, b);
        if (!err.isEmpty())
            return err;
        text = b ? "true" : "false";
        return "";
    }
    else if (QRegExp("(([1-9][0-9]*)\\.)?(f|e|E|g|G)(\\.([1-9]|1[0-5]))?").exactMatch(format))
    {
        QStringList sl = format.split('.');
        int prep = 0;
        char f = '\0';
        int prec = 6;
        if (sl.size() == 3)
        {
            prep = sl.first().toInt();
            f = sl.at(1).at(0).toAscii();
            prec = sl.last().toInt();
        }
        else if (sl.size() == 2)
        {
            if (QRegExp("f|e|E|g|G").exactMatch(sl.first()))
            {
                f = sl.first().at(0).toAscii();
                prec = sl.last().toInt();
            }
            else
            {
                prep = sl.first().toInt();
                f = sl.last().at(0).toAscii();
            }
        }
        bool ok = false;
        double d = text.toDouble(&ok);
        if (!ok)
            return "Unable to convert";
        text = QString::number(d, f, prec);
        if (prep)
            text.prepend(QString().fill('0', prep - text.length()));
    }
    else
    {
        return "Unknown format";
    }
    return "";
}

QString toRawText(QString s)
{
    typedef QMap<char, char> Map;
    init_once(Map, map, Map())
    {
        map.insert('\\', '\\');
        map.insert('n', '\n');
        map.insert('t', '\t');
        map.insert('%', '%');
        map.insert('$', '$');
        map.insert('{', '{');
        map.insert('}', '}');
        map.insert('[', '[');
        map.insert(']', ']');
    }
    int i = 0;
    while (i < s.length())
    {
        if (s.at(i) == '\\' && i < s.length() - 1)
        {
            char c = s.at(i + 1).toLatin1();
            if (c == 'e')
                s.remove(i, 2);
            else if (map.contains(c))
                s.replace(i++, 2, map.value(c));
            else
                ++i;
        }
        else
        {
            ++i;
        }
    }
    return s;
}

QString toVisibleText(QString s)
{
    s.remove('\r');
    s.replace('\\', "\\\\");
    s.replace('\n', "\\n");
    s.replace('\t', "\\t");
    s.replace('%', "\\%");
    s.replace('$', "\\$");
    s.replace('{', "\\{");
    s.replace('}', "\\}");
    s.replace('[', "\\[");
    s.replace(']', "\\]");
    return s;
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

}
