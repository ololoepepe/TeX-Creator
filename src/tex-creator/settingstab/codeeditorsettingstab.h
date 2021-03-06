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

#ifndef CODEEDITORSETTINGSTAB_H
#define CODEEDITORSETTINGSTAB_H

class BTextCodecComboBox;

class QCheckBox;
class QComboBox;
class QFontComboBox;
class QIcon;
class QSpinBox;
class QString;

#include <BAbstractSettingsTab>

/*============================================================================
================================ CodeEditorSettingsTab =======================
============================================================================*/

class CodeEditorSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
private:
    QCheckBox *mcboxSimple;
    QFontComboBox *mfntcmbox;
    QSpinBox *msboxFontPointSize;
    QSpinBox *msboxLineLength;
    QComboBox *mcmboxTabWidth;
    QCheckBox *mcboxAutoCodecDetection;
    BTextCodecComboBox *mcmboxEncoding;
    QSpinBox *msboxMaxFileSize;
public:
    explicit CodeEditorSettingsTab();
public:
    QString id() const;
    QIcon icon() const;
    bool hasDefault() const;
    bool restoreDefault();
    bool saveSettings();
    QString title() const;
private:
    Q_DISABLE_COPY(CodeEditorSettingsTab)
};

#endif // CODEEDITORSETTINGSTAB_H
