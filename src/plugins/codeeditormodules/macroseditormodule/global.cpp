#include "global.h"

#include <BeQtGlobal>

#include <QString>
#include <QStringList>
#include <QRegExp>

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
    if (QRegExp("i(\\.([1-9]|1[0-5]))?").exactMatch(format))
    {
        QStringList sl = format.split('.');
        int prec = 0;
        if (sl.size() == 2)
            prec = sl.last().toInt();
        bool ok = false;
        int i = (text.contains('.') || "inf" == text) ? (int) text.toDouble(&ok) : text.toInt(&ok);
        if (!ok)
            return "Unable to convert";
        text = QString::number(i);
        if (prec)
            text += "." + QString().fill('0', prec);
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
    else if (QRegExp("(f|e|E|g|G)(\\.([1-9]|1[0-5]))?").exactMatch(format))
    {
        QStringList sl = format.split('.');
        char f = sl.first().at(0).toAscii();
        int prec = 6;
        if (sl.size() == 2)
            prec = sl.last().toInt();
        bool ok = false;
        double d = text.toDouble(&ok);
        if (!ok)
            return "Unable to convert";
        text = QString::number(d, f, prec);
    }
    else
    {
        return "Unknown format";
    }
    return "";
}

QString toRawText(QString s)
{
    s.replace("\\\\", "\\");
    s.replace("\\n", "\n");
    s.replace("\\t", "\t");
    s.replace("\\%", "%");
    s.replace("\\$", "$");
    s.remove("\\e");
    s.replace("\\{", "{");
    s.replace("\\}", "}");
    s.replace("\\[", "[");
    s.replace("\\]", "]");
    return s;
}

QString toVisibleText(QString s)
{
    s.remove('\r');
    s.replace('\n', "\\n");
    s.replace('\t', "\\t");
    s.replace('\\', "\\\\");
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
