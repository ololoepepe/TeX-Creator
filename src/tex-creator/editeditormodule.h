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

#ifndef EDITEDITORMODULE_H
#define EDITEDITORMODULE_H

class BAbstractCodeEditorDocument;
class BCodeEditor;

class QString;

#include <BEditEditorModule>

#include <QMenu>
#include <QObject>
#include <QPointer>

/*============================================================================
================================ EditEditorModule ============================
============================================================================*/

class EditEditorModule : public BEditEditorModule
{
    Q_OBJECT
private:
    QPointer<QMenu> mmnuAutotext;
public:
    explicit EditEditorModule();
public:
    void checkAutotext();
    QString id() const;
    void setAutotextMenu(QMenu *mnu);
protected:
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void documentCopyAvailableChanged(bool available);
    void documentCutAvailableChanged(bool available);
    void documentPasteAvailableChanged(bool available);
    void documentRedoAvailableChanged(bool available);
    void documentUndoAvailableChanged(bool available);
    void editorSet(BCodeEditor *edr);
};

#endif // EDITEDITORMODULE_H
