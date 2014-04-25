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

#include "samplesmodel.h"
#include "cache.h"

#include <TSampleInfo>

#include <BApplication>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QString>

#include <QDebug>

/*============================================================================
================================ SamplesModel ================================
============================================================================*/

/*============================== Static public methods =====================*/

SamplesModel *SamplesModel::instance()
{
    if (!minstance)
        minstance = new SamplesModel;
    return minstance;
}

/*============================== Public constructors =======================*/

SamplesModel::SamplesModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    insertSamples(sCache->sampleInfos());
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
}

/*============================== Public methods ============================*/

int SamplesModel::rowCount(const QModelIndex &) const
{
    return msamples.size();
}

int SamplesModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant SamplesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (Qt::DisplayRole != role && Qt::ToolTipRole != role))
        return QVariant();
    const TSampleInfo *s = sample( index.row() );
    if (!s)
        return QVariant();
    switch (index.column())
    {
    case 0:
        return s->id();
    case 1:
        return s->title();
    case 2:
        return (Qt::ToolTipRole == role && !s->sender().realName().isEmpty()) ? s->sender().realName() :
                                                                                s->sender().login();
    default:
        return QVariant();
    }
}

QVariant SamplesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::Horizontal != orientation || Qt::DisplayRole != role)
        return QVariant();
    switch (section)
    {
    case 1:
        return tr("Title", "headerData");
    case 2:
        return tr("Sender", "headerData");
    default:
        return QVariant();
    }
}

void SamplesModel::insertSample(const TSampleInfo &s)
{
    TSampleInfoList list;
    list << s;
    insertSamples(list);
}

void SamplesModel::insertSamples(const TSampleInfoList &list)
{
    TSampleInfoList nlist = list;
    foreach (int i, bRangeR(nlist.size() - 1, 0))
    {
        const TSampleInfo &s = nlist.at(i);
        if ( !s.isValid() )
            nlist.removeAt(i);
        else if ( msampleMap.contains( s.id() ) )
            removeSample( s.id() );
    }
    if (nlist.isEmpty())
        return;
    int ind = msamples.size();
    beginInsertRows(QModelIndex(), ind, ind + nlist.size() - 1);
    foreach (const TSampleInfo &s, nlist)
    {
        msamples.append(s);
        msampleMap.insert( s.id(), &msamples.last() );
    }
    endInsertRows();
}

void SamplesModel::removeSample(quint64 id)
{
    if (!id || !msampleMap.contains(id))
        return;
    TSampleInfo *s = msampleMap.take(id);
    int ind = msamples.indexOf(*s);
    beginRemoveRows(QModelIndex(), ind, ind);
    msamples.removeAt(ind);
    endRemoveRows();
}

void SamplesModel::removeSamples(const QList<quint64> &list)
{
    foreach (const quint64 &s, list)
        removeSample(s);
}

void SamplesModel::clear()
{
    if (msamples.isEmpty())
        return;
    beginRemoveRows(QModelIndex(), 0, msamples.size() - 1);
    msampleMap.clear();
    msamples.clear();
    endRemoveRows();
}

const TSampleInfo *SamplesModel::sample(int index) const
{
    return ( index >= 0 && index < msamples.size() ) ? &msamples.at(index) : 0;
}

const TSampleInfo *SamplesModel::sample(quint64 id) const
{
    return id ? msampleMap.value(id) : 0;
}

const QList<TSampleInfo> *SamplesModel::samples() const
{
    return &msamples;
}

quint64 SamplesModel::indexAt(int row) const
{
    const TSampleInfo *s = sample(row);
    return s ? s->id() : 0;
}

bool SamplesModel::isEmpty() const
{
    return msamples.isEmpty();
}

/*============================== Private slots =============================*/

void SamplesModel::retranslateUi()
{
    headerDataChanged(Qt::Horizontal, 1, 2);
}

/*============================== Static private members ====================*/

SamplesModel *SamplesModel::minstance = 0;
