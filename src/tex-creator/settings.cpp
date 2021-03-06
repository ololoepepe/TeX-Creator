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

#include "settings.h"

#include "application.h"

#include <TTexCompiler>

#include <BCodeEditor>
#include <BeQt>
#include <BGuiTools>
#include <BPassword>
#include <BPasswordWidget>

#include <QByteArray>
#include <QFont>
#include <QList>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QVariant>

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
    return bSettings->value(EditLineLengthPath, 300).toInt();
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

/*============================================================================
================================ Compiler ====================================
============================================================================*/

namespace Compiler
{

static const QString RootPath = "Compiler";
static const QString CompilerPath = RootPath + "/compiler";
static const QString CompilerCommandsPath = RootPath + "/compiler_commands";
static const QString CompilerOptionsPath = RootPath + "/compiler_options";
static const QString DvipsEnabledPath = RootPath + "/dvips_enabled";
static const QString MakeindexEnabledPath = RootPath + "/makeindex_enabled";

TTexCompiler compiler()
{
    return bSettings->value(CompilerPath, TTexCompiler(TTexCompiler::Pdflatex)).value<TTexCompiler>();
}

QStringList compilerCommands()
{
    return bSettings->value(CompilerCommandsPath).toStringList();
}

QStringList compilerOptions()
{
    return bSettings->value(CompilerOptionsPath).toStringList();
}

bool dvipsEnabled()
{
    return bSettings->value(DvipsEnabledPath).toBool();
}

bool makeindexEnabled()
{
    return bSettings->value(MakeindexEnabledPath).toBool();
}

void setCompiler(const TTexCompiler &c)
{
    bSettings->setValue(CompilerPath, c);
}

void setCompilerCommands(const QStringList &list)
{
    bSettings->setValue(CompilerCommandsPath, list);
}

void setCompilerOptions(const QStringList &list)
{
    bSettings->setValue(CompilerOptionsPath, list);
}

void setDvipsEnabled(bool enabled)
{
    bSettings->setValue(DvipsEnabledPath, enabled);
}

void setMakeindexEnabled(bool enabled)
{
    bSettings->setValue(MakeindexEnabledPath, enabled);
}

}

/*============================================================================
================================ Console =====================================
============================================================================*/

namespace Console
{

static const QString RootPath = "Console";
static const QString AlwaysLatinEnabledPath = RootPath + "/always_latin_enabled";
static const QString FallbackToLocalCompilerPath = RootPath + "/fallback_to_local_compiler";
static const QString UseRemoteCompilerPath = RootPath + "/use_remote_compiler";

bool alwaysLatinEnabled()
{
    return bSettings->value(AlwaysLatinEnabledPath, false).toBool();
}

bool fallbackToLocalCompiler()
{
    return bSettings->value(FallbackToLocalCompilerPath).toBool();
}

bool hasFallbackToLocalCompiler()
{
    return bSettings->contains(FallbackToLocalCompilerPath);
}

void setAlwaysLatinEnabled(bool enabled)
{
    bSettings->setValue(AlwaysLatinEnabledPath, enabled);
}

void setFallbackToLocalCompiler(bool b)
{
    bSettings->setValue(FallbackToLocalCompilerPath, b);
}

void setUseRemoteCompiler(bool b)
{
    bSettings->setValue(UseRemoteCompilerPath, b);
}

bool useRemoteCompiler()
{
    return bSettings->value(UseRemoteCompilerPath).toBool();
}

}

/*============================================================================
================================ General =====================================
============================================================================*/

namespace General
{

static const QString RootPath = "Core";
static const QString CheckForNewVersionOnStartupPath = RootPath + "/check_for_new_version_on_startup";
static const QString MultipleWindowsEnabledPath = RootPath + "/multiple_windows_enabled";

bool checkForNewVersionOnStartup()
{
    return bSettings->value(CheckForNewVersionOnStartupPath, true).toBool();
}

bool multipleWindowsEnabled()
{
    return bSettings->value(MultipleWindowsEnabledPath, false).toBool();
}

void setCheckForNewVersionOnStartup(bool b)
{
    bSettings->setValue(CheckForNewVersionOnStartupPath, b);
}

void setMultipleWindowsEnabled(bool enabled)
{
    bSettings->setValue(MultipleWindowsEnabledPath, enabled);
}

}

