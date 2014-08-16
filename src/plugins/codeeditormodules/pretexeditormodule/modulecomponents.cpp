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

#include "modulecomponents.h"
#include "pretexeditormodule.h"
#include "pretexeditormoduleplugin.h"

#include <BCodeEditor>
#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QString>

/*============================================================================
================================ ModuleComponents ============================
============================================================================*/

/*============================== Public constructors =======================*/

ModuleComponents::ModuleComponents()
{
    module = 0;
    editor = 0;
    window = 0;
    menu = 0;
    dock = 0;
}

ModuleComponents::ModuleComponents(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw) {
        module = 0;
        editor = 0;
        window = 0;
        menu = 0;
        dock = 0;
        return;
    }
    module = new PretexEditorModule;
    editor = cedtr;
    window = mw;
    cedtr->addModule(module);
    mw->installEventFilter(module->closeHandler());
    module->restoreState(PretexEditorModulePlugin::moduleState(module));
    dock = new QDockWidget;
    dock->setObjectName("DockWidgetMacrosEditor");
    dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dock->setWidget(module->widget(PretexEditorModule::PretexEditorWidget));
    dock->installEventFilter(module->dropHandler());
    mw->addDockWidget(Qt::TopDockWidgetArea, dock);
    QMenu *mnu = mw->findChild<QMenu *>("MenuTools");
    QList<QAction *> acts = mnu->actions();
    if (!acts.isEmpty()) {
        menu = new QMenu;
        mnu->insertSeparator(acts.first());
        mnu->insertMenu(mnu->actions().first(), menu);
    } else {
        menu = mnu->addMenu("");
    }
    menu->setObjectName("MenuPretex");
    menu->addActions(module->actions(true));
}

/*============================== Public methods ============================*/

void ModuleComponents::retranslate()
{
    if (!isValid())
        return;
    if (menu)
        menu->setTitle(tr("PreTeX", "mnu title"));
    dock->setWindowTitle(tr("PreTeX", "dwgt windowTitle"));
}

void ModuleComponents::uninstall()
{
    if (!isValid())
        return;
    delete menu;
    delete dock;
    editor->removeModule(module);
    module = 0;
    editor = 0;
    window = 0;
    menu = 0;
    dock = 0;
}

bool ModuleComponents::isValid() const
{
    return module && window && dock;
}
