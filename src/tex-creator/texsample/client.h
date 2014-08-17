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

#ifndef CLIENT_H
#define CLIENT_H

#include <TNetworkClient>

#include <QObject>

/*============================================================================
================================ Client ======================================
============================================================================*/

class Client : public TNetworkClient
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    ~Client();
private:
    //static void showProgressDialog(BNetworkOperation *op, QWidget *parent = 0);
    //static QWidget *chooseParent(QWidget *supposed = 0);
    //static void showConnectionErrorMessage(const QString &errorString);
//private:
    //static const int ProgressDialogDelay;
    //static const int MaxSampleSize;
//private:
//    mutable QDateTime msamplesListUpdateDT;
private:
    Q_DISABLE_COPY(Client)
};

#endif // CLIENT_H