/*============================================================================
================================ MainWindow ==================================
============================================================================*/

namespace MainWindow
{

static const QString RootPath = "MainWindow";
static const QString WindowGeometryPath = RootPath + "/window_geometry";
static const QString WindowStatePath = RootPath + "/window_state";

void setWindowGeometry(const QByteArray &geometry)
{
    bSettings->setValue(WindowGeometryPath, geometry);
}

void setWindowState(const QByteArray &state)
{
    bSettings->setValue(WindowStatePath, state);
}

QByteArray windowGeometry()
{
    return bSettings->value(WindowGeometryPath).toByteArray();
}

QByteArray windowState()
{
    return bSettings->value(WindowStatePath).toByteArray();
}

}

/*============================================================================
================================ Network =====================================
============================================================================*/

namespace Network
{

static const QString RootPath = "Network";
static const QString ProxyPath = RootPath + "/Proxy";
static const QString ProxyHostPath = ProxyPath + "/host";
static const QString ProxyLoginPath = ProxyPath + "/login";
static const QString ProxyModePath = ProxyPath + "/mode";
static const QString ProxyPasswordPath = ProxyPath + "/password";
static const QString ProxyPortPath = ProxyPath + "/port";

QList<ProxyMode> allProxyModes()
{
    return QList<ProxyMode>() << NoProxy << SystemProxy << UserProxy;
}

QString proxyHost()
{
    return bSettings->value(ProxyHostPath).toString();
}

ProxyMode proxyMode()
{
    return enum_cast<ProxyMode>(bSettings->value(ProxyModePath, NoProxy), allProxyModes(), NoProxy);
}

QString proxyLogin()
{
    return bSettings->value(ProxyLoginPath).toString();
}

QString proxyPassword()
{
    return bSettings->value(ProxyPasswordPath).toString();
}

quint16 proxyPort()
{
    return bSettings->value(ProxyPortPath).toUInt();
}

void setProxyHost(const QString &host)
{
    bSettings->setValue(ProxyHostPath, host);
}

void setProxyLogin(const QString &login)
{
    bSettings->setValue(ProxyLoginPath, login);
}

void setProxyMode(ProxyMode m)
{
    bSettings->setValue(ProxyModePath, (int) m);
}

void setProxyPassword(const QString &pwd)
{
    bSettings->setValue(ProxyPasswordPath, pwd);
}

void setProxyPort(quint16 p)
{
    bSettings->setValue(ProxyPortPath, p);
}

}

/*============================================================================
================================ SampleInfoWidget ============================
============================================================================*/

namespace SampleInfoWidget
{

static const QString RootPath = "SampleInfoWidget";
static const QString SelectExternalFileDialogGeometryPath = RootPath + "/select_external_file_dialog_geometry";
static const QString SelectExternalFileDialogStatePath = RootPath + "/select_external_file_dialog_state";

QByteArray selectExternalFileDialogGeometry()
{
    return bSettings->value(SelectExternalFileDialogGeometryPath).toByteArray();
}

QByteArray selectExternalFileDialogState()
{
    return bSettings->value(SelectExternalFileDialogStatePath).toByteArray();
}

void setSelectExternalFileDialogGeometry(const QByteArray &geometry)
{
    bSettings->setValue(SelectExternalFileDialogGeometryPath, geometry);
}

void setSelectExternalFileDialogState(const QByteArray &state)
{
    bSettings->setValue(SelectExternalFileDialogStatePath, state);
}

}

/*============================================================================
================================ Texsample ===================================
============================================================================*/

