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

#include "global.h"
#include "application.h"

#include <BCodeEdit>
#include <BCodeEditor>
#include <BPasswordWidget>
#include <BPassword>
#include <TCompilerParameters>

#include <QFont>
#include <QString>
#include <QTextCodec>
#include <QStringList>
#include <QByteArray>
#include <QVariant>
#include <QSettings>

#include <QDebug>

namespace Global
{

QByteArray pwdState;

QStringList splitArguments(const QString &string)
{
    QStringList list = string.split(' ', QString::SkipEmptyParts);
    if (list.isEmpty())
        return list;
    int x = -1;
    foreach (int i, bRangeR(list.size() - 1, 0))
    {
        QString &arg = list[i];
        if (x < 0 && arg.at(arg.length() - 1) == '\"')
            x = i;
        if (x >= 0 && arg.at(0) == '\"')
        {
            if (x != i)
                foreach (int j, bRange(x, i + 1))
                    arg.append(" " + list.takeAt(j));
            x = -1;
        }
    }
    return list;
}

QString joinArguments(const QStringList &list)
{
    QStringList nlist = list;
    if ( !nlist.isEmpty() )
    {
        foreach (int i, bRangeD(0, list.size() - 1))
        {
            QString &arg = nlist[i];
            if (arg.contains(' ') && (arg.at(0) != '\"' || arg.at(arg.length() - 1) != '\"'))
                arg.prepend('\"').append('\"');
        }
    }
    return nlist.join(" ");
}

int indexOfHelper(const QString &text, const QString &what, int from)
{
    if (text.isEmpty() || what.isEmpty())
        return -1;
    int ind = text.indexOf(what, from);
    while (ind >= 0)
    {
        if (!ind || text.at(ind - 1) != '\\')
            return ind;
        ind = text.indexOf(what, ++from);
    }
    return -1;
}

//CodeEditor

void setEditorDocumentType(int t)
{
    BCodeEditor::StandardDocumentType tt = BCodeEditor::standardDocumentTypeFromInt(t);
    if (editorDocumentType() == tt)
        return;
    bSettings->setValue("CodeEditor/document_type", tt);
    Application::updateDocumentType();
}

void setEditorSpellCheckEnabled(bool b)
{
    bSettings->setValue("CodeEditor/spell_check_enabled", b);
}

void setEditFont(const QFont &font)
{
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditFont(font);
    setEditFontFamily(font.family());
    setEditFontPointSize(font.pointSize());
}

void setEditFontFamily(const QString &family)
{
    if (family.isEmpty())
        return;
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditFontFamily(family);
    bSettings->setValue("CodeEditor/edit_font_family", family);
}

void setEditFontPointSize(int pointSize)
{
    if (pointSize < 1)
        return;
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditFontPointSize(pointSize);
    bSettings->setValue("CodeEditor/edit_font_point_size", pointSize);
}

void setAutoCodecDetectionEnabled(bool b)
{
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setAutoCodecDetectionEnabled(b);
    bSettings->setValue("CodeEditor/auto_codec_detection_enabled", b);
}

void setDefaultCodec(QTextCodec *codec)
{
    if (!codec)
        return;
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setDefaultCodec(codec);
    bSettings->setValue("CodeEditor/default_codec", BeQt::codecName(codec));
}

void setDefaultCodec(const QByteArray &codecName)
{
    setDefaultCodec(QTextCodec::codecForName(codecName));
}

void setDefaultCodec(const QString &codecName)
{
    setDefaultCodec(codecName.toLatin1());
}

void setMaxDocumentSize(int sz)
{
    if (maxDocumentSize() == sz)
        return;
    bSettings->setValue("CodeEditor/maximum_file_size", sz < 0 ? (2 * BeQt::Megabyte) : sz);
    Application::updateMaxDocumentSize();
}

void setEditLineLength(int lineLength)
{
    if (lineLength < 10 || lineLength > 1000)
        return;
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditLineLength(lineLength);
    bSettings->setValue("CodeEditor/edit_line_length", lineLength);
}

void setEditTabWidth(int tabWidth)
{
    switch (tabWidth)
    {
    case BeQt::TabWidth2:
    case BeQt::TabWidth4:
    case BeQt::TabWidth8:
        break;
    default:
        return;
    }
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditTabWidth(static_cast<BeQt::TabWidth>(tabWidth));
    bSettings->setValue("CodeEditor/edit_tab_width", tabWidth);
}

void setFileHistory(const QStringList &history)
{
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setFileHistory(history);
    bSettings->setValue("CodeEditor/file_history", history);
}

void setDocumentDriverState(const QByteArray &state)
{
    bSettings->setValue("CodeEditor/document_driver_state", state);
}

void setSearchModuleState(const QByteArray &state)
{
    bSettings->setValue("CodeEditor/search_moudle_state", state);
}

//Console

void setCompilerParameters(const TCompilerParameters &param)
{
    bSettings->setValue("Console/compiler_parameters", param);
}

void setCompiler(int c)
{
    TCompilerParameters param = compilerParameters();
    param.setCompiler(c);
    setCompilerParameters(param);
}

void setCompilerOptions(const QStringList &list)
{
    TCompilerParameters param = compilerParameters();
    param.setOptions(list);
    setCompilerParameters(param);
}

void setCompilerOptions(const QString &string)
{
    setCompilerOptions(QStringList() << string);
}

void setCompilerCommands(const QStringList &list)
{
    TCompilerParameters param = compilerParameters();
    param.setCommands(list);
    setCompilerParameters(param);
}

void setCompilerCommands(const QString &string)
{
    setCompilerCommands(QStringList() << string);
}

void setMakeindexEnabled(bool enabled)
{
    TCompilerParameters param = compilerParameters();
    param.setMakeindexEnabled(enabled);
    setCompilerParameters(param);
}

void setDvipsEnabled(bool enabled)
{
    TCompilerParameters param = compilerParameters();
    param.setDvipsEnabled(enabled);
    setCompilerParameters(param);
}

void setUseRemoteCompiler(bool b)
{
    bSettings->setValue("Console/use_remote_compiler", b);
    Application::emitUseRemoteCompilerChanged();
}

void setFallbackToLocalCompiler(bool b)
{
    bSettings->setValue("Console/fallback_to_local_compiler", b);
}

void setAlwaysLatinEnabled(bool enabled)
{
    bSettings->setValue("Console/always_latin_enabled", enabled);
}

//General

void setMultipleWindowsEnabled(bool enabled)
{
    bSettings->setValue("Core/multiple_windows_enabled", enabled);
}

void setCheckForNewVersions(bool b)
{
    bSettings->setValue("Core/check_for_new_versions", b);
}

//TeXSample

void setAutoconnection(bool enabled)
{
    bSettings->setValue("TeXSample/Client/autoconnection", enabled);
}

void setHost(const QString &host)
{
    bSettings->setValue("TeXSample/Client/host", host);
}

void setHostHistory(const QStringList &history)
{
    bSettings->setValue("TeXSample/Client/host_history", history);
}

void setLogin(const QString &login)
{
    bSettings->setValue("TeXSample/Client/login", login);
}

void setPasswordWidgetSate(const QByteArray &state)
{
    bSettings->setValue("TeXSample/Client/password_widget_state", state);
}

void setPasswordState(const QByteArray &state)
{
    pwdState = state;
}

void setPassword(const BPassword &pwd)
{
    setPasswordState(pwd.save(BPassword::AlwaysEncryptedMode));
}

void setPassword(const QByteArray &pwd, int charCountHint)
{
    setPassword(BPassword(QCryptographicHash::Sha1, pwd, charCountHint));
}

void setPassword(const QString &pwd)
{
    setPassword(BPassword(pwd));
}

void setCachingEnabled(bool enabled)
{
    bSettings->setValue("TeXSample/Cache/enabled", enabled);
}

//Network

void setProxyMode(ProxyMode m)
{
    if (!bRangeD(NoProxy, UserProxy).contains(m))
        return;
    bSettings->setValue("Network/Proxy/mode", (int) m);
}

void setProxyHost(const QString &host)
{
    bSettings->setValue("Network/Proxy/host", host);
}

void setProxyPort(int p)
{
    if (p < 0)
        return;
    bSettings->setValue("Network/Proxy/port", p);
}

void setProxyLogin(const QString &login)
{
    bSettings->setValue("Network/Proxy/login", login);
}

void setProxyPassword(const QString &pwd)
{
    bSettings->setValue("Network/Proxy/password", pwd);
}

//CodeEditor

BCodeEditor::StandardDocumentType editorDocumentType()
{
    return BCodeEditor::standardDocumentTypeFromInt(bSettings->value("CodeEditor/document_type",
                                                                     BCodeEditor::StandardDocument).toInt());
}

bool editorSpellCheckEnabled()
{
    return bSettings->value("CodeEditor/spell_check_enabled", true).toBool();
}

QFont editFont()
{
    QFont fnt = Application::createMonospaceFont();
    fnt.setFamily(bSettings->value("CodeEditor/edit_font_family", fnt.family()).toString());
    fnt.setPointSize(bSettings->value("CodeEditor/edit_font_point_size", fnt.pointSize()).toInt());
    return fnt;
}

QString editFontFamily()
{
    return editFont().family();
}

int editFontPointSize()
{
    return editFont().pointSize();
}

bool autoCodecDetectionEnabled()
{
    return bSettings->value("CodeEditor/auto_codec_detection_enabled", true).toBool();
}

QTextCodec *defaultCodec()
{
    return BeQt::codec(bSettings->value("CodeEditor/default_codec",
                                        QTextCodec::codecForLocale()->name()).toByteArray());
}

QString defaultCodecName()
{
    return BeQt::codecName(defaultCodec());
}

int maxDocumentSize()
{
    bool ok = false;
    int sz = bSettings->value("CodeEditor/maximum_file_size", 2 * BeQt::Megabyte).toInt(&ok);
    if (sz < 0 || !ok)
        sz = 2 * BeQt::Megabyte;
    return sz;
}

int editLineLength()
{
    return bSettings->value("CodeEditor/edit_line_length", 120).toInt();
}

BeQt::TabWidth editTabWidth()
{
    return static_cast<BeQt::TabWidth>(bSettings->value("CodeEditor/edit_tab_width", BeQt::TabWidth4).toInt());
}

QStringList fileHistory()
{
    return bSettings->value("CodeEditor/file_history").toStringList();
}

QByteArray documentDriverState()
{
    return bSettings->value("CodeEditor/document_driver_state").toByteArray();
}

QByteArray searchModuleState()
{
    return bSettings->value("CodeEditor/search_moudle_state").toByteArray();
}

//Console

bool hasFallbackToLocalCompiler()
{
    return bSettings->contains("Console/fallback_to_local_compiler");
}

TCompilerParameters compilerParameters()
{
    return bSettings->value("Console/compiler_parameters").value<TCompilerParameters>();
}

TCompilerParameters::Compiler compiler()
{
    return compilerParameters().compiler();
}

QStringList compilerOptions()
{
    return compilerParameters().options();
}

QString compilerOptionsString(bool command)
{
    return compilerParameters().optionsString(command);
}

QStringList compilerCommands()
{
    return compilerParameters().commands();
}

QString compilerCommandsString(bool command)
{
    return compilerParameters().commandsString(command);
}

bool makeindexEnabled()
{
    return compilerParameters().makeindexEnabled();
}

bool dvipsEnabled()
{
    return compilerParameters().dvipsEnabled();
}

bool useRemoteCompiler()
{
    return bSettings->value("Console/use_remote_compiler").toBool();
}

bool fallbackToLocalCompiler()
{
    return bSettings->value("Console/fallback_to_local_compiler").toBool();
}

bool alwaysLatinEnabled()
{
    return bSettings->value("Console/always_latin_enabled", false).toBool();
}

//General

bool multipleWindowsEnabled()
{
    return bSettings->value("Core/multiple_windows_enabled", false).toBool();
}

bool checkForNewVersions()
{
    return bSettings->value("Core/check_for_new_versions", true).toBool();
}

//TeXSample

bool hasTexsample()
{
    return bSettings->contains("TeXSample/Client/autoconnection");
}

bool autoconnection()
{
    return bSettings->value("TeXSample/Client/autoconnection", true).toBool();
}

QString host()
{
    QString host = bSettings->value("TeXSample/Client/host", "auto_select").toString();
    return hostHistory().contains(host) ? host : "auto_select";
}

QStringList hostHistory()
{
    return bSettings->value("TeXSample/Client/host_history").toStringList();
}

QString login()
{
    return bSettings->value("TeXSample/Client/login").toString();
}

QByteArray passwordWidgetState()
{
    return bSettings->value("TeXSample/Client/password_widget_state").toByteArray();
}

QByteArray passwordState()
{
    return pwdState;
}

BPassword password()
{
    BPassword pwd;
    pwd.restore(passwordState());
    return pwd;
}

QByteArray encryptedPassword(int *charCountHint)
{
    return password().encryptedPassword(charCountHint);
}

bool cachingEnabled()
{
    return bSettings->value("TeXSample/Cache/enabled", true).toBool();
}

void savePasswordState()
{
    BPasswordWidget *pwdwgt = new BPasswordWidget;
    pwdwgt->restoreWidgetState(passwordWidgetState());
    bSettings->setValue("TeXSample/Client/password_state", pwdwgt->savePassword() ? pwdState : QByteArray());
    delete pwdwgt;
}

void loadPasswordState()
{
    pwdState = bSettings->value("TeXSample/Client/password_state").toByteArray();
}

//Network

ProxyMode proxyMode()
{
    bool ok = false;
    int m = bSettings->value("Network/Proxy/mode").toInt(&ok);
    return (ok && bRangeD(NoProxy, UserProxy).contains(m)) ? static_cast<ProxyMode>(m) : NoProxy;
}

QString proxyHost()
{
    return bSettings->value("Network/Proxy/host").toString();
}

int proxyPort()
{
    bool ok = false;
    int p = bSettings->value("Network/Proxy/port").toInt(&ok);
    return (ok && p >= 0) ? p : 0;
}

QString proxyLogin()
{
    return bSettings->value("Network/Proxy/login").toString();
}

QString proxyPassword()
{
    return bSettings->value("Network/Proxy/password").toString();
}

}
