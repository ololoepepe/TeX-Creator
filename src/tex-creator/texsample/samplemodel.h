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

#ifndef SAMPLEMODEL_H
#define SAMPLEMODEL_H

class QVariant;

#include <TIdList>
#include <TSampleInfo>
#include <TSampleInfoList>

#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include <QModelIndex>

/*============================================================================
================================ SampleModel =================================
============================================================================*/

class SampleModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    QMap<quint64, TSampleInfo *> map;
    QList<TSampleInfo> samples;
public:
    explicit SampleModel(QObject *parent = 0);
public:
    void addSample(const TSampleInfo &sample);
    void addSamples(const TSampleInfoList &sampleList);
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void removeSample(quint64 id);
    void removeSamples(const TIdList &idList);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    quint64 sampleIdAt(int index) const;
    TSampleInfo sampleInfo(quint64 id) const;
    TSampleInfo sampleInfoAt(int index) const;
    void updateSample(quint64 sampleId, const TSampleInfo &newInfo);
private:
    int indexOf(quint64 id) const;
private:
    Q_DISABLE_COPY(SampleModel)
};

#endif // SAMPLEMODEL_H
