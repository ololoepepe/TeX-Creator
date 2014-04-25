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

#ifndef MAINDOCUMENTEDITORMODULE_H
#define MAINDOCUMENTEDITORMODULE_H

class BCodeEditor;
class BAbstractCodeEditorDocument;

class QString;

#include <BAbstractEditorModule>

#include <QObject>
#include <QList>
#include <QPointer>
#include <QAction>

/*============================================================================
================================ MainDocumentEditorModule ====================
============================================================================*/

class MainDocumentEditorModule : public BAbstractEditorModule
{
    Q_OBJECT
public:
    enum Action
    {
        SwitchCurrentDocumentMainAction
    };
public:
    explicit MainDocumentEditorModule(QObject *parent = 0);
    ~MainDocumentEditorModule();
public:
    QString id() const;
    QAction *action(int type);
    QList<QAction *> actions(bool extended = false);
    void setMainDocument(BAbstractCodeEditorDocument *doc);
    bool isCurrentDocumentMain() const;
    BAbstractCodeEditorDocument *mainDocument() const;
    QString mainDocumentFileName() const;
public slots:
    void switchCurrentDocumentMain();
protected:
    void editorSet(BCodeEditor *edr);
    void editorUnset(BCodeEditor *edr);
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
private:
    void resetAction();
private slots:
    void retranslateUi();
signals:
    void mainDocumentChanged(BAbstractCodeEditorDocument *doc);
private:
    BAbstractCodeEditorDocument *mmainDocument;
    QPointer<QAction> mact;
private:
    Q_DISABLE_COPY(MainDocumentEditorModule)
};

#endif // MAINDOCUMENTEDITORMODULE_H