namespace Texsample
{

static const QString RootPath = "TeXSample";
static const QString CachingEnabledPath = RootPath + "/caching_enabled";
static const QString ConnectOnStartupPath = RootPath + "/connect_on_startup";
static const QString HostPath = RootPath + "/host";
static const QString HostHistoryPath = RootPath + "/host_history";
static const QString LoginPath = RootPath + "/login";
static const QString PasswordPath = RootPath + "/password";
static const QString PasswordWidgetStatePath = RootPath + "/password_widget_state";

BPassword mpassword;

bool cachingEnabled()
{
    return bSettings->value(CachingEnabledPath, true).toBool();
}

bool connectOnStartup()
{
    return bSettings->value(ConnectOnStartupPath, true).toBool();
}

bool hasTexsample()
{
    return bSettings->contains(ConnectOnStartupPath);
}

QString host(bool resolveSpecialName)
{
    QString h = bSettings->value(HostPath, UsueTexsampleServerHost).toString();
    typedef QMap<QString, QString> StringMap;
    init_once(StringMap, map, StringMap()) {
        map.insert(UsueTexsampleServerHost, "texsample-server.no-ip.org");
    }
    return (resolveSpecialName && map.contains(h)) ? map.value(h) : h;
}

QStringList hostHistory()
{
    return bSettings->value(HostHistoryPath).toStringList();
}

void loadPassword()
{
    mpassword = bSettings->value(PasswordPath).value<BPassword>();
}

QString login()
{
    return bSettings->value(LoginPath).toString();
}

BPassword password()
{
    return mpassword;
}

QByteArray passwordWidgetState()
{
    return bSettings->value(PasswordWidgetStatePath).toByteArray();
}

void savePassword()
{
    bSettings->setValue(PasswordPath, mpassword.toEncrypted());
}

void setCachingEnabled(bool enabled)
{
    bSettings->setValue(CachingEnabledPath, enabled);
}

void setConnectOnStartup(bool enabled)
{
    bSettings->setValue(ConnectOnStartupPath, enabled);
}

void setHost(const QString &host)
{
    bSettings->setValue(HostPath, host);
}

void setHostHistory(const QStringList &history)
{
    bSettings->setValue(HostHistoryPath, history);
}

void setLogin(const QString &login)
{
    bSettings->setValue(LoginPath, login);
}

void setPassword(const BPassword &pwd)
{
    mpassword = pwd;
}

void setPasswordWidgetState(const QByteArray &state)
{
    bSettings->setValue(PasswordWidgetStatePath, state);
    if (!BPasswordWidget::savePassword(state))
        bSettings->remove(PasswordPath);
}

}

/*============================================================================
================================ TexsampleCore ===============================
============================================================================*/

