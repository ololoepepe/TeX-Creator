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

#ifndef SAMPLESPROXYMODEL_H
#define SAMPLESPROXYMODEL_H

class SamplesModel;

class TSampleInfo;

class QVariant;
class QModelIndex;
class QString;

#include <QSortFilterProxyModel>
#include <QStringList>

/*============================================================================
================================ SamplesProxyModel ===========================
============================================================================*/

class SamplesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum ExtraSampleType
    {
        CurrentUserSample = -1
    };
public:
    explicit SamplesProxyModel(QObject *parent = 0);
public:
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    void setSourceModel(QAbstractItemModel *sourceModel);
#endif
    void setSampleType(int type);
    void setSearchKeywords(const QStringList &list);
public slots:
    void setSearchKeywordsString(const QString &string);
protected:
    bool filterAcceptsColumn(int column, const QModelIndex &parent) const;
    bool filterAcceptsRow(int row, const QModelIndex &parent) const;
private:
    bool matchesKeywords(const TSampleInfo &info) const;
private slots:
    void sourceModelChangedSlot();
private:
    SamplesModel *msamplesModel;
    int msampleType;
    QStringList msearchKeywords;
private:
    Q_DISABLE_COPY(SamplesProxyModel)
};

#endif // SAMPLESPROXYMODEL_H
