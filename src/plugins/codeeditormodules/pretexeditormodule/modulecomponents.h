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

#ifndef MODULECOMPONENTS_H
#define MODULECOMPONENTS_H

class PretexEditorModule;

class BCodeEditor;

class QMainWindow;
class QMenu;
class QDockWidget;

#include <QApplication>

/*============================================================================
================================ ModuleComponents ============================
============================================================================*/

class ModuleComponents
{
    Q_DECLARE_TR_FUNCTIONS(ModuleComponents)
public:
    explicit ModuleComponents();
    explicit ModuleComponents(BCodeEditor *cedtr, QMainWindow *mw);
public:
    void retranslate();
    void uninstall();
    bool isValid() const;
public:
    PretexEditorModule *module;
    BCodeEditor *editor;
    QMainWindow *window;
    QMenu *menu;
    QDockWidget *dock;
};

#endif // MODULECOMPONENTS_H
