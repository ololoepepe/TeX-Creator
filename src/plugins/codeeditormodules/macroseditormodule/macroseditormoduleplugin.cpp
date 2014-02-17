#include "macroseditormoduleplugin.h"
#include "macroseditormodule.h"
#include "macrossettingstab.h"

#include <BPluginWrapper>
#include <BeQt>
#include <BTranslator>
#include <BApplication>
#include <BCodeEditor>

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

#include <QDebug>

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static QSettings *settings()
{
    foreach (BPluginWrapper *pw, BApplication::pluginWrappers("editor-module"))
        if (pw && pw->name() == "MacrosEditorModulePlugin")
            return pw->settings();
    return 0;
}

/*============================================================================
================================ MacrosSettingsTab ===========================
============================================================================*/

/*============================== Static public methods =====================*/

void MacrosEditorModulePlugin::setMacrosModuleState(const QByteArray &state)
{
    QSettings *s = settings();
    if (!s)
        return;
    s->setValue("Macros/moudle_state", state);
}

void MacrosEditorModulePlugin::setSaveMacroStack(bool b)
{
    QSettings *s = settings();
    if (!s)
        return;
    s->setValue("Macros/save_stack", b);
}

void MacrosEditorModulePlugin::setExternalTools(const QMap<QString, QString> &map)
{
    QSettings *s = settings();
    if (!s)
        return;
    s->remove("Macros/ExternalTools");
    foreach (const QString &k, map.keys())
        s->setValue("Macros/ExternalTools/" + k, map.value(k));
}

QByteArray MacrosEditorModulePlugin::macrosModuleState()
{
    QSettings *s = settings();
    if (!s)
        return QByteArray();
    return s->value("Macros/moudle_state").toByteArray();
}

bool MacrosEditorModulePlugin::saveMacroStack()
{
    QSettings *s = settings();
    if (!s)
        return false;
    return s->value("Macros/save_stack", true).toBool();
}

QMap<QString, QString> MacrosEditorModulePlugin::externalTools()
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

MacrosEditorModulePlugin::MacrosEditorModulePlugin()
{
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
}

MacrosEditorModulePlugin::~MacrosEditorModulePlugin()
{
    //
}

/*============================== Public methods ============================*/

QString MacrosEditorModulePlugin::type() const
{
    return "editor-module";
}

QString MacrosEditorModulePlugin::name() const
{
    return "MacrosEditorModulePlugin";
}

bool MacrosEditorModulePlugin::prefereStaticInfo() const
{
    return true;
}

MacrosEditorModulePlugin::PluginInfoStatic MacrosEditorModulePlugin::staticInfo() const
{
    PluginInfoStatic i;
    i.organization = "Andrey Bogdanov";
    i.copyrightYears = "2014";
    i.website = "https://github.com/the-dark-angel";
    return i;
}

MacrosEditorModulePlugin::PluginInfo MacrosEditorModulePlugin::info() const
{
    return PluginInfo();
}

void MacrosEditorModulePlugin::activate()
{
    BCoreApplication::installTranslator(new BTranslator("macroseditormodule"));
    if (saveMacroStack())
        MacrosEditorModule::loadMacroStack();
}

void MacrosEditorModulePlugin::deactivate()
{
    BCoreApplication::removeTranslator(BCoreApplication::translator("macroseditormodule"));
    if (saveMacroStack())
        MacrosEditorModule::saveMacroStack();
}

QPixmap MacrosEditorModulePlugin::pixmap() const
{
    return QPixmap(":/pixmaps/gstreamer.png");
}

BAbstractSettingsTab *MacrosEditorModulePlugin::settingsTab() const
{
    return new MacrosSettingsTab;
}

void MacrosEditorModulePlugin::handleSettings(const QVariantMap &)
{
    //
}

bool MacrosEditorModulePlugin::installModule(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw)
        return false;
    ModuleComponents c(cedtr, mw);
    if (!c.isValid())
        return false;
    mmap.insert(cedtr, c);
    return true;
}

bool MacrosEditorModulePlugin::uninstallModule(BCodeEditor *cedtr, QMainWindow *mw)
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

void MacrosEditorModulePlugin::retranslateUi()
{
    foreach (ModuleComponents c, mmap)
        c.retranslate();
}

/*============================================================================
================================ ::ModuleComponentsMacrosSettingsTab =========
============================================================================*/

/*============================== Public constructors =======================*/

MacrosEditorModulePlugin::ModuleComponents::ModuleComponents()
{
    module = 0;
    editor = 0;
    window = 0;
    menu = 0;
    dock = 0;
}

MacrosEditorModulePlugin::ModuleComponents::ModuleComponents(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw)
    {
        module = 0;
        editor = 0;
        window = 0;
        menu = 0;
        dock = 0;
        return;
    }
    module = new MacrosEditorModule;
    editor = cedtr;
    window = mw;
    cedtr->addModule(module);
    mw->installEventFilter(module->dropHandler());
    mw->installEventFilter(module->closeHandler());
    module->restoreState(MacrosEditorModulePlugin::macrosModuleState());
    dock = new QDockWidget;
      dock->setObjectName("DockWidgetMacrosEditor");
      dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
      dock->setWidget(module->widget(MacrosEditorModule::MacrosEditorWidget));
    mw->addDockWidget(Qt::TopDockWidgetArea, dock);
    QMenu *mnu = mw->findChild<QMenu *>("MenuTools");
    if (mnu)
    {
        QList<QAction *> acts = mnu->actions();
        if (!acts.isEmpty())
        {
            menu = new QMenu;
            menu->setObjectName("MenuMacros");
            mnu->insertSeparator(acts.first());
            mnu->insertMenu(mnu->actions().first(), menu);
        }
        else
        {
            menu = mnu->addMenu("");
            menu->setObjectName("MenuMacros");
        }
        menu->addActions(module->actions(true));
    }
    else
    {
        menu = 0;
    }
}

/*============================== Public methods ============================*/

void MacrosEditorModulePlugin::ModuleComponents::retranslate()
{
    if (!isValid())
        return;
    if (menu)
        menu->setTitle(tr("Macros", "mnu title"));
    dock->setWindowTitle(tr("Macros editor", "dwgt windowTitle"));
}

void MacrosEditorModulePlugin::ModuleComponents::uninstall()
{
    if (!isValid())
        return;
    menu->deleteLater();
    dock->deleteLater();
    editor->removeModule(module);
    module = 0;
    editor = 0;
    window = 0;
    menu = 0;
    dock = 0;
}

bool MacrosEditorModulePlugin::ModuleComponents::isValid() const
{
    return module && window && dock;
}

Q_EXPORT_PLUGIN2(macroseditormodule, MacrosEditorModulePlugin)
