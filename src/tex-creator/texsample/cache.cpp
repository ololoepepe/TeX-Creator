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

#include "cache.h"

#include <TeXSample/TeXSampleCore>

#include <BDirTools>
#include <BeQt>
#include <BSqlDatabase>
#include <BSqlQuery>
#include <BSqlResult>
#include <BSqlWhere>
#include <BUuid>

#include <QDateTime>
#include <QDebug>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QVariantMap>

/*============================================================================
================================ Cache =======================================
============================================================================*/

/*============================== Public constructors =======================*/

Cache::Cache(const QString &location) : Location(location)
{
    menabled = true;
    QString schemaFileName = BDirTools::findResource("texsample/db/cache.schema", BDirTools::GlobalOnly);
    mdb = new BSqlDatabase("QSQLITE", BUuid::createUuid().toString(true));
    mdb->setDatabaseName(location + "/cache.sqlite");
    mdb->setOnOpenQuery("PRAGMA foreign_keys = ON");
    if (!mdb->open() || !mdb->initializeFromSchemaFile(schemaFileName, "UTF-8")) {
        delete mdb;
        mdb = 0;
    }
}

Cache::~Cache()
{
    delete mdb;
}

/*============================== Public methods ============================*/

void Cache::clear()
{
    if (!mdb)
        return;
    if (!mdb->transaction())
        return;
    if (!mdb->deleteFrom("users") || !mdb->deleteFrom("invite_codes") || !mdb->deleteFrom("groups")
            || !mdb->deleteFrom("samples") || !mdb->deleteFrom("sample_previews") || !mdb->deleteFrom("sample_sources")
            || !mdb->deleteFrom("last_request_date_times") || !mdb->exec("VACUUM")) {
        bRet(mdb->rollback());
    }
    mdb->commit();
}

QVariant Cache::data(const QString &operation, const QVariant &id) const
{
    typedef QMap<QString, GetDataFunction> GetDataFunctionMap;
    init_once(GetDataFunctionMap, functionMap, GetDataFunctionMap()) {
        functionMap.insert(TOperation::GetSamplePreview, &Cache::getSamplePreview);
        functionMap.insert(TOperation::GetSampleSource, &Cache::getSampleSource);
    }
    if (!menabled || !mdb)
        return QVariant();
    GetDataFunction f = functionMap.value(operation);
    if (!f)
        return QVariant();
    return (getSelf()->*f)(id);
}

TGroupInfoList Cache::groupInfoList() const
{
    TGroupInfoList list;
    if (!menabled || !mdb)
        return list;
    BSqlResult result = getSelf()->mdb->select("groups", "info");
    if (!result.success())
        return list;
    foreach (const QVariantMap &m, result.values())
        list << BeQt::deserialize(m.value("info").toByteArray()).value<TGroupInfo>();
    return list;
}

TInviteInfoList Cache::inviteInfoList() const
{
    TInviteInfoList list;
    if (!menabled || !mdb)
        return list;
    BSqlResult result = getSelf()->mdb->select("invite_codes", "info");
    if (!result.success())
        return list;
    foreach (const QVariantMap &m, result.values())
        list << BeQt::deserialize(m.value("info").toByteArray()).value<TInviteInfo>();
    return list;
}

bool Cache::isEnabled() const
{
    return menabled;
}

QDateTime Cache::lastRequestDateTime(RequestType type, const quint64 id) const
{
    if (!menabled || !mdb)
        return QDateTime();
    if (!bRangeD(GroupListRequest, UserListRequest).contains(type))
        return QDateTime();
    QDateTime dt;
    dt.setTimeSpec(Qt::UTC);
    if (bRangeD(SamplePreviewRequest, SampleSourceRequest).contains(type)) {
        if (!id)
            return QDateTime();
        QString table = (SamplePreviewRequest == type) ? "sample_previews" : "sample_sources";
        BSqlResult result = mdb->select(table, "last_request_date_time", BSqlWhere("id = :id", ":id", id));
        if (!result.success() || result.values().isEmpty())
            return QDateTime();
        dt.setMSecsSinceEpoch(result.value("last_request_date_time").toLongLong());
        return dt;
    } else {
        BSqlWhere where("request_type = :request_type", ":request_type", int(type));
        BSqlResult result = mdb->select("last_request_date_times", "date_time", where);
        if (!result.success() || result.values().isEmpty())
            return QDateTime();
        dt.setMSecsSinceEpoch(result.value("date_time").toLongLong());
        return dt;
    }
}