namespace TexsampleCore
{

static const QString RootPath = "TexsampleCore";
static const QString AccountManagementDialogGeometryPath = RootPath + "/account_management_dialog_geometry";
static const QString GroupManagementDialogGeometryPath = RootPath + "/group_management_dialog_geometry";
static const QString InviteManagementDialogGeometryPath = RootPath + "/invite_management_dialog_geometry";
static const QString SampleInfoDialogGeometryPath = RootPath + "/sample_info_dialog_geometry";
static const QString SaveSampleDirPath = RootPath + "/save_sample_dir";
static const QString SelectSampleSubdirDialogGeometryPath = RootPath + "/select_sample_subdir_dialog_geometry";
static const QString SelectSampleSubdirDialogStatePath = RootPath + "/select_sample_subdir_dialog_state";
static const QString SendSampleDialogGeometryPath = RootPath + "/send_sample_dialog_geometry";
static const QString SendSampleWidgetStatePath = RootPath + "/send_sample_widget_state";
static const QString UserInfoDialogGeometryPath = RootPath + "/user_info_dialog_geometry";
static const QString UserManagementDialogGeometryPath = RootPath + "/user_management_dialog_geometry";

QByteArray accountManagementDialogGeometry()
{
    return bSettings->value(AccountManagementDialogGeometryPath).toByteArray();
}

QByteArray groupManagementDialogGeometry()
{
    return bSettings->value(GroupManagementDialogGeometryPath).toByteArray();
}

QByteArray inviteManagementDialogGeometry()
{
    return bSettings->value(InviteManagementDialogGeometryPath).toByteArray();
}

QByteArray sampleInfoDialogGeometry()
{
    return bSettings->value(SampleInfoDialogGeometryPath).toByteArray();
}

QString saveSampleDir()
{
    return bSettings->value(SaveSampleDirPath).toString();
}

QByteArray selectSampleSubdirDialogGeometry()
{
    return bSettings->value(SelectSampleSubdirDialogGeometryPath).toByteArray();
}

QByteArray selectSampleSubdirDialogState()
{
    return bSettings->value(SelectSampleSubdirDialogStatePath).toByteArray();
}

QByteArray sendSampleDialogGeometry()
{
    return bSettings->value(SendSampleDialogGeometryPath).toByteArray();
}

QByteArray sendSampleWidgetState()
{
    return bSettings->value(SendSampleWidgetStatePath).toByteArray();
}

QByteArray userInfoDialogGeometry()
{
    return bSettings->value(SampleInfoDialogGeometryPath).toByteArray();
}

QByteArray userManagementDialogGeometry()
{
    return bSettings->value(UserManagementDialogGeometryPath).toByteArray();
}

void setAccountManagementDialogGeometry(const QByteArray &geometry)
{
    bSettings->setValue(AccountManagementDialogGeometryPath, geometry);
}

void setGroupManagementDialogGeometry(const QByteArray &geometry)
{
    bSettings->setValue(GroupManagementDialogGeometryPath, geometry);
}

void setInviteManagementDialogGeometry(const QByteArray &geometry)
{
    bSettings->setValue(InviteManagementDialogGeometryPath, geometry);
}

void setSampleInfoDialogGeometry(const QByteArray &geometry)
{
    bSettings->setValue(SampleInfoDialogGeometryPath, geometry);
}

void setSaveSampleDir(const QString &dir)
{
    bSettings->setValue(SaveSampleDirPath, dir);
}

void setSelectSampleSubdirDialogGeometry(const QByteArray &geometry)
{
    bSettings->setValue(SelectSampleSubdirDialogGeometryPath, geometry);
}

void setSelectSampleSubdirDialogState(const QByteArray &state)
{
    bSettings->setValue(SelectSampleSubdirDialogStatePath, state);
}

void setSendSampleDialogGeometry(const QByteArray &geometry)
{
    bSettings->setValue(SendSampleDialogGeometryPath, geometry);
}

void setSendSampleWidgetState(const QByteArray &state)
{
    bSettings->setValue(SendSampleWidgetStatePath, state);
}

void setUserInfoDialogGeometry(const QByteArray &geometry)
{
    bSettings->setValue(UserInfoDialogGeometryPath, geometry);
}

void setUserManagementDialogGeometry(const QByteArray &geometry)
{
    bSettings->setValue(UserManagementDialogGeometryPath, geometry);
}

}

/*============================================================================
================================ TexsampleWidget =============================
============================================================================*/

namespace TexsampleWidget
{

static const QString RootPath = "TexsampleWidget";
static const QString SampleTableHeaderStatePath = RootPath + "/sample_table_header_state";
static const QString SelectedSampleTypePath = RootPath + "/selected_sample_type";

void setSampleTableHeaderState(const QByteArray &state)
{
    bSettings->setValue(SampleTableHeaderStatePath, state);
}

void setSelectedSampleType(int type)
{
    bSettings->setValue(SelectedSampleTypePath, type);
}

QByteArray sampleTableHeaderState()
{
    return bSettings->value(SampleTableHeaderStatePath).toByteArray();
}

int selectedSampleType()
{
    return bSettings->value(SelectedSampleTypePath).toInt();
}

}

}
