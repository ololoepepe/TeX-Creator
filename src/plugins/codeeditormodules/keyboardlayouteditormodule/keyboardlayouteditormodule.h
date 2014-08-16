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

#ifndef KEYBOARDLAYOUTEDITORMODULE_H
#define KEYBOARDLAYOUTEDITORMODULE_H

class BAbstractCodeEditorDocument;

class QString;

#include "keyboardlayoutmap.h"

#include <BAbstractEditorModule>

#include <QAction>
#include <QList>
#include <QObject>
#include <QPointer>

/*============================================================================
================================ KeyboardLayoutEditorModule ==================
============================================================================*/

class KeyboardLayoutEditorModule : public BAbstractEditorModule
{
    Q_OBJECT
public:
    enum Action
    {
        OpenUserKLMDirAction,
        SwitchSelectedTextLayoutAction
    };
public:
    explicit KeyboardLayoutEditorModule(QObject *parent = 0);
public:
    QAction *action(int type);
    QList<QAction *> actions(bool extended = false);
    QString id() const;
public slots:
    void openUserDir();
    void reloadMap();
    void switchLayout();
protected:
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void documentHasSelectionChanged(bool hasSelection);
private:
    void checkSwitchAction();
private slots:
    void retranslateUi();
private:
    QPointer<QAction> mactOpenDir;
    QPointer<QAction> mactSwitch;
    KeyboardLayoutMap mmap;
private:
    Q_DISABLE_COPY(KeyboardLayoutEditorModule)
};

#endif // KEYBOARDLAYOUTEDITORMODULE_H
