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

#ifndef PRETEXEDITORMODULE_H
#define PRETEXEDITORMODULE_H

class RecordingModule;

class BAbstractCodeEditorDocument;

class QByteArray;
class QEvent;
class QListWidgetItem;
class QWidget;

#include <BAbstractEditorModule>
#include <BCodeEditor>

#include <QAction>
#include <QList>
#include <QListWidget>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSplitter>
#include <QStatusBar>
#include <QString>

/*============================================================================
================================ PretexEditorModule ==========================
============================================================================*/

class PretexEditorModule : public BAbstractEditorModule
{
    Q_OBJECT
public:
    enum Action
    {
        ClearAction,
        LoadAction,
        OpenUserDirAction,
        RunAction,
        SaveAsAction,
        StartStopRecordingAction
    };
    enum Widget
    {
        PretexEditorWidget
    };
private:
    int mlastN;
    RecordingModule *mrecModule;
    bool mrunning;
    bool mterminate;
    //
    QPointer<QAction> mactClear;
    QPointer<QAction> mactLoad;
    QPointer<QAction> mactOpenDir;
    QPointer<QAction> mactRun;
    QPointer<QAction> mactRun10;
    QPointer<QAction> mactRun100;
    QPointer<QAction> mactRun20;
    QPointer<QAction> mactRun5;
    QPointer<QAction> mactRun50;
    QPointer<QAction> mactRunN;
    QPointer<QAction> mactSaveAs;
    QPointer<QAction> mactStartStop;
    QPointer<BCodeEditor> mcedtr;
    QPointer<QListWidget> mlstwgt;
    QPointer<QSplitter> mspltr;
    QPointer<QStatusBar> mstbar;
public:
    explicit PretexEditorModule(QObject *parent = 0);
    ~PretexEditorModule();
public:
    QAction *action(int type);
    QList<QAction *> actions(bool extended = false);
    QObject *closeHandler() const;
    QObject *dropHandler() const;
    QString id() const;
    bool isRunning() const;
    void restoreState(const QByteArray &state);
    QByteArray saveState() const;
    bool shouldTerminate() const;
    QWidget *widget(int type);
public slots:
    void clear();
    bool load(const QString &fileName = QString());
    void openUserDir();
    void reload();
    void run(int n = 0);
    void run5();
    void run10();
    void run20();
    void run50();
    void run100();
    void runN();
    bool saveAs();
    void startStopRecording();
    void terminate();
protected:
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void editorSet(BCodeEditor *edr);
    void editorUnset(BCodeEditor *edr);
signals:
    void terminated();
private:
    static QListWidgetItem *findItemByFileName(QListWidget *lwgt, const QString &fn);
    static void showErrorMessage(BAbstractCodeEditorDocument *doc, const QString &err, int pos,
                                 const QString &fn = QString());
private:
    void checkActions();
    void resetStartStopRecordingAction();
    void resetStartStopRunningAction();
private slots:
    void cedtrCurrentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void cedtrCurrentDocumentFileNameChanged(const QString &fileName);
    void cedtrDocumentAboutToBeAdded(BAbstractCodeEditorDocument *doc);
    void cedtrDocumentAboutToBeRemoved(BAbstractCodeEditorDocument *doc);
    void lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void retranslateUi();
private:
    Q_DISABLE_COPY(PretexEditorModule)
};

#endif // PRETEXEDITORMODULE_H
