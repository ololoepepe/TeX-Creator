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

#include "samplemodel.h"

#include <TAuthorInfoList>
#include <TFileInfo>
#include <TFileInfoList>
#include <TSampleInfo>
#include <TSampleInfoList>
#include <TSampleType>

#include <QAbstractTableModel>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QVariant>

/*============================================================================
================================ SampleModel =================================
============================================================================*/

/*============================== Public constructors =======================*/

SampleModel::SampleModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    //
}

/*============================== Public methods ============================*/

void SampleModel::addSample(const TSampleInfo &sample)
{
    TSampleInfoList list;
    list << sample;
    addSamples(list);
}

void SampleModel::addSamples(const TSampleInfoList &sampleList)
{
    TSampleInfoList list = sampleList;
    foreach (int i, bRangeR(list.size() - 1, 0)) {
        const TSampleInfo &info = list.at(i);
        if (!info.isValid())
            list.removeAt(i);
        else if (map.contains(info.id()))
            removeSample(info.id());
    }
    if (list.isEmpty())
        return;
    int ind = samples.size();
    beginInsertRows(QModelIndex(), ind, ind + list.size() - 1);
    foreach (TSampleInfo info, list) {
        samples.append(info);
        map.insert(info.id(), &samples.last());
    }
    endInsertRows();
}

int SampleModel::columnCount(const QModelIndex &) const
{
    return 15;
}

QVariant SampleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() > 14 || Qt::DisplayRole != role)
        return QVariant();
    TSampleInfo info = sampleInfoAt(index.row());
    if (!info.isValid())
        return QVariant();
    switch (index.column()) {
    case 0:
        return info.id();
    case 1:
        return info.title();
    case 2:
        return info.type();
    case 3:
        return info.senderId();
    case 4:
        return info.senderLogin();
    case 5:
        return info.description();
    case 6:
        return info.rating();
    case 7:
        return info.authors();
    case 8:
        return info.tags();
    case 9:
        return info.adminRemark();
    case 10:
        return info.mainSourceFile();
    case 11:
        return info.mainPreviewFile();
    case 12:
        return info.extraSourceFiles();
    case 13:
        return info.creationDateTime();
    case 14:
        return info.lastModificationDateTime();
    default:
        return QVariant();
    }
}

QVariant SampleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::Horizontal != orientation || Qt::DisplayRole != role)
        return QVariant();
    switch (section) {
    case 0:
        return tr("ID", "headerData");
    case 1:
        return tr("Title", "headerData");
    case 2:
        return tr("Type", "headerData");
    case 3:
        return tr("Sender ID", "headerData");
    case 4:
        return tr("Sender login", "headerData");
    case 5:
        return tr("Description", "headerData");
    case 6:
        return tr("Rating", "headerData");
    case 7:
        return tr("Authors", "headerData");
    case 8:
        return tr("Tags", "headerData");
    case 9:
        return tr("Admin remark", "headerData");
    case 10:
        return tr("Main source file", "headerData");
    case 11:
        return tr("Main preview file", "headerData");
    case 12:
        return tr("Extra source files", "headerData");
    case 13:
        return tr("Creation date", "headerData");
    case 14:
        return tr("Last modified", "headerData");
    default:
        return QVariant();
    }
}

void SampleModel::removeSample(quint64 id)
{
    if (!id || !map.contains(id))
        return;
    map.remove(id);
    int ind = indexOf(id);
    beginRemoveRows(QModelIndex(), ind, ind);
    samples.removeAt(ind);
    endRemoveRows();
}

void SampleModel::removeSamples(const TIdList &idList)
{
    foreach (quint64 id, idList)
        removeSample(id);
}

int SampleModel::rowCount(const QModelIndex &parent) const
{
    return samples.size();
}

quint64 SampleModel::sampleIdAt(int index) const
{
    if (index < 0 || index >= samples.size())
        return 0;
    return samples.at(index).id();
}

TSampleInfo SampleModel::sampleInfo(quint64 id) const
{
    const TSampleInfo *info = id ? map.value(id) : 0;
    if (!info)
        return TSampleInfo();
    return *info;
}

TSampleInfo SampleModel::sampleInfoAt(int index) const
{
    if (index < 0 || index >= samples.size())
        return TSampleInfo();
    return samples.at(index);
}

/*============================== Private methods ===========================*/

int SampleModel::indexOf(quint64 id) const
{
    if (!id)
        return -1;
    foreach (int i, bRangeD(0, samples.size() - 1)) {
        if (samples.at(i).id() == id)
            return i;
    }
    return -1;
}
