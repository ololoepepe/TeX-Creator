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

#ifndef CONSOLESETTINGSTAB_H
#define CONSOLESETTINGSTAB_H

class QCheckBox;
class QComboBox;
class QIcon;
class QLineEdit;
class QString;

#include <BAbstractSettingsTab>

/*============================================================================
================================ ConsoleSettingsTab ==========================
============================================================================*/

class ConsoleSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit ConsoleSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool hasDefault() const;
    bool restoreDefault();
    bool saveSettings();
private:
    QComboBox *mcmboxCompiler;
    QLineEdit *mledtOptions;
    QLineEdit *mledtCommands;
    QCheckBox *mcboxMakeindex;
    QCheckBox *mcboxDvips;
    QCheckBox *mcboxRemoteCompiler;
    QCheckBox *mcboxFallbackToLocalCompiler;
    QCheckBox *mcboxAlwaysLatin;
private:
    Q_DISABLE_COPY(ConsoleSettingsTab)
};

#endif // CONSOLESETTINGSTAB_H
