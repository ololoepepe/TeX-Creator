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

#include "settings.h"

#include "application.h"

class QByteArray;
class QFont;
class QString;
class QStringList;
class QTextCodec;

#include <BCodeEditor>
#include <BeQt>
#include <BGuiTools>

#include <QByteArray>
#include <QFont>
#include <QList>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QVariant>

namespace Settings
{

namespace CodeEditor
{

static const QString RootPath = "CodeEditor";
static const QString AutoCodecDetectionEnabledPath = RootPath + "/auto_codec_detection_enabled";
static const QString DefaultCodecNamePath = RootPath + "/default_codec_name";
static const QString DocumentDriverStatePath = RootPath + "/document_driver_state";
static const QString DocumentTypePath = RootPath + "/document_type";
static const QString EditFontPath = RootPath + "/edit_font";
static const QString EditLineLengthPath = RootPath + "/edit_line_length";
static const QString EditTabWidthPath = RootPath + "/edit_tab_width";
static const QString FileHistoryPath = RootPath + "/file_history";
static const QString MaximumFileSizePath = RootPath + "/maximum_file_size";
static const QString SearchModuleStatePath = RootPath + "/search_module_state";
static const QString SpellCheckEnabledPath = RootPath + "/spell_check_enabled";

bool autoCodecDetectionEnabled()
{
    return bSettings->value(AutoCodecDetectionEnabledPath, true).toBool();
}

QTextCodec *defaultCodec()
{
    return BeQt::codec(bSettings->value(DefaultCodecNamePath, QTextCodec::codecForLocale()->name()).toByteArray());
}

QString defaultCodecName()
{
    return BeQt::codecName(defaultCodec());
}

QByteArray documentDriverState()
{
    return bSettings->value(DocumentDriverStatePath).toByteArray();
}

int documentType()
{
    return bSettings->value(DocumentTypePath, int(BCodeEditor::StandardDocument)).toInt();
}

QFont editFont()
{
    return bSettings->value(EditFontPath, BGuiTools::createMonospaceFont()).value<QFont>();
}

QString editFontFamily()
{
    return editFont().family();
}

int editFontPointSize()
{
    return editFont().pointSize();
}

int editLineLength()
{
    return bSettings->value(EditLineLengthPath, 120).toInt();
}

BeQt::TabWidth editTabWidth()
{
    return enum_cast<BeQt::TabWidth>(bSettings->value(EditTabWidthPath, BeQt::TabWidth4), BeQt::allTabWidths(),
                                     BeQt::TabWidth4);
}

QStringList fileHistory()
{
    return bSettings->value(FileHistoryPath).toStringList();
}

int maximumFileSize()
{
    return bSettings->value(MaximumFileSizePath, 2 * BeQt::Megabyte).toInt();
}

QByteArray searchModuleState()
{
    return bSettings->value(SearchModuleStatePath).toByteArray();
}

void setAutoCodecDetectionEnabled(bool b)
{
    bSettings->setValue(AutoCodecDetectionEnabledPath, b);
}

void setDefaultCodec(QTextCodec *codec)
{
    bSettings->setValue(DefaultCodecNamePath, BeQt::codecName(codec));
}

void setDefaultCodec(const QString &codecName)
{
    setDefaultCodec(BeQt::codec(codecName));
}

void setDocumentDriverState(const QByteArray &state)
{
    bSettings->setValue(DocumentDriverStatePath, state);
}

void setDocumentType(int t)
{
    bSettings->setValue(DocumentTypePath, t);
}

void setEditFont(const QFont &font)
{
    bSettings->setValue(EditFontPath, font);
}

void setEditFontFamily(const QString &family)
{
    QFont font = editFont();
    font.setFamily(family);
    setEditFont(font);
}

void setEditFontPointSize(int pointSize)
{
    QFont font = editFont();
    font.setPointSize(pointSize);
    setEditFont(font);
}

void setEditLineLength(int lineLength)
{
    bSettings->setValue(EditLineLengthPath, lineLength);
}

void setEditTabWidth(BeQt::TabWidth tabWidth)
{
    bSettings->setValue(EditTabWidthPath, int(tabWidth));
}

void setFileHistory(const QStringList &history)
{
    bSettings->setValue(FileHistoryPath, history);
}

void setMaximumFileSize(int sz)
{
    bSettings->setValue(MaximumFileSizePath, sz);
}

void setSearchModuleState(const QByteArray &state)
{
    bSettings->setValue(SearchModuleStatePath, state);
}

void setSpellCheckEnabled(bool b)
{
    bSettings->setValue(SpellCheckEnabledPath, b);
}

bool spellCheckEnabled()
{
    return bSettings->value(SpellCheckEnabledPath, true).toBool();
}

}

}
