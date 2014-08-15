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

#include "modulecomponents.h"
#include "keyboardlayouteditormodule.h"
#include "keyboardlayouteditormoduleplugin.h"

#include <BCodeEditor>
#include <QMainWindow>
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
}

ModuleComponents::ModuleComponents(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw)
    {
        module = 0;
        editor = 0;
        window = 0;
        menu = 0;
        return;
    }
    module = new KeyboardLayoutEditorModule;
    editor = cedtr;
    window = mw;
    cedtr->addModule(module);
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

void ModuleComponents::retranslate()
{
    if (!isValid())
        return;
    if (menu)
        menu->setTitle(tr("KeyboardLayout", "mnu title"));
}

void ModuleComponents::uninstall()
{
    if (!isValid())
        return;
    menu->deleteLater();
    editor->removeModule(module);
    module = 0;
    editor = 0;
    window = 0;
    menu = 0;
}

bool ModuleComponents::isValid() const
{
    return module && window;
}
