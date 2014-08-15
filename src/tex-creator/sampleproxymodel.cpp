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

#include "sampleproxymodel.h"

#include "samplemodel.h"

#include <TAuthorInfo>
#include <TAuthorInfoList>
#include <TSampleType>

#include <BTextTools>

#include <QList>
#include <QModelIndex>
#include <QRegExp>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QVariant>

/*============================================================================
================================ SampleProxyModel ============================
============================================================================*/

/*============================== Public constructors =======================*/

SampleProxyModel::SampleProxyModel(SampleModel *sourceModel, QObject *parent) :
    QSortFilterProxyModel(parent)
{
    msampleModel = sourceModel;
    msampleType = TSampleType::Approved;
    setSourceModel(sourceModel);
}

/*============================== Public slots ==============================*/

void SampleProxyModel::setCurrentUserLogin(const QString &login)
{
    if (login == mcurrentUserLogin)
        return;
    mcurrentUserLogin = login;
    if (CurrentUserSample == msampleType)
        invalidate();
}

void SampleProxyModel::setSampleType(int type)
{
    if (type == msampleType)
        return;
    static const QList<int> Types = QList<int>() << CurrentUserSample << TSampleType::Unverified
                                                 << TSampleType::Approved << TSampleType::Rejected;
    if (!Types.contains(type))
        return;
    msampleType = type;
    invalidate();
}

void SampleProxyModel::setSearchKeywords(const QStringList &list)
{
    QStringList nlist = list;
    nlist.removeAll("");
    nlist.removeDuplicates();
    if (nlist == msearchKeywords)
        return;
    msearchKeywords = nlist;
    invalidate();
}

void SampleProxyModel::setSearchKeywordsString(const QString &string)
{
    QRegExp rx("\\,\\s*");
    setSearchKeywords(string.split(rx, QString::SkipEmptyParts));
}

/*============================== Protected methods =========================*/

bool SampleProxyModel::filterAcceptsColumn(int column, const QModelIndex &) const
{
    static const QList<int> Columns = QList<int>() << 0;
    return msampleModel && Columns.contains(column);
}

bool SampleProxyModel::filterAcceptsRow(int row, const QModelIndex &) const
{
    if (!msampleModel)
        return false;
    TSampleInfo info = msampleModel->sampleInfoAt(row);
    if (!matchesKeywords(info))
        return false;
    return (CurrentUserSample == msampleType) ? (info.senderLogin() == mcurrentUserLogin) :
                                                (int(info.type()) == msampleType);
}

/*============================== Private methods ===========================*/

bool SampleProxyModel::matchesKeywords(const TSampleInfo &info) const
{
    static const Qt::CaseSensitivity Cs = Qt::CaseInsensitive;
    if (msearchKeywords.isEmpty())
        return true;
    if (msearchKeywords.contains(QString::number(info.id()), Cs) || msearchKeywords.contains(info.senderLogin()))
       return true;
    foreach (const TAuthorInfo &author, info.authors()) {
        if (msearchKeywords.contains(author.name(), Cs) || msearchKeywords.contains(author.surname(), Cs)
                || msearchKeywords.contains(author.patronymic(), Cs)) {
            return true;
        }
    }
    if (msearchKeywords.contains(info.title(), Cs) || BTextTools::intersects(msearchKeywords, info.tags(), Cs))
        return true;
    return false;
}
