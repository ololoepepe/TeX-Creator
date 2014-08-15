/****************************************************************************
**
** Copyright (C) 2014 TeXSample Team
**
** This file is part of the Kayboard Layout Editor Module plugin
** of TeX Creator.
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

#include "keyboardlayouteditormoduleplugin.h"
#include "keyboardlayouteditormodule.h"
#include "modulecomponents.h"

#include <BPluginWrapper>
#include <BeQt>
#include <BTranslator>
#include <BApplication>
#include <BCodeEditor>
#include <BDirTools>
#include <BVersion>

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
================================ KeyboardLayoutEditorModulePlugin ============
============================================================================*/

/*============================== Public constructors =======================*/

KeyboardLayoutEditorModulePlugin::KeyboardLayoutEditorModulePlugin()
{
    //
}

KeyboardLayoutEditorModulePlugin::~KeyboardLayoutEditorModulePlugin()
{
    //
}

/*============================== Public methods ============================*/

QString KeyboardLayoutEditorModulePlugin::type() const
{
    return "editor-module";
}

QString KeyboardLayoutEditorModulePlugin::name() const
{
    return "Keyboard Layout Editor Module";
}

bool KeyboardLayoutEditorModulePlugin::prefereStaticInfo() const
{
    return false;
}

KeyboardLayoutEditorModulePlugin::StaticPluginInfo KeyboardLayoutEditorModulePlugin::staticInfo() const
{
    return StaticPluginInfo();
}

KeyboardLayoutEditorModulePlugin::PluginInfo KeyboardLayoutEditorModulePlugin::info() const
{
    PluginInfo pi;
    pi.organization = "TeXSample Team";
    pi.copyrightYears = "2014";
    pi.website = "https://github.com/TeXSample-Team/TeX-Creator";
    pi.descriptionFileName = ":/keyboardlayouteditormodule/description/DESCRIPTION.txt";
    pi.changeLogFileName = ":/keyboardlayouteditormodule/changelog/ChangeLog.txt";
    pi.licenseFileName = ":/keyboardlayouteditormodule/copying/COPYING.txt";
    pi.authorsFileName = ":/keyboardlayouteditormodule/infos/authors.beqt-info";
    pi.translatorsFileName = ":/keyboardlayouteditormodule/infos/translators.beqt-info";
    return pi;
}

void KeyboardLayoutEditorModulePlugin::activate()
{
    BCoreApplication::installBeqtTranslator("keyboardlayouteditormodule");
    //BDirTools::createUserLocation("pretex");
    //TODO
}

void KeyboardLayoutEditorModulePlugin::deactivate()
{
    BCoreApplication::removeBeqtTranslator("keyboardlayouteditormodule");
}

QPixmap KeyboardLayoutEditorModulePlugin::pixmap() const
{
    return QPixmap(":/keyboardlayouteditormodule/pixmaps/keyboardlayouteditormodule.png");
}

BAbstractSettingsTab *KeyboardLayoutEditorModulePlugin::createSettingsTab()
{
    return 0;
}

QStringList KeyboardLayoutEditorModulePlugin::helpSearchPaths() const
{
    //TODO
    return QStringList();
}

QString KeyboardLayoutEditorModulePlugin::helpIndex() const
{
    //TODO
    return QString();
}

BAboutDialog *KeyboardLayoutEditorModulePlugin::createAboutDialog()
{
    return 0;
}

void KeyboardLayoutEditorModulePlugin::processStandardAboutDialog(BAboutDialog *) const
{
    //
}

bool KeyboardLayoutEditorModulePlugin::installModule(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw)
        return false;
    ModuleComponents c(cedtr, mw);
    if (!c.isValid())
        return false;
    mmap.insert(cedtr, c);
    return true;
}

bool KeyboardLayoutEditorModulePlugin::uninstallModule(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw)
        return false;
    ModuleComponents c = mmap.take(cedtr);
    if (!c.isValid())
        return false;
    c.uninstall();
    return true;
}

BVersion KeyboardLayoutEditorModulePlugin::version() const
{
    //TODO
    return BVersion(1, 0, 0);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(keyboardlayouteditormodule, KeyboardLayoutEditorModulePlugin)
#endif
