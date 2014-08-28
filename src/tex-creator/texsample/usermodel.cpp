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

#include "usermodel.h"

#include "application.h"
#include "cache.h"

#include <TUserModel>

#include <QImage>
#include <QObject>

/*============================================================================
================================ UserModel ===================================
============================================================================*/

/*============================== Public constructors =======================*/

UserModel::UserModel(QObject *parent) :
    TUserModel(parent)
{
    //
}

/*============================== Protected methods =========================*/

bool UserModel::avatarStoredSeparately() const
{
    //TODO: Change to true
    return false;
}

QImage UserModel::loadAvatar(quint64 userId) const
{
    //TODO
}

void UserModel::removeAvatar(quint64 userId)
{
    //TODO
}

void UserModel::saveAvatar(quint64 userId, const QImage &avatar)
{
    //TODO
}
