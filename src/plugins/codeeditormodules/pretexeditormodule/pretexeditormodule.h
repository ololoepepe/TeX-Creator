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
        ClearMacroAction,
        PlayAction,
        LoadAction,
        SaveAsAction,
        OpenUserMacrosDirAction,
        ClearMacroStackAction
    };
    enum Widget
    {
        MacrosEditorWidget
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
    bool eventFilter(QObject *o, QEvent *e);
    QByteArray saveState() const;
    void restoreState(const QByteArray &state);
    bool isPlaying() const;
    QObject *closeHandler() const;
    QObject *dropHandler() const;
public slots:
    void startStopRecording();
    void clearMacro();
    void playMacro(int n = 0);
    void playMacro5();
    void playMacro10();
    void playMacro20();
    void playMacro50();
    void playMacro100();
    void playMacroN();
    bool loadMacro(const QString &fileName = QString());
    bool saveMacroAs();
    void openUserDir();
    void reloadMacros();
protected:
    void editorSet(BCodeEditor *edr);
    void editorUnset(BCodeEditor *edr);
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
private:
    static QString fileDialogFilter();
private:
    void resetStartStopAction();
    void checkActions();
    void appendPtedtText(const QString &text);
    void setPtedtText(const QString &text);
    void clearPtedt();
private slots:
    void retranslateUi();
    void ptedtTextChanged();
    void lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void cedtrCurrentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void cedtrDocumentAboutToBeAdded(BAbstractCodeEditorDocument *doc);
    void cedtrDocumentAboutToBeRemoved(BAbstractCodeEditorDocument *doc);
    void cedtrCurrentDocumentFileNameChanged(const QString &fileName);
    void clearMacroStackSlot();
private:
    static QMap<QString, ExecutionStack *> mstacks;
    static QMap<QString, int> mstackRefs;
private:
    bool mplaying;
    bool mrecording;
    BAbstractCodeEditorDocument *mprevDoc;
    //
    QPointer<QAction> mactStartStop;
    QPointer<QAction> mactClearMacro;
    QPointer<QAction> mactClearStack;
    QPointer<QAction> mactPlay;
    QPointer<QAction> mactPlay5;
    QPointer<QAction> mactPlay10;
    QPointer<QAction> mactPlay20;
    QPointer<QAction> mactPlay50;
    QPointer<QAction> mactPlay100;
    QPointer<QAction> mactPlayN;
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
