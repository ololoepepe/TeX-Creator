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

#ifndef NETWORKSETTINGSTAB_H
#define NETWORKSETTINGSTAB_H

class BLoginWidget;

class QButtonGroup;
class QIcon;
class QString;

#include <BAbstractSettingsTab>

#include <QObject>

/*============================================================================
================================ NetworkSettingsTab ==========================
============================================================================*/

class NetworkSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
private:
    QButtonGroup *btngr;
    BLoginWidget *lwgt;
public:
    explicit NetworkSettingsTab();
public:
    QIcon icon() const;
    QString id() const;
    bool hasDefault() const;
    bool restoreDefault();
    bool saveSettings();
    QString title() const;
private slots:
    void btnClicked(int index);
private:
    Q_DISABLE_COPY(NetworkSettingsTab)
};

#endif // NETWORKSETTINGSTAB_H
