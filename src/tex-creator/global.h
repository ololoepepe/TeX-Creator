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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <BCodeEdit>
#include <BCodeEditor>

#include <QMap>
#include <QString>

class BPassword;

class QFont;
class QTextCodec;
class QStirngList;
class QByteArray;

namespace Global
{

enum ProxyMode
{
    NoProxy = 0,
    SystemProxy,
    UserProxy
};

int indexOfHelper(const QString &text, const QString &what, int from = 0);

//CodeEditor
void setEditorDocumentType(int t);
void setEditorSpellCheckEnabled(bool b);
void setEditFont(const QFont &font);
void setEditFontFamily(const QString &family);
void setEditFontPointSize(int pointSize);
void setAutoCodecDetectionEnabled(bool b);
void setDefaultCodec(QTextCodec *codec);
void setDefaultCodec(const QByteArray &codecName);
void setDefaultCodec(const QString &codecName);
void setMaxDocumentSize(int sz);
void setEditLineLength(int lineLength);
void setEditTabWidth(int tabWidth);
void setFileHistory(const QStringList &history);
void setDocumentDriverState(const QByteArray &state);
void setSearchModuleState(const QByteArray &state);
//Console
/*void setCompilerParameters(const TCompilerParameters &param);
void setCompiler(int c);
void setCompilerOptions(const QStringList &list);
void setCompilerOptions(const QString &string);
void setCompilerCommands(const QStringList &list);
void setCompilerCommands(const QString &string);
void setMakeindexEnabled(bool enabled);
void setDvipsEnabled(bool enabled);
void setUseRemoteCompiler(bool b);
void setFallbackToLocalCompiler(bool b);
void setAlwaysLatinEnabled(bool enabled);*/
//General
void setMultipleWindowsEnabled(bool enabled);
void setCheckForNewVersions(bool b);
//TeXSample
void setAutoconnection(bool enabled);
void setHost(const QString &host);
void setHostHistory(const QStringList &history);
void setLogin(const QString &login);
void setPasswordWidgetSate(const QByteArray &state);
void setPasswordState(const QByteArray &state);
void setPassword(const BPassword &pwd);
void setPassword(const QByteArray &pwd, int charCountHint = 0);
void setPassword(const QString &pwd);
void setCachingEnabled(bool enabled);
//Network
void setProxyMode(ProxyMode m);
void setProxyHost(const QString &host);
void setProxyPort(int p);
void setProxyLogin(const QString &login);
void setProxyPassword(const QString &pwd);
//CodeEditor
int editorDocumentType();
bool editorSpellCheckEnabled();
QFont editFont();
QString editFontFamily();
int editFontPointSize();
bool autoCodecDetectionEnabled();
QTextCodec *defaultCodec();
QString defaultCodecName();
int maxDocumentSize();
int editLineLength();
BeQt::TabWidth editTabWidth();
QStringList fileHistory();
QByteArray documentDriverState();
QByteArray searchModuleState();
//Console
/*bool hasFallbackToLocalCompiler();
TCompilerParameters compilerParameters();
TCompilerParameters::Compiler compiler();
QStringList compilerOptions();
QString compilerOptionsString(bool command = true);
QStringList compilerCommands();
QString compilerCommandsString(bool command = true);
bool makeindexEnabled();
bool dvipsEnabled();
bool useRemoteCompiler();
bool fallbackToLocalCompiler();
bool alwaysLatinEnabled();*/
//General
bool multipleWindowsEnabled();
bool checkForNewVersions();
//TeXSample
bool hasTexsample();
bool autoconnection();
QString host();
QStringList hostHistory();
QString login();
QByteArray passwordWidgetState();
QByteArray passwordState();
BPassword password();
QByteArray encryptedPassword(int *charCountHint = 0);
bool cachingEnabled();
void savePasswordState();
void loadPasswordState();
//Network
ProxyMode proxyMode();
QString proxyHost();
int proxyPort();
QString proxyLogin();
QString proxyPassword();

}

#endif // GLOBAL_H
