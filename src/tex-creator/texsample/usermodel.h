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

#ifndef USERMODEL_H
#define USERMODEL_H

class QImage;
class QString;

class BSqlDatabase;

#include <TUserModel>

#include <QObject>

/*============================================================================
================================ UserModel ===================================
============================================================================*/

class UserModel : public TUserModel
{
    Q_OBJECT
private:
    const QString Location;
private:
    BSqlDatabase *mdb;
public:
    explicit UserModel(const QString &location, QObject *parent = 0);
    ~UserModel();
protected:
    bool avatarStoredSeparately() const;
    QImage loadAvatar(quint64 userId) const;
    void removeAvatar(quint64 userId);
    void saveAvatar(quint64 userId, const QImage &avatar);
private:
    Q_DISABLE_COPY(UserModel)
};

#endif // USERMODEL_H