void Cache::removeData(const QString &operation, const QVariant &id)
{
    typedef QMap<QString, RemoveDataFunction> RemoveDataFunctionMap;
    init_once(RemoveDataFunctionMap, functionMap, RemoveDataFunctionMap()) {
        functionMap.insert(TOperation::DeleteGroup, &Cache::handleDeleteGroup);
        functionMap.insert(TOperation::DeleteInvites, &Cache::handleDeleteInvites);
        functionMap.insert(TOperation::DeleteSample, &Cache::handleDeleteSample);
        functionMap.insert(TOperation::DeleteUser, &Cache::handleDeleteUser);
    }
    if (!menabled || !mdb)
        return;
    RemoveDataFunction f = functionMap.value(operation);
    if (!f)
        return;
    if (!mdb->transaction())
        return;
    if (!(this->*f)(id))
        return bRet(mdb->rollback());
    mdb->commit();
}

TSampleInfoList Cache::sampleInfoList() const
{
    TSampleInfoList list;
    if (!menabled || !mdb)
        return list;
    BSqlResult result = getSelf()->mdb->select("samples", "info");
    if (!result.success())
        return list;
    foreach (const QVariantMap &m, result.values())
        list << BeQt::deserialize(m.value("info").toByteArray()).value<TSampleInfo>();
    return list;
}

void Cache::setData(const QString &operation, const QDateTime &requestDateTime, const QVariant &data,
                    const QVariant &id)
{
    typedef QMap<QString, SetDataFunction> SetDataFunctionMap;
    init_once(SetDataFunctionMap, functionMap, SetDataFunctionMap()) {
        functionMap.insert(TOperation::AddGroup, &Cache::handleAddGroup);
        functionMap.insert(TOperation::AddSample, &Cache::handleAddSample);
        functionMap.insert(TOperation::AddUser, &Cache::handleAddUser);
        functionMap.insert(TOperation::GenerateInvites, &Cache::handleGenerateInvites);
        functionMap.insert(TOperation::EditGroup, &Cache::handleEditGroup);
        functionMap.insert(TOperation::EditSample, &Cache::handleEditSample);
        functionMap.insert(TOperation::EditSampleAdmin, &Cache::handleEditSampleAdmin);
        functionMap.insert(TOperation::EditSelf, &Cache::handleEditSelf);
        functionMap.insert(TOperation::EditUser, &Cache::handleEditUser);
        functionMap.insert(TOperation::GetGroupInfoList, &Cache::handleGetGroupInfoList);
        functionMap.insert(TOperation::GetInviteInfoList, &Cache::handleGetInviteInfoList);
        functionMap.insert(TOperation::GetSampleInfoList, &Cache::handleGetSampleInfoList);
        functionMap.insert(TOperation::GetSamplePreview, &Cache::handleGetSamplePreview);
        functionMap.insert(TOperation::GetSampleSource, &Cache::handleGetSampleSource);
        functionMap.insert(TOperation::GetSelfInfo, &Cache::handleGetSelfInfo);
        functionMap.insert(TOperation::GetUserInfo, &Cache::handleGetUserInfo);
        functionMap.insert(TOperation::GetUserInfoAdmin, &Cache::handleGetUserInfoAdmin);
        functionMap.insert(TOperation::GetUserInfoListAdmin, &Cache::handleGetUserInfoListAdmin);
    }
    if (!menabled || !mdb)
        return;
    SetDataFunction f = functionMap.value(operation);
    if (!f)
        return;
    if (!mdb->transaction())
        return;
    if (!(this->*f)(requestDateTime, data, id))
        return bRet(mdb->rollback());
}

