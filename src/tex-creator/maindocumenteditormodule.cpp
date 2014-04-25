/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
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

#include "maindocumenteditormodule.h"
#include "application.h"

#include <BAbstractEditorModule>
#include <BCodeEditor>
#include <BAbstractCodeEditorDocument>

#include <QObject>
#include <QList>
#include <QString>
#include <QAction>
#include <QVariant>
#include <QPointer>
#include <QIcon>

/*============================================================================
================================ MainDocumentEditorModule ====================
============================================================================*/

/*============================== Public constructors =======================*/

MainDocumentEditorModule::MainDocumentEditorModule(QObject *parent) :
    BAbstractEditorModule(parent)
{
    mmainDocument = 0;
    mact = new QAction(this);
      connect(mact.data(), SIGNAL(triggered()), this, SLOT(switchCurrentDocumentMain()));
    //
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
    retranslateUi();
}

MainDocumentEditorModule::~MainDocumentEditorModule()
{
    //
}

/*============================== Public methods ============================*/

QString MainDocumentEditorModule::id() const
{
    return "main_document";
}

QAction *MainDocumentEditorModule::action(int type)
{
    switch (type)
    {
    case SwitchCurrentDocumentMainAction:
        return mact.data();
    default:
        return 0;
    }
}

QList<QAction *> MainDocumentEditorModule::actions(bool)
{
    QList<QAction *> list;
    list << action(SwitchCurrentDocumentMainAction);
    return list;
}

void MainDocumentEditorModule::setMainDocument(BAbstractCodeEditorDocument *doc)
{
    if (doc == mmainDocument)
        return;
    if ( doc && ( !editor() || !editor()->documents().contains(doc) ) )
        return;
    mmainDocument = doc;
    resetAction();
    emit mainDocumentChanged(doc);
}

bool MainDocumentEditorModule::isCurrentDocumentMain() const
{
    return mmainDocument && (currentDocument() == mmainDocument);
}

BAbstractCodeEditorDocument *MainDocumentEditorModule::mainDocument() const
{
    return mmainDocument;
}

QString MainDocumentEditorModule::mainDocumentFileName() const
{
    return mmainDocument ? mmainDocument->fileName() : QString();
}

/*============================== Public slots ==============================*/

void MainDocumentEditorModule::switchCurrentDocumentMain()
{
    setMainDocument(!isCurrentDocumentMain() ? currentDocument() : 0);
    resetAction();
}

/*============================== Protected methods =========================*/

void MainDocumentEditorModule::editorSet(BCodeEditor *edr)
{
    if (!edr)
        return setMainDocument(0);
    foreach (BAbstractCodeEditorDocument *doc, edr->documents())
    {
        if (doc->property("main_document").toBool())
        {
            setMainDocument(doc);
            break;
        }
    }
}

void MainDocumentEditorModule::editorUnset(BCodeEditor *)
{
    setMainDocument(0);
}

void MainDocumentEditorModule::currentDocumentChanged(BAbstractCodeEditorDocument *doc)
{
    if (!doc)
        setMainDocument(0);
    resetAction();
}

/*============================== Private methods ===========================*/

void MainDocumentEditorModule::resetAction()
{
    if ( mact.isNull() )
        return;
    mact->setEnabled( currentDocument() );
    if ( isCurrentDocumentMain() )
    {
        mact->setIcon( Application::icon("main_document_current") );
        mact->setText( tr("Unset main", "act text") );
        mact->setToolTip( tr("Unset main document", "act toolTip") );
        mact->setWhatsThis( tr("Use this action to remove the \"Main\" flag from this document", "act whatsThis") );
    }
    else
    {
        mact->setIcon( Application::icon(mmainDocument ? "main_document_not_current" : "main_document_no") );
        mact->setText( tr("Set main", "act text") );
        mact->setToolTip( tr("Set current document as main", "act toolTip") );
        mact->setWhatsThis( tr("Use this action to mark current document as \"Main\"", "act whatsThis") );
    }
}

/*============================== Private slots =============================*/

void MainDocumentEditorModule::retranslateUi()
{
    resetAction();
}
