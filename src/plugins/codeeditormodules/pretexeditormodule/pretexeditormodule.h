/****************************************************************************
**
** Copyright (C) 2014 TeXSample Team
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

#ifndef PRETEXEDITORMODULE_H
#define PRETEXEDITORMODULE_H

class ExecutionStack;
class RecordingModule;

class BAbstractCodeEditorDocument;

class QEvent;
class QListWidgetItem;
class QByteArray;
class QWidget;

#include <BAbstractEditorModule>
#include <BCodeEditor>

#include <QObject>
#include <QList>
#include <QPointer>
#include <QAction>
#include <QString>
#include <QSplitter>
#include <QListWidget>
#include <QStatusBar>
#include <QMap>

/*============================================================================
================================ PretexEditorModule ==========================
============================================================================*/

class PretexEditorModule : public BAbstractEditorModule
{
    Q_OBJECT
public:
    enum Action
    {
        StartStopRecordingAction,
        ClearAction,
        RunAction,
        LoadAction,
        SaveAsAction,
        OpenUserDirAction,
        ClearStackAction
    };
    enum Widget
    {
        PretexEditorWidget
    };
public:
    static ExecutionStack *executionStack(PretexEditorModule *module = 0);
public:
    explicit PretexEditorModule(QObject *parent = 0);
    ~PretexEditorModule();
public:
    QString id() const;
    QAction *action(int type);
    QList<QAction *> actions(bool extended = false);
    QWidget *widget(int type);
    QByteArray saveState() const;
    void restoreState(const QByteArray &state);
    bool isRunning() const;
    QObject *closeHandler() const;
    QObject *dropHandler() const;
public slots:
    void startStopRecording();
    void clear();
    void run(int n = 0);
    void run5();
    void run10();
    void run20();
    void run50();
    void run100();
    void runN();
    bool load(const QString &fileName = QString());
    bool saveAs();
    void openUserDir();
    void reload();
protected:
    void editorSet(BCodeEditor *edr);
    void editorUnset(BCodeEditor *edr);
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
private:
    static void showErrorMessage(BAbstractCodeEditorDocument *doc, const QString &err, int pos,
                                 const QString &fn = QString());
private:
    void resetStartStopAction();
    void checkActions();
private slots:
    void retranslateUi();
    void lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void cedtrCurrentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void cedtrDocumentAboutToBeAdded(BAbstractCodeEditorDocument *doc);
    void cedtrDocumentAboutToBeRemoved(BAbstractCodeEditorDocument *doc);
    void cedtrCurrentDocumentFileNameChanged(const QString &fileName);
    void clearStackSlot();
private:
    static QMap<QString, ExecutionStack *> mstacks;
    static QMap<QString, int> mstackRefs;
private:
    bool mrunning;
    RecordingModule *mrecModule;
    //
    QPointer<QAction> mactStartStop;
    QPointer<QAction> mactClear;
    QPointer<QAction> mactClearStack;
    QPointer<QAction> mactRun;
    QPointer<QAction> mactRun5;
    QPointer<QAction> mactRun10;
    QPointer<QAction> mactRun20;
    QPointer<QAction> mactRun50;
    QPointer<QAction> mactRun100;
    QPointer<QAction> mactRunN;
    QPointer<QAction> mactLoad;
    QPointer<QAction> mactSaveAs;
    QPointer<QAction> mactOpenDir;
    QPointer<BCodeEditor> mcedtr;
    QPointer<QStatusBar> mstbar;
    QPointer<QListWidget> mlstwgt;
    QPointer<QSplitter> mspltr;
    int mlastN;
private:
    Q_DISABLE_COPY(PretexEditorModule)
};

#endif // PRETEXEDITORMODULE_H
