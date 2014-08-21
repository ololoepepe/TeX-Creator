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

#ifndef SETTINGS_H
#define SETTINGS_H

class TTexCompiler;

class BPassword;

class QByteArray;
class QFont;
class QString;
class QStringList;
class QTextCodec;

#include <BeQt>

#include <QList>

/*============================================================================
================================ Settings ====================================
============================================================================*/

namespace Settings
{

/*============================================================================
================================ CodeEditor ==================================
============================================================================*/

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

/*============================================================================
================================ Compiler ====================================
============================================================================*/

namespace Compiler
{

TTexCompiler compiler();
QStringList compilerCommands();
QStringList compilerOptions();
bool dvipsEnabled();
bool makeindexEnabled();
void setCompiler(const TTexCompiler &c);
void setCompilerCommands(const QStringList &list);
void setCompilerOptions(const QStringList &list);
void setDvipsEnabled(bool enabled);
void setMakeindexEnabled(bool enabled);

}

/*============================================================================
================================ Console =====================================
============================================================================*/

namespace Console
{

bool alwaysLatinEnabled();
bool fallbackToLocalCompiler();
bool hasFallbackToLocalCompiler();
void setAlwaysLatinEnabled(bool enabled);
void setFallbackToLocalCompiler(bool b);
void setUseRemoteCompiler(bool b);
bool useRemoteCompiler();

}

/*============================================================================
================================ General =====================================
============================================================================*/

namespace General
{

bool checkForNewVersionOnStartup();
bool multipleWindowsEnabled();
void setCheckForNewVersionOnStartup(bool b);
void setMultipleWindowsEnabled(bool enabled);

}

/*============================================================================
================================ MainWindow ==================================
============================================================================*/

namespace MainWindow
{

void setWindowGeometry(const QByteArray &geometry);
void setWindowState(const QByteArray &state);
QByteArray windowGeometry();
QByteArray windowState();

}

/*============================================================================
================================ Network =====================================
============================================================================*/

namespace Network
{

enum ProxyMode
{
    NoProxy = 0,
    SystemProxy,
    UserProxy
};

QList<ProxyMode> allProxyModes();
QString proxyHost();
ProxyMode proxyMode();
QString proxyLogin();
QString proxyPassword();
quint16 proxyPort();
void setProxyHost(const QString &host);
void setProxyLogin(const QString &login);
void setProxyMode(ProxyMode m);
void setProxyPassword(const QString &pwd);
void setProxyPort(quint16 p);

}

/*============================================================================
================================ SampleInfoWidget ============================
============================================================================*/

namespace SampleInfoWidget
{

QByteArray selectExternalFileDialogGeometry();
QByteArray selectExternalFileDialogState();
void setSelectExternalFileDialogGeometry(const QByteArray &geometry);
void setSelectExternalFileDialogState(const QByteArray &state);

}

/*============================================================================
================================ Texsample ===================================
============================================================================*/

namespace Texsample
{

const QString UsueTexsampleServerHost = "USUE TeXSample Server";

bool cachingEnabled();
bool connectOnStartup();
bool hasTexsample();
QString host(bool resolveSpecialName = false);
QStringList hostHistory();
void loadPassword();
QString login();
BPassword password();
QByteArray passwordWidgetState();
void savePassword();
void setCachingEnabled(bool enabled);
void setConnectOnStartup(bool enabled);
void setHost(const QString &host);
void setHostHistory(const QStringList &history);
void setLogin(const QString &login);
void setPassword(const BPassword &pwd);
void setPasswordWidgetState(const QByteArray &state);

}

/*============================================================================
================================ TexsampleWidget =============================
============================================================================*/

namespace TexsampleWidget
{

void setSampleTableHeaderState(const QByteArray &state);
void setSelectedSampleType(int type);
QByteArray sampleTableHeaderState();
int selectedSampleType();

}

}

#endif // SETTINGS_H
