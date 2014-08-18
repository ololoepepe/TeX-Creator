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

class BAbstractCodeEditorDocument;
class BCodeEditor;

class QString;

#include <BAbstractEditorModule>

#include <QAction>
#include <QList>
#include <QObject>
#include <QPointer>

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
private:
    QPointer<QAction> mact;
    BAbstractCodeEditorDocument *mmainDocument;
public:
    explicit MainDocumentEditorModule(QObject *parent = 0);
    ~MainDocumentEditorModule();
public:
    QAction *action(int type);
    QList<QAction *> actions(bool extended = false);
    QString id() const;
    bool isCurrentDocumentMain() const;
    BAbstractCodeEditorDocument *mainDocument() const;
    QString mainDocumentFileName() const;
    void setMainDocument(BAbstractCodeEditorDocument *doc);
public slots:
    void switchCurrentDocumentMain();
protected:
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void editorSet(BCodeEditor *edr);
    void editorUnset(BCodeEditor *edr);
private:
    void resetAction();
private slots:
    void retranslateUi();
signals:
    void mainDocumentChanged(BAbstractCodeEditorDocument *doc);
private:
    Q_DISABLE_COPY(MainDocumentEditorModule)
};

#endif // MAINDOCUMENTEDITORMODULE_H
