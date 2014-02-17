#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

namespace Global
{

QString toDouble(const QString &text, double &d, bool *native = 0);
QString toInt(const QString &text, int &i, bool *native = 0);
QString toBool(const QString &text, bool &b, bool *native = 0);
QString formatText(QString &text, const QString &format = QString());

}

#endif // GLOBAL_H
