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

#include <TUserModel>

#include <BDirTools>
#include <BeQt>
#include <BSqlDatabase>
#include <BSqlQuery>
#include <BSqlResult>
#include <BSqlWhere>
#include <BUuid>

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

/*============================================================================
================================ UserModel ===================================
============================================================================*/

/*============================== Public constructors =======================*/

UserModel::UserModel(const QString &location, QObject *parent) :
    TUserModel(parent), Location(location)
{
    QString schemaFileName = BDirTools::findResource("texsample/db/temporary.schema", BDirTools::GlobalOnly);
    mdb = new BSqlDatabase("QSQLITE", BUuid::createUuid().toString(true));
    mdb->setDatabaseName(location + "/temporary.sqlite");
    mdb->setOnOpenQuery("PRAGMA foreign_keys = ON");
    if (!mdb->open() || !mdb->initializeFromSchemaFile(schemaFileName, "UTF-8")) {
        delete mdb;
        mdb = 0;
    }
}

UserModel::~UserModel()
{
    delete mdb;
    QFile::remove(Location + "/temporary.sqlite");
}

/*============================== Protected methods =========================*/

bool UserModel::avatarStoredSeparately() const
{
    return true;
}

QImage UserModel::loadAvatar(quint64 userId) const
{
    if (!mdb || !userId)
        return QImage();
    BSqlResult result = mdb->select("user_avatars", "avatar", BSqlWhere("user_id = :user_id", ":user_id", userId));
    if (!result.success())
        return QImage();
    return BeQt::deserialize(result.value("avatar").toByteArray()).value<QImage>();
}

void UserModel::removeAvatar(quint64 userId)
{
    if (!mdb || !userId)
        return;
    if (!mdb->transaction())
        return;
    if (!mdb->deleteFrom("user_avatars", BSqlWhere("user_id = :user_id", ":user_id", userId)).success())
        bRet(mdb->rollback());
    mdb->commit();
}

void UserModel::saveAvatar(quint64 userId, const QImage &avatar)
{
    if (!mdb || !userId)
        return;
    BSqlWhere where("user_id = :user_id", ":user_id", userId);
    BSqlResult result = mdb->select("user_avatars", "COUNT(*)", where);
    if (!result.success())
        return;
    if (!mdb->transaction())
        return;
    QByteArray data = BeQt::serialize(avatar);
    if (result.value("COUNT(*)").toInt() > 0) {
        if (!mdb->update("user_avatars", "user_id", userId, "avatar", data, where))
            bRet(mdb->rollback());
    } else {
        if (!mdb->insert("user_avatars", "user_id", userId, "avatar", data))
            bRet(mdb->rollback());
    }
    mdb->commit();
}
