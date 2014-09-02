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

class QStringList;

#include "application.h"

#include <BApplicationServer>

#include <QDebug>
#include <QDir>
#include <QHash>
#include <QObject>
#include <QString>

int main(int argc, char *argv[])
{
    static const QString AppName = "TeX Creator";
    QString home = QDir::home().dirName();
    BApplicationServer s(9950 + qHash(home) % 10, AppName + "4" + home);
    int ret = 0;
    if (!s.testServer()) {
        Application app(argc, argv, AppName, "Andrey Bogdanov");
        QObject::connect(&s, SIGNAL(messageReceived(QStringList)), &app, SLOT(messageReceived(QStringList)));
        s.listen();
        ret = app.exec();
    } else {
        s.sendMessage(argc, argv);
    }
    return ret;
}
