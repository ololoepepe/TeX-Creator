/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
**
** This file is part of TeX Creator.
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

#include "editeditormodule.h"

#include <BAbstractCodeEditorDocument>
#include <BCodeEditor>
#include <BEditEditorModule>

#include <QMenu>
#include <QPointer>
#include <QString>

/*============================================================================
================================ EditEditorModule ============================
============================================================================*/

/*============================== Public constructors =======================*/

EditEditorModule::EditEditorModule()
{
    //
}

/*============================== Public methods ============================*/

void EditEditorModule::checkAutotext()
{
    if (mmnuAutotext.isNull() || mmnuAutotext->isEmpty())
        return;
    action(PasteAction)->setEnabled(currentDocument());
}

QString EditEditorModule::id() const
{
    return "edit";
}

void EditEditorModule::setAutotextMenu(QMenu *mnu)
{
    mmnuAutotext = mnu;
    checkAutotext();
}

/*============================== Protected methods =========================*/

void EditEditorModule::currentDocumentChanged(BAbstractCodeEditorDocument *doc)
{
    BEditEditorModule::currentDocumentChanged(doc);
    checkAutotext();
}

void EditEditorModule::documentCopyAvailableChanged(bool available)
{
    BEditEditorModule::documentCopyAvailableChanged(available);
    checkAutotext();
}

void EditEditorModule::documentCutAvailableChanged(bool available)
{
    BEditEditorModule::documentCutAvailableChanged(available);
    checkAutotext();
}

void EditEditorModule::documentPasteAvailableChanged(bool available)
{
    BEditEditorModule::documentPasteAvailableChanged(available);
    checkAutotext();
}

void EditEditorModule::documentRedoAvailableChanged(bool available)
{
    BEditEditorModule::documentRedoAvailableChanged(available);
    checkAutotext();
}

void EditEditorModule::documentUndoAvailableChanged(bool available)
{
    BEditEditorModule::documentUndoAvailableChanged(available);
    checkAutotext();
}

void EditEditorModule::editorSet(BCodeEditor *edr)
{
    BEditEditorModule::editorSet(edr);
    checkAutotext();
}
