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

#include "keyboardlayouteditormodule.h"

#include <BAbstractCodeEditorDocument>
#include <BAbstractEditorModule>
#include <BApplication>
#include <BCodeEditor>
#include <BDirTools>

#include <QAction>
#include <QDebug>
#include <QIcon>
#include <QKeySequence>
#include <QList>
#include <QLocale>
#include <QObject>
#include <QPointer>
#include <QString>

/*============================================================================
================================ KeyboardLayoutEditorModule ==================
============================================================================*/

/*============================== Public constructors =======================*/

KeyboardLayoutEditorModule::KeyboardLayoutEditorModule(QObject *parent) :
    BAbstractEditorModule(parent)
{
    mactSwitch = new QAction(this);
      mactSwitch->setEnabled(false);
      mactSwitch->setIcon(BApplication::icon("charset"));
      mactSwitch->setShortcut(QKeySequence("Ctrl+L"));
      connect(mactSwitch, SIGNAL(triggered()), this, SLOT(switchLayout()));
    mactOpenDir = new QAction(this);
      mactOpenDir->setIcon(BApplication::icon("folder_open"));
      connect(mactOpenDir, SIGNAL(triggered()), this, SLOT(openUserDir()));
    //
    reloadMap();
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
    retranslateUi();
}

/*============================== Public methods ============================*/

QString KeyboardLayoutEditorModule::id() const
{
    return "plugin/keyboard_layout";
}

QAction *KeyboardLayoutEditorModule::action(int type)
{
    switch (type) {
    case SwitchSelectedTextLayoutAction:
        return mactSwitch;
    case OpenUserKLMDirAction:
        return mactOpenDir;
    default:
        return 0;
    }
}

QList<QAction *> KeyboardLayoutEditorModule::actions(bool extended)
{
    QList<QAction *> list;
    list << action(SwitchSelectedTextLayoutAction);
    if (extended)
        list << action(OpenUserKLMDirAction);
    list.removeAll(0);
    return list;
}

/*============================== Public slots ==============================*/

void KeyboardLayoutEditorModule::openUserDir()
{
    bApp->openLocalFile(BDirTools::findResource("klm", BDirTools::UserOnly));
}

void KeyboardLayoutEditorModule::reloadMap()
{
    QString fn = BDirTools::findResource("klm/en-" + QLocale::system().name().left(2) + ".klm");
    mmap.load(fn);
    checkSwitchAction();
}

void KeyboardLayoutEditorModule::switchLayout()
{
    BAbstractCodeEditorDocument *doc = currentDocument();
    if (!doc || !doc->hasSelection())
        return;
    QString txt = doc->selectedText();
    int sstart = doc->selectionStart();
    int send = doc->selectionEnd();
    if (!mmap.switchLayout(txt))
        return;
    doc->insertText(txt);
    doc->selectText(sstart, send);
}

/*============================== Protected methods =========================*/

void KeyboardLayoutEditorModule::currentDocumentChanged(BAbstractCodeEditorDocument *)
{
    checkSwitchAction();
}

void KeyboardLayoutEditorModule::documentHasSelectionChanged(bool)
{
    checkSwitchAction();
}

/*============================== Private methods ===========================*/

void KeyboardLayoutEditorModule::checkSwitchAction()
{
    if (mactSwitch.isNull())
        return;
    mactSwitch->setEnabled(currentDocument() && currentDocument()->hasSelection() && mmap.isValid());
}

/*============================== Private slots =============================*/

void KeyboardLayoutEditorModule::retranslateUi()
{
    if (!mactSwitch.isNull()) {
        mactSwitch->setText(tr("Switch layout", "act text"));
        mactSwitch->setToolTip(tr("Switch selected text layout", "act toolTip"));
        mactSwitch->setWhatsThis(tr("Use this action to switch selected text layout (e.g. from EN to RU)",
                                    "act whatsThis"));
    }
    if (!mactOpenDir.isNull()) {
        mactOpenDir->setText(tr("Open user keyboard layout map directory", "act text"));
        mactOpenDir->setWhatsThis(tr("Use this action to open keyboard layouts user directory", "act whatsThis"));
    }
}
