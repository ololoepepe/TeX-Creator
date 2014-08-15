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

#include <QAction>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QString>

/*============================================================================
================================ ModuleComponents ============================
============================================================================*/

/*============================== Public constructors =======================*/

ModuleComponents::ModuleComponents()
{
    editor = 0;
    module = 0;
    separator1 = 0;
    separator2 = 0;
    window = 0;
}

ModuleComponents::ModuleComponents(BCodeEditor *cedtr, QMainWindow *mw)
{
    if (!cedtr || !mw) {
        module = 0;
        editor = 0;
        separator1 = 0;
        separator2 = 0;
        window = 0;
        return;
    }
    module = new KeyboardLayoutEditorModule;
    editor = cedtr;
    window = mw;
    cedtr->addModule(module);
    QMenu *mnu = mw->findChild<QMenu *>("MenuEdit");
    if (mnu) {
        QAction *last = mnu->actions().last();
        separator1 = mnu->insertSeparator(last);
        mnu->insertAction(last, module->action(KeyboardLayoutEditorModule::SwitchSelectedTextLayoutAction));
        separator2 = mnu->insertSeparator(last);
    }
    mnu = mw->findChild<QMenu *>("MenuTools");
    if (mnu)
        mnu->addAction(module->action(KeyboardLayoutEditorModule::OpenUserKLMDirAction));
}

/*============================== Public methods ============================*/

bool ModuleComponents::isValid() const
{
    return module && window && separator1 && separator2;
}

void ModuleComponents::uninstall()
{
    if (!isValid())
        return;
    editor->removeModule(module);
    QMenu *mnu = window->findChild<QMenu *>("MenuEdit");
    if (mnu) {
        mnu->removeAction(separator1);
        mnu->removeAction(separator2);
        mnu->removeAction(module->action(KeyboardLayoutEditorModule::SwitchSelectedTextLayoutAction));
    }
    mnu = window->findChild<QMenu *>("MenuTools");
    if (mnu)
        mnu->removeAction(module->action(KeyboardLayoutEditorModule::OpenUserKLMDirAction));
    separator1 = 0;
    separator2 = 0;
    module = 0;
    editor = 0;
    window = 0;
}
