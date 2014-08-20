/****************************************************************************
**
** Copyright (C) 2014 Andrey Bogdanov
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

#include <BApplication>
#include <BeQt>
#include <BLocationProvider>
#include <BSignalDelayProxy>
#include <BVersion>

#include <QDebug>
#include <QFileSystemWatcher>
#include <QList>
#include <QMap>
#include <QPixmap>
#include <QString>
#include <QtPlugin>

/*============================================================================
================================ KeyboardLayoutEditorModulePlugin ============
============================================================================*/

/*============================== Public constructors =======================*/

KeyboardLayoutEditorModulePlugin::KeyboardLayoutEditorModulePlugin()
{
    mprovider = new BLocationProvider;
    mprovider->addLocation("klm");
    mprovider->createLocationPath("klm", BApplication::UserResource);
    mfsWatcher = new QFileSystemWatcher(this);
    QStringList paths;
    paths << mprovider->locationPath("klm", BApplication::SharedResource);
    paths << mprovider->locationPath("klm", BApplication::UserResource);
    paths.removeAll("");
    mfsWatcher->addPaths(paths);
    BSignalDelayProxy *sdp = new BSignalDelayProxy(BeQt::Second, 2 * BeQt::Second, this);
    sdp->setStringConnection(mfsWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));
}

KeyboardLayoutEditorModulePlugin::~KeyboardLayoutEditorModulePlugin()
{
    delete mprovider;
}

/*============================== Public methods ============================*/

void KeyboardLayoutEditorModulePlugin::activate()
{
    BApplication::installBeqtTranslator("keyboardlayouteditormodule");
    BApplication::installLocationProvider(mprovider);
}

BAboutDialog *KeyboardLayoutEditorModulePlugin::createAboutDialog()
{
    return 0;
}

QList<BAbstractSettingsTab *> KeyboardLayoutEditorModulePlugin::createSettingsTabs()
{
    return QList<BAbstractSettingsTab *>();
}

void KeyboardLayoutEditorModulePlugin::deactivate()
{
    BApplication::removeLocationProvider(mprovider);
    BApplication::removeBeqtTranslator("keyboardlayouteditormodule");
}

QString KeyboardLayoutEditorModulePlugin::helpIndex() const
{
    return QString();
}

QStringList KeyboardLayoutEditorModulePlugin::helpSearchPaths() const
{
    return QStringList();
}

QString KeyboardLayoutEditorModulePlugin::id() const
{
    return type() + "/keyboard_layout";
}

KeyboardLayoutEditorModulePlugin::PluginInfo KeyboardLayoutEditorModulePlugin::info() const
{
    PluginInfo pi;
    pi.organization = "Andrey Bogdanov";
    pi.copyrightYears = "2014";
    pi.website = "https://github.com/ololoepepe/TeX-Creator";
    pi.descriptionFileName = ":/keyboardlayouteditormodule/description/DESCRIPTION.txt";
    pi.changeLogFileName = ":/keyboardlayouteditormodule/changelog/ChangeLog.txt";
    pi.licenseFileName = ":/keyboardlayouteditormodule/copying/COPYING.txt";
    pi.authorsFileName = ":/keyboardlayouteditormodule/infos/authors.beqt-info";
    pi.translatorsFileName = ":/keyboardlayouteditormodule/infos/translators.beqt-info";
    return pi;
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

QPixmap KeyboardLayoutEditorModulePlugin::pixmap() const
{
    return QPixmap(":/keyboardlayouteditormodule/pixmaps/keyboardlayouteditormodule.png");
}

bool KeyboardLayoutEditorModulePlugin::prefereStaticInfo() const
{
    return false;
}

void KeyboardLayoutEditorModulePlugin::processStandardAboutDialog(BAboutDialog *) const
{
    //
}

KeyboardLayoutEditorModulePlugin::StaticPluginInfo KeyboardLayoutEditorModulePlugin::staticInfo() const
{
    return StaticPluginInfo();
}

QString KeyboardLayoutEditorModulePlugin::title() const
{
    return tr("Keyboard Layout Editor Module", "title");
}

QString KeyboardLayoutEditorModulePlugin::type() const
{
    return "editor-module";
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
    return BVersion(0, 1, 0);
}

/*============================== Private slots =============================*/

void KeyboardLayoutEditorModulePlugin::directoryChanged(const QString &path)
{
    if (!BApplication::locations("klm").contains(path))
        return;
    foreach (ModuleComponents c, mmap) {
        if (!c.isValid())
            continue;
        c.module->reloadMap();
    }
    mfsWatcher->addPath(path);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(keyboardlayouteditormodule, KeyboardLayoutEditorModulePlugin)
#endif
