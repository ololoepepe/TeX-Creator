/****************************************************************************
**
** Copyright (C) 2014 TeXSample Team
**
** This file is part of the PreTeX Editor Module plugin of TeX Creator.
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

#include "pretexeditormoduleplugin.h"
#include "pretexeditormodule.h"
#include "macrossettingstab.h"
#include "modulecomponents.h"
#include "pretexarray.h"
#include "pretexfunction.h"
#include "pretexstatement.h"
#include "pretexvariant.h"

#include <BPluginWrapper>
#include <BeQt>
#include <BTranslator>
#include <BApplication>
#include <BCodeEditor>
#include <BDirTools>

#include <QString>
#include <QPixmap>
#include <QtPlugin>
#include <QSettings>
#include <QVariant>
#include <QMainWindow>
#include <QMap>
#include <QList>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QMetaType>

#include <QDebug>

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static QSettings *settings()
{
    foreach (BPluginWrapper *pw, BApplication::pluginWrappers("editor-module"))
        if (pw && pw->name() == "PreTex Editor Module Plugin")
            return pw->settings();
    return 0;
}

/*============================================================================
================================ PretexEditorModulePlugin ====================
============================================================================*/

/*============================== Static public methods =====================*/

void PretexEditorModulePlugin::setMacrosModuleState(const QByteArray &state)
{
    QSettings *s = settings();
    if (!s)
        return;
    s->setValue("Macros/moudle_state", state);
}

void PretexEditorModulePlugin::setSaveMacroStack(bool b)
{
    QSettings *s = settings();
    if (!s)
        return;
    s->setValue("Macros/save_stack", b);
}

void PretexEditorModulePlugin::setExternalTools(const QMap<QString, QString> &map)
{
    QSettings *s = settings();
    if (!s)
        return;
    s->remove("Macros/ExternalTools");
    foreach (const QString &k, map.keys())
        s->setValue("Macros/ExternalTools/" + k, map.value(k));
}

QByteArray PretexEditorModulePlugin::macrosModuleState()
{
    QSettings *s = settings();
    if (!s)
        return QByteArray();
    return s->value("Macros/moudle_state").toByteArray();
}

bool PretexEditorModulePlugin::saveMacroStack()
{
    QSettings *s = settings();
    if (!s)
        return false;
    return s->value("Macros/save_stack", true).toBool();
}

QMap<QString, QString> PretexEditorModulePlugin::externalTools()
{
    QMap<QString, QString> map;
    QSettings *s = settings();
    if (!s)
        return map;
    s->beginGroup("Macros/ExternalTools");
    foreach (const QString &k, s->childKeys())
        map.insert(k, s->value(k).toString());
    s->endGroup();
    return map;
}

/*============================== Public constructors =======================*/

PretexEditorModulePlugin::PretexEditorModulePlugin()
{
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
}

PretexEditorModulePlugin::~PretexEditorModulePlugin()
{
    //
}

/*============================== Public methods ============================*/

QString PretexEditorModulePlugin::type() const
{
    return "editor-module";
}

QString PretexEditorModulePlugin::name() const
{
    return "PreTeX Editor Module";
}

bool PretexEditorModulePlugin::prefereStaticInfo() const
{
    return false;
}

PretexEditorModulePlugin::PluginInfoStatic PretexEditorModulePlugin::staticInfo() const
{
    return PluginInfoStatic();
}

PretexEditorModulePlugin::PluginInfo PretexEditorModulePlugin::info() const
{
    PluginInfo pi;
    pi.organization = "TeXSample Team";
    pi.copyrightYears = "2014";
    pi.website = "https://github.com/TeXSample-Team/TeX-Creator";
    pi.descriptionFileName = ":/pretexeditormodule/description/DESCRIPTION.txt";
    pi.changeLogFileName = ":/pretexeditormodule/changelog/ChangeLog.txt";
    pi.licenseFileName = ":/pretexeditormodule/copying/COPYING.txt";
    pi.authorsFileName = ":/pretexeditormodule/infos/authors.beqt-info";
    pi.translatorsFileName = ":/pretexeditormodule/infos/translators.beqt-info";
    return pi;
}

void PretexEditorModulePlugin::activate()
{
    qRegisterMetaType<PretexArray>();
    qRegisterMetaType<PretexFunction>();
    qRegisterMetaType<PretexStatement>();
    qRegisterMetaType<PretexVariant>();
    BCoreApplication::installTranslator(new BTranslator("pretexeditormodule"));
    if (saveMacroStack())
        PretexEditorModule::loadMacroStack();
}

void PretexEditorModulePlugin::deactivate()
{
    BCoreApplication::removeTranslator(BCoreApplication::translator("pretexeditormodule"));
    if (saveMacroStack())
        PretexEditorModule::saveMacroStack();
}

QPixmap PretexEditorModulePlugin::pixmap() const
{
    return QPixmap(":/pretexeditormodule/pixmaps/pretexeditormodule.png");
}

BAbstractSettingsTab *PretexEditorModulePlugin::settingsTab() const
{
    return new MacrosSettingsTab;
}

void PretexEditorModulePlugin::handleSettings(const QVariantMap &)
{
    //
}

bool PretexEditorModulePlugin::installModule(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw)
        return false;
    ModuleComponents c(cedtr, mw);
    if (!c.isValid())
        return false;
    mmap.insert(cedtr, c);
    return true;
}

bool PretexEditorModulePlugin::uninstallModule(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw)
        return false;
    ModuleComponents c = mmap.take(cedtr);
    if (!c.isValid())
        return false;
    setMacrosModuleState(c.module->saveState());
    c.uninstall();
    return true;
}

/*============================== Private slots =============================*/

void PretexEditorModulePlugin::retranslateUi()
{
    foreach (ModuleComponents c, mmap)
        c.retranslate();
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(pretexeditormodule, PretexEditorModulePlugin)
#endif