void Cache::setEnabled(bool enabled)
{
    menabled = enabled;
}

TUserInfoList Cache::userInfoList() const
{
    TUserInfoList list;
    if (!menabled || !mdb)
        return list;
    BSqlResult result = getSelf()->mdb->select("users", "info");
    if (!result.success())
        return list;
    foreach (const QVariantMap &m, result.values())
        list << BeQt::deserialize(m.value("info").toByteArray()).value<TUserInfo>();
    return list;
}

/*============================== Private methods ===========================*/

QVariant Cache::getSamplePreview(const QVariant &id)
{
    BSqlResult result = mdb->select("sample_previews", "data", BSqlWhere("id = :id", ":id", id.toULongLong()));
    if (!result.success() || result.values().isEmpty())
        return QVariant();
    return BeQt::deserialize(result.value("data").toByteArray());
}

QVariant Cache::getSampleSource(const QVariant &id)
{
    BSqlResult result = mdb->select("sample_sources", "data", BSqlWhere("id = :id", ":id", id.toULongLong()));
    if (!result.success() || result.values().isEmpty())
        return QVariant();
    return BeQt::deserialize(result.value("data").toByteArray());
}

Cache *Cache::getSelf() const
{
    return const_cast<Cache *>(this);
}

bool Cache::handleAddGroup(const QDateTime &, const QVariant &v, const QVariant &)
{
    TAddGroupReplyData data = v.value<TAddGroupReplyData>();
    TGroupInfo info = data.groupInfo();
    if (!mdb->insert("groups", "id", info.id(), "info", BeQt::serialize(info)).success())
        return false;
    return true;
}

bool Cache::handleAddSample(const QDateTime &, const QVariant &v, const QVariant &)
{
    TAddSampleReplyData data = v.value<TAddSampleReplyData>();
    TSampleInfo info = data.sampleInfo();
    if (!mdb->insert("samples", "id", info.id(), "info", BeQt::serialize(info)).success())
        return false;
    return true;
}

bool Cache::handleAddUser(const QDateTime &, const QVariant &v, const QVariant &)
{
    TAddUserReplyData data = v.value<TAddUserReplyData>();
    TUserInfo info = data.userInfo();
    if (!mdb->insert("users", "id", info.id(), "info", BeQt::serialize(info)).success())
        return false;
    return true;
}

bool Cache::handleDeleteGroup(const QVariant &v)
{
    quint64 groupId = v.toULongLong();
    if (!mdb->deleteFrom("groups", BSqlWhere("id = :id", ":id", groupId)).success())
        return false;
    return true;
}

bool Cache::handleDeleteInvites(const QVariant &v)
{
    foreach (quint64 id, v.value<TIdList>()) {
        if (mdb->deleteFrom("invite_codes", BSqlWhere("id = :id", ":id", id)))
            return false;
    }
    return true;
}

bool Cache::handleDeleteSample(const QVariant &v)
{
    return mdb->deleteFrom("samples", BSqlWhere("id = :id", ":id", v.toULongLong()))
            && mdb->deleteFrom("sample_previews", BSqlWhere("id = :id", ":id", v.toULongLong()))
            && mdb->deleteFrom("sample_sources", BSqlWhere("id = :id", ":id", v.toULongLong()));
}

bool Cache::handleDeleteUser(const QVariant &v)
{
    return mdb->deleteFrom("users", BSqlWhere("id = :id", ":id", v.toULongLong()));
}

bool Cache::handleEditGroup(const QDateTime &, const QVariant &v, const QVariant &)
{
    TEditGroupReplyData data = v.value<TEditGroupReplyData>();
    TGroupInfo info = data.groupInfo();
    if (!mdb->update("groups", "info", BeQt::serialize(info), BSqlWhere("id = :id", ":id", info.id())).success())
        return false;
    return true;
}

