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

#ifndef SAMPLEPROXYMODEL_H
#define SAMPLEPROXYMODEL_H

class SampleModel;

class TSampleInfo;

class QModelIndex;

#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>

/*============================================================================
================================ SampleProxyModel ============================
============================================================================*/

class SampleProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum ExtraSampleType
    {
        CurrentUserSample = -1
    };
private:
    QString mcurrentUserLogin;
    SampleModel *msampleModel;
    int msampleType;
    QStringList msearchKeywords;
public:
    explicit SampleProxyModel(SampleModel *sourceModel, QObject *parent = 0);
public slots:
    void setCurrentUserLogin(const QString &login);
    void setSampleType(int type);
    void setSearchKeywords(const QStringList &list);
    void setSearchKeywordsString(const QString &string);
protected:
    bool filterAcceptsColumn(int column, const QModelIndex &parent) const;
    bool filterAcceptsRow(int row, const QModelIndex &parent) const;
private:
    bool matchesKeywords(const TSampleInfo &info) const;
private:
    Q_DISABLE_COPY(SampleProxyModel)
};

#endif // SAMPLEPROXYMODEL_H
