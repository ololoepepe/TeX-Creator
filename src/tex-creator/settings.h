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

#ifndef SETTINGS_H
#define SETTINGS_H

class QByteArray;
class QFont;
class QString;
class QStringList;
class QTextCodec;

#include <BeQt>

namespace Settings
{

namespace CodeEditor
{

bool autoCodecDetectionEnabled();
QTextCodec *defaultCodec();
QString defaultCodecName();
QByteArray documentDriverState();
int documentType();
QFont editFont();
QString editFontFamily();
int editFontPointSize();
int editLineLength();
BeQt::TabWidth editTabWidth();
QStringList fileHistory();
int maximumFileSize();
QByteArray searchModuleState();
void setAutoCodecDetectionEnabled(bool b);
void setDefaultCodec(QTextCodec *codec);
void setDefaultCodec(const QString &codecName);
void setDocumentDriverState(const QByteArray &state);
void setDocumentType(int t);
void setEditFont(const QFont &font);
void setEditFontFamily(const QString &family);
void setEditFontPointSize(int pointSize);
void setEditLineLength(int lineLength);
void setEditTabWidth(BeQt::TabWidth tabWidth);
void setFileHistory(const QStringList &history);
void setMaximumFileSize(int sz);
void setSearchModuleState(const QByteArray &state);
void setSpellCheckEnabled(bool b);
bool spellCheckEnabled();

}

}

#endif // SETTINGS_H