bool Cache::handleEditSample(const QDateTime &, const QVariant &v, const QVariant &)
{
    TEditSampleReplyData data = v.value<TEditSampleReplyData>();
    TSampleInfo info = data.sampleInfo();
    BSqlWhere where("id = :id", ":id", info.id());
    if (!mdb->update("samples", "info", BeQt::serialize(info), where).success())
        return false;
    if (!mdb->deleteFrom("sample_previews", where) || !mdb->deleteFrom("sample_sources", where))
        return false;
    return true;
}

bool Cache::handleEditSampleAdmin(const QDateTime &, const QVariant &v, const QVariant &)
{
    TEditSampleAdminReplyData data = v.value<TEditSampleAdminReplyData>();
    TSampleInfo info = data.sampleInfo();
    BSqlWhere where("id = :id", ":id", info.id());
    if (!mdb->update("samples", "info", BeQt::serialize(info), where).success())
        return false;
    if (!mdb->deleteFrom("sample_previews", where) || !mdb->deleteFrom("sample_sources", where))
        return false;
    return true;
}

bool Cache::handleEditSelf(const QDateTime &, const QVariant &v, const QVariant &)
{
    TEditSelfReplyData data = v.value<TEditSelfReplyData>();
    TUserInfo info = data.userInfo();
    if (!mdb->update("users", "info", BeQt::serialize(info), BSqlWhere("id = :id", ":id", info.id())).success())
        return false;
    return true;
}

bool Cache::handleEditUser(const QDateTime &, const QVariant &v, const QVariant &)
{
    TEditUserReplyData data = v.value<TEditUserReplyData>();
    TUserInfo info = data.userInfo();
    if (!mdb->update("users", "info", BeQt::serialize(info), BSqlWhere("id = :id", ":id", info.id())).success())
        return false;
    return true;
}

bool Cache::handleGenerateInvites(const QDateTime &, const QVariant &v, const QVariant &)
{
    TGenerateInvitesReplyData data = v.value<TGenerateInvitesReplyData>();
    foreach (const TInviteInfo &info, data.generatedInvites()) {
        if (!mdb->insert("invite_codes", "id", info.id(), "info", BeQt::serialize(info)).success())
            return false;
    }
    return true;
}

bool Cache::handleGetGroupInfoList(const QDateTime &dt, const QVariant &v, const QVariant &)
{
    TGetGroupInfoListReplyData data = v.value<TGetGroupInfoListReplyData>();
    QVariantMap values;
    values.insert("request_type", int(GroupListRequest));
    values.insert("date_time", dt.toUTC().toMSecsSinceEpoch());
    if (!mdb->insertOrReplace("last_request_date_times", values).success())
        return false;
    foreach (quint64 groupId, data.deletedGroups()) {
        if (!mdb->deleteFrom("groups", BSqlWhere("id = :id", ":id", groupId)))
            return false;
    }
    foreach (const TGroupInfo &info, data.newGroups()) {
        if (!mdb->insertOrReplace("groups", "id", info.id(), "info", BeQt::serialize(info)))
            return false;
    }
    return true;
}

bool Cache::handleGetInviteInfoList(const QDateTime &dt, const QVariant &v, const QVariant &)
{
    TGetInviteInfoListReplyData data = v.value<TGetInviteInfoListReplyData>();
    QVariantMap values;
    values.insert("request_type", int(InviteListRequest));
    values.insert("date_time", dt.toUTC().toMSecsSinceEpoch());
    if (!mdb->insertOrReplace("last_request_date_times", values).success())
        return false;
    foreach (quint64 inviteId, data.deletedInvites()) {
        if (!mdb->deleteFrom("invite_codes", BSqlWhere("id = :id", ":id", inviteId)))
            return false;
    }
    foreach (const TInviteInfo &info, data.newInvites()) {
        if (!mdb->insertOrReplace("invite_codes", "id", info.id(), "info", BeQt::serialize(info)))
            return false;
    }
    return true;
}

