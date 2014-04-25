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

#include "samplesproxymodel.h"
#include "samplesmodel.h"
#include "client.h"
#include "application.h"
#include "texsamplesettingstab.h"

#include <TSampleInfo>
#include <BTextTools>

#include <BeQtGlobal>

#include <QSortFilterProxyModel>
#include <QVariant>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QRegExp>

/*============================================================================
================================ SamplesProxyModel ===========================
============================================================================*/

/*============================== Public constructors =======================*/

SamplesProxyModel::SamplesProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    msamplesModel = 0;
    msampleType = TSampleInfo::Approved;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    connect( this, SIGNAL( sourceModelChanged() ), SLOT( sourceModelChangedSlot() ) );
#endif
    connect( sClient, SIGNAL( loginChanged(QString) ), this, SLOT( invalidate() ) );
}

/*============================== Public methods ============================*/

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
void SamplesProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
    sourceModelChangedSlot();
}
#endif

void SamplesProxyModel::setSampleType(int type)
{
    if (type < CurrentUserSample || type > TSampleInfo::Rejected)
        return;
    msampleType = type;
    invalidate();
}

void SamplesProxyModel::setSearchKeywords(const QStringList &list)
{
    msearchKeywords = list;
    msearchKeywords.removeAll("");
    msearchKeywords.removeDuplicates();
    invalidate();
}

/*============================== Public slots ==============================*/

void SamplesProxyModel::setSearchKeywordsString(const QString &string)
{
    static QRegExp rx("\\,\\s*");
    setSearchKeywords( string.split(rx, QString::SkipEmptyParts) );
}

/*============================== Protected methods =========================*/

bool SamplesProxyModel::filterAcceptsColumn(int column, const QModelIndex &) const
{
    return msamplesModel && bRange(1, 2).contains(column);
}

bool SamplesProxyModel::filterAcceptsRow(int row, const QModelIndex &) const
{
    const TSampleInfo *s = msamplesModel ? msamplesModel->sample(row) : 0;
    return s && ((CurrentUserSample == msampleType && s->sender().login() == sClient->login())
                 || s->type() == msampleType) && matchesKeywords(*s);
}

/*============================== Private methods ===========================*/

bool SamplesProxyModel::matchesKeywords(const TSampleInfo &info) const
{
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    if (msearchKeywords.isEmpty())
        return true;
    return msearchKeywords.contains(info.idString(), cs) || msearchKeywords.contains(info.sender().login())
            || msearchKeywords.contains(info.sender().realName(), cs)
            || BTextTools::intersects(msearchKeywords, info.authors(), cs)
            || msearchKeywords.contains(info.title(), cs) || BTextTools::intersects(msearchKeywords, info.tags(), cs);
}

/*============================== Private slots =============================*/

void SamplesProxyModel::sourceModelChangedSlot()
{
    msamplesModel = static_cast<SamplesModel *>( sourceModel() );
}
