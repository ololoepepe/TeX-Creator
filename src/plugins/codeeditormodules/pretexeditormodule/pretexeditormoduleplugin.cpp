/****************************************************************************
**
** Copyright (C) 2014 Andrey Bogdanov
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

#include "modulecomponents.h"
#include "pretexeditormodule.h"
#include "pretexsettingstab.h"

#include <BApplication>
#include <BCodeEditor>
#include <BDirTools>
#include <BeQt>
#include <BLocationProvider>
#include <BPluginWrapper>
#include <BProperties>
#include <BTranslator>
#include <BVersion>

#include <QAction>
#include <QDebug>
#include <QDockWidget>
#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMetaType>
#include <QPixmap>
#include <QSettings>
#include <QString>
#include <QtPlugin>
#include <QVariant>

/*============================================================================
================================ PretexEditorModulePlugin ====================
============================================================================*/

/*============================== Private static members ====================*/

PretexEditorModulePlugin *PretexEditorModulePlugin::minstance = 0;

/*============================== Public constructors =======================*/

PretexEditorModulePlugin::PretexEditorModulePlugin()
{
    Q_INIT_RESOURCE(pretexeditormodule_1);
    Q_INIT_RESOURCE(pretexeditormodule_2);
    Q_INIT_RESOURCE(pretexeditormodule_3);
    minstance = this;
    mprovider = new BLocationProvider;
    mprovider->addLocation("pretex");
    mprovider->createLocationPath("pretex", BApplication::UserResource);
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
}

PretexEditorModulePlugin::~PretexEditorModulePlugin()
{
    delete mprovider;
    minstance = 0;
    Q_CLEANUP_RESOURCE(pretexeditormodule_1);
    Q_CLEANUP_RESOURCE(pretexeditormodule_2);
    Q_CLEANUP_RESOURCE(pretexeditormodule_3);
}

/*============================== Static public methods =====================*/

BProperties PretexEditorModulePlugin::externalTools()
{
    BProperties map;
    QSettings *s = BPluginWrapper::parentWrapper(instance())->settings();
    s->beginGroup(path("ExternalTools"));
    foreach (const QString &k, s->childKeys())
        map.insert(k, s->value(k).toString());
    s->endGroup();
    return map;
}

PretexEditorModulePlugin *PretexEditorModulePlugin::instance()
{
    return minstance;
}

QByteArray PretexEditorModulePlugin::moduleState(PretexEditorModule *module)
{
    return BPluginWrapper::parentWrapper(instance())->settings()->value(path("moudle_state", "", module)).toByteArray();
}

void PretexEditorModulePlugin::setExternalTools(const BProperties &map)
{
    QSettings *s = BPluginWrapper::parentWrapper(instance())->settings();
    s->remove(path("ExternalTools"));
    foreach (const QString &k, map.keys())
        s->setValue(path("ExternalTools/" + k), map.value(k));
}

void PretexEditorModulePlugin::setModuleState(const QByteArray &state, PretexEditorModule *module)
{
    BPluginWrapper::parentWrapper(instance())->settings()->setValue(path("moudle_state", "", module), state);
}

/*============================== Public methods ============================*/

void PretexEditorModulePlugin::activate()
{
    BApplication::installBeqtTranslator("pretexeditormodule");
    BApplication::installLocationProvider(mprovider);
}

BAboutDialog *PretexEditorModulePlugin::createAboutDialog()
{
    return 0;
}

QList<BAbstractSettingsTab *> PretexEditorModulePlugin::createSettingsTabs()
{
    return QList<BAbstractSettingsTab *>() << new PretexSettingsTab;
}

void PretexEditorModulePlugin::deactivate()
{
    BApplication::removeLocationProvider(mprovider);
    BApplication::removeBeqtTranslator("pretexeditormodule");
}

QString PretexEditorModulePlugin::helpIndex() const
{
    return "index.html";
}

QStringList PretexEditorModulePlugin::helpSearchPaths() const
{
    return QStringList() << BDirTools::localeBasedDirName(":/pretexeditormodule/doc");
}

QString PretexEditorModulePlugin::id() const
{
    return type() + "/pretex-js";
}

PretexEditorModulePlugin::PluginInfo PretexEditorModulePlugin::info() const
{
    PluginInfo pi;
    pi.organization = "Andrey Bogdanov";
    pi.copyrightYears = "2014-2015";
    pi.website = "https://github.com/ololoepepe/TeX-Creator";
    pi.descriptionFileName = ":/pretexeditormodule/description/DESCRIPTION.txt";
    pi.changeLogFileName = ":/pretexeditormodule/changelog/ChangeLog.txt";
    pi.licenseFileName = ":/pretexeditormodule/copying/COPYING.txt";
    pi.authorsFileName = ":/pretexeditormodule/infos/authors.beqt-info";
    pi.translatorsFileName = ":/pretexeditormodule/infos/translators.beqt-info";
    return pi;
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

QPixmap PretexEditorModulePlugin::pixmap() const
{
    return QPixmap(":/pretexeditormodule/pixmaps/pretexeditormodule.png");
}

bool PretexEditorModulePlugin::prefereStaticInfo() const
{
    return false;
}

void PretexEditorModulePlugin::processStandardAboutDialog(BAboutDialog *) const
{
    //
}

PretexEditorModulePlugin::StaticPluginInfo PretexEditorModulePlugin::staticInfo() const
{
    return StaticPluginInfo();
}

QString PretexEditorModulePlugin::title() const
{
    return tr("PreTeX Editor Module", "title");
}

QString PretexEditorModulePlugin::type() const
{
    return "editor-module";
}

bool PretexEditorModulePlugin::uninstallModule(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw)
        return false;
    ModuleComponents c = mmap.take(cedtr);
    if (!c.isValid())
        return false;
    setModuleState(c.module->saveState(), c.module);
    c.uninstall();
    return true;
}

BVersion PretexEditorModulePlugin::version() const
{
    return BVersion(2, 0, 0, BVersion::Beta);
}

/*============================== Static private methods ====================*/

QString PretexEditorModulePlugin::path(const QString &key, const QString &section, PretexEditorModule *module)
{
    QString s = "PreTeX";
    if (!section.isEmpty())
        s += "/" + section;
    if (module && module->editor())
        s += "/" + module->editor()->objectName();
    if (!key.isEmpty())
        s += "/" + key;
    return s;
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