bool Cache::handleGetSampleInfoList(const QDateTime &dt, const QVariant &v, const QVariant &)
{
    TGetSampleInfoListReplyData data = v.value<TGetSampleInfoListReplyData>();
    QVariantMap values;
    values.insert("request_type", int(SampleListRequest));
    values.insert("date_time", dt.toUTC().toMSecsSinceEpoch());
    if (!mdb->insertOrReplace("last_request_date_times", values).success())
        return false;
    foreach (quint64 sampleId, data.deletedSamples()) {
        if (!mdb->deleteFrom("samples", BSqlWhere("id = :id", ":id", sampleId)))
            return false;
    }
    foreach (const TSampleInfo &info, data.newSamples()) {
        if (!mdb->insertOrReplace("samples", "id", info.id(), "info", BeQt::serialize(info)))
            return false;
    }
    return true;
}

bool Cache::handleGetSamplePreview(const QDateTime &dt, const QVariant &v, const QVariant &id)
{
    QVariantMap values;
    values.insert("id", id.toULongLong());
    values.insert("data", BeQt::serialize(v));
    values.insert("last_request_date_time", dt.toUTC().toMSecsSinceEpoch());
    if (!mdb->insertOrReplace("sample_previews", values))
        return false;
    return true;
}

bool Cache::handleGetSampleSource(const QDateTime &dt, const QVariant &v, const QVariant &id)
{
    QVariantMap values;
    values.insert("id", id.toULongLong());
    values.insert("data", BeQt::serialize(v));
    values.insert("last_request_date_time", dt.toUTC().toMSecsSinceEpoch());
    if (!mdb->insertOrReplace("sample_sources", values))
        return false;
    return true;
}

bool Cache::handleGetUserInfo(const QDateTime &, const QVariant &v, const QVariant &)
{
    TGetUserInfoReplyData data = v.value<TGetUserInfoReplyData>();
    TUserInfo info = data.userInfo();
    if (!mdb->insertOrReplace("users", "id", info.id(), "info", BeQt::serialize(info)).success())
        return false;
    return true;
}

bool Cache::handleGetSelfInfo(const QDateTime &, const QVariant &v, const QVariant &)
{
    TGetSelfInfoReplyData data = v.value<TGetSelfInfoReplyData>();
    TUserInfo info = data.userInfo();
    if (!mdb->insertOrReplace("users", "id", info.id(), "info", BeQt::serialize(info)).success())
        return false;
    return true;
}

bool Cache::handleGetUserInfoAdmin(const QDateTime &, const QVariant &v, const QVariant &)
{
    TGetUserInfoAdminReplyData data = v.value<TGetUserInfoAdminReplyData>();
    TUserInfo info = data.userInfo();
    if (!mdb->insertOrReplace("users", "id", info.id(), "info", BeQt::serialize(info)).success())
        return false;
    return true;
}

bool Cache::handleGetUserInfoListAdmin(const QDateTime &dt, const QVariant &v, const QVariant &)
{
    TGetUserInfoListAdminReplyData data = v.value<TGetUserInfoListAdminReplyData>();
    QVariantMap values;
    values.insert("request_type", int(UserListRequest));
    values.insert("date_time", dt.toUTC().toMSecsSinceEpoch());
    if (!mdb->insertOrReplace("last_request_date_times", values).success())
        return false;
    foreach (quint64 userId, data.deletedUsers()) {
        if (!mdb->deleteFrom("users", BSqlWhere("id = :id", ":id", userId)))
            return false;
    }
    foreach (const TUserInfo &info, data.newUsers()) {
        if (!mdb->insertOrReplace("users", "id", info.id(), "info", BeQt::serialize(info)))
            return false;
    }
    return true;
}
