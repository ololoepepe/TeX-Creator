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

#include "application.h"
#include "applicationserver.h"

#include <QDebug>
#include <QDir>
#include <QHash>
#include <QString>
#include <QStringList>

int main(int argc, char *argv[])
{
    static const QString AppName = "TeX Creator";
    QString home = QDir::home().dirName();
    ApplicationServer s(9950 + qHash(home) % 10, AppName + "3" + home);
    int ret = 0;
    QStringList args;
    foreach (int i, bRangeD(1, argc - 1))
        args << argv[i];
    if (!s.testServer()) {
        s.listen();
        Application app(argc, argv, AppName, "Andrey Bogdanov");
        ret = app.exec();
    } else {
        if (args.isEmpty())
            args << "";
        s.sendMessage(args);
    }
    return ret;
}
