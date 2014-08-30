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

#ifndef GENERALSETTINGSTAB_H
#define GENERALSETTINGSTAB_H

class BLocaleComboBox;

class QCheckBox;
class QIcon;
class QString;

#include <BAbstractSettingsTab>

/*============================================================================
================================ GeneralSettingsTab ==========================
============================================================================*/

class GeneralSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
private:
    BLocaleComboBox *mlcmbox;
    QCheckBox *mcboxMultipleWindows;
    QCheckBox *mcboxNewVersions;
public:
    explicit GeneralSettingsTab();
public:
    QIcon icon() const;
    QString id() const;
    bool hasDefault() const;
    bool restoreDefault();
    bool saveSettings();
    QString title() const;
private:
    Q_DISABLE_COPY(GeneralSettingsTab)
};

#endif // GENERALSETTINGSTAB_H
