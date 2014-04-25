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

#ifndef SAMPLESMODEL_H
#define SAMPLESMODEL_H

class QStringList;
class QVariant;

#include <TSampleInfo>
#include <TSampleInfoList>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QString>
#include <QList>
#include <QMap>

#define sModel SamplesModel::instance()

/*============================================================================
================================ SamplesModel ================================
============================================================================*/

class SamplesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    static SamplesModel *instance();
public:
    explicit SamplesModel(QObject *parent = 0);
public:
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void insertSample(const TSampleInfo &s);
    void insertSamples(const TSampleInfoList &list);
    void removeSample(quint64 id);
    void removeSamples(const QList<quint64> &list);
    void clear();
    const TSampleInfo* sample(int index) const;
    const TSampleInfo* sample(quint64 id) const;
    const QList<TSampleInfo> *samples() const;
    quint64 indexAt(int row) const;
    bool isEmpty() const;
private slots:
    void retranslateUi();
private:
    static SamplesModel *minstance;
private:
    QList<TSampleInfo> msamples;
    QMap<quint64, TSampleInfo *> msampleMap;
private:
    Q_DISABLE_COPY(SamplesModel)
};

#endif // SAMPLESMODEL_H
