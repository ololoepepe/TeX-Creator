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
#include "pretexsettingstab.h"
#include "modulecomponents.h"
#include "pretexarray.h"
#include "pretexfunction.h"
#include "pretexvariant.h"
#include "pretexbuiltinfunction.h"

#include <BPluginWrapper>
#include <BeQt>
#include <BTranslator>
#include <BApplication>
#include <BCodeEditor>
#include <BDirTools>
#include <BVersion>
#include <BLocationProvider>

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

static QString path(const QString &key = QString(), const QString &section = QString(), PretexEditorModule *module = 0)
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

/*============================================================================
================================ PretexEditorModulePlugin ====================
============================================================================*/

/*============================== Public constructors =======================*/

PretexEditorModulePlugin::PretexEditorModulePlugin()
{
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
}

/*============================== Static public methods =====================*/

PretexEditorModulePlugin *PretexEditorModulePlugin::instance()
{
    return minstance;
}

void PretexEditorModulePlugin::setExecutionStackState(const QByteArray &state, PretexEditorModule *module)
{
    BPluginWrapper::parentWrapper(instance())->settings()->setValue(path("state", "ExecutionStack", module), state);
}

void PretexEditorModulePlugin::setModuleState(const QByteArray &state, PretexEditorModule *module)
{
    BPluginWrapper::parentWrapper(instance())->settings()->setValue(path("moudle_state", "", module), state);
}

void PretexEditorModulePlugin::setSaveExecutionStack(bool b)
{
    BPluginWrapper::parentWrapper(instance())->settings()->setValue(path("save_execution_stack"), b);
}

void PretexEditorModulePlugin::setExternalTools(const QMap<QString, QString> &map)
{
    QSettings *s = BPluginWrapper::parentWrapper(instance())->settings();
    s->remove(path("ExternalTools"));
    foreach (const QString &k, map.keys())
        s->setValue(path("ExternalTools/" + k), map.value(k));
}

QByteArray PretexEditorModulePlugin::executionStackState(PretexEditorModule *module)
{
    return BPluginWrapper::parentWrapper(instance())->settings()->value(path("state", "ExecutionStack",
                                                                             module)).toByteArray();
}

QByteArray PretexEditorModulePlugin::moduleState(PretexEditorModule *module)
{
    return BPluginWrapper::parentWrapper(instance())->settings()->value(path("moudle_state", "", module)).toByteArray();
}

bool PretexEditorModulePlugin::saveExecutionStack()
{
    return BPluginWrapper::parentWrapper(instance())->settings()->value(path("save_execution_stack"), true).toBool();
}

QMap<QString, QString> PretexEditorModulePlugin::externalTools()
{
    QMap<QString, QString> map;
    QSettings *s = BPluginWrapper::parentWrapper(instance())->settings();
    s->beginGroup(path("ExternalTools"));
    foreach (const QString &k, s->childKeys())
        map.insert(k, s->value(k).toString());
    s->endGroup();
    return map;
}

void PretexEditorModulePlugin::clearExecutionStack()
{
    BPluginWrapper::parentWrapper(instance())->settings()->remove(path("", "ExecutionStack"));
}

void PretexEditorModulePlugin::clearExecutionStack(PretexEditorModule *module)
{
    BPluginWrapper::parentWrapper(instance())->settings()->remove(path("state", "ExecutionStack", module));
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

PretexEditorModulePlugin::StaticPluginInfo PretexEditorModulePlugin::staticInfo() const
{
    return StaticPluginInfo();
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
    qRegisterMetaTypeStreamOperators<PretexArray>();
    qRegisterMetaType<PretexFunction>();
    qRegisterMetaTypeStreamOperators<PretexFunction>();
    qRegisterMetaType<PretexVariant>();
    qRegisterMetaTypeStreamOperators<PretexVariant>();
    BApplication::installBeqtTranslator("pretexeditormodule");
    BApplication::installLocationProvider(mprovider);
    PretexBuiltinFunction::init();
}

void PretexEditorModulePlugin::deactivate()
{
    BApplication::removeLocationProvider(mprovider);
    BApplication::removeBeqtTranslator("pretexeditormodule");
    PretexBuiltinFunction::cleanup();
}

QPixmap PretexEditorModulePlugin::pixmap() const
{
    return QPixmap(":/pretexeditormodule/pixmaps/pretexeditormodule.png");
}

BAbstractSettingsTab *PretexEditorModulePlugin::createSettingsTab()
{
    return new PretexSettingsTab;
}

QStringList PretexEditorModulePlugin::helpSearchPaths() const
{
    return QStringList() << BDirTools::localeBasedDirName(":/pretexeditormodule/doc");
}

QString PretexEditorModulePlugin::helpIndex() const
{
    return "index.html";
}

BAboutDialog *PretexEditorModulePlugin::createAboutDialog()
{
    return 0;
}

void PretexEditorModulePlugin::processStandardAboutDialog(BAboutDialog *) const
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
    setModuleState(c.module->saveState(), c.module);
    c.uninstall();
    return true;
}

BVersion PretexEditorModulePlugin::version() const
{
    return BVersion(1, 0, 0, BVersion::Beta);
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

/*============================== Private static members ====================*/

PretexEditorModulePlugin *PretexEditorModulePlugin::minstance = 0;
