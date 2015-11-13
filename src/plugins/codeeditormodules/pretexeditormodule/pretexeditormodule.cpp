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

#include "pretexeditormodule.h"

#include "pretexeditormoduleplugin.h"
#include "pretexfiletype.h"
#include "pretexobject.h"
#include "recordingmodule.h"

#include <BAbstractCodeEditorDocument>
#include <BAbstractDocumentDriver>
#include <BAbstractEditorModule>
#include <BAbstractFileType>
#include <BApplication>
#include <BCodeEdit>
#include <BCodeEditor>
#include <BDirTools>
#include <BeQt>
#include <BIndicatorsEditorModule>
#include <BLocalDocumentDriver>
#include <BOpenSaveEditorModule>
#include <BSignalDelayProxy>

#include <QAction>
#include <QByteArray>
#include <QDebug>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLayout>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QRegExp>
#include <QScriptContext>
#include <QScriptContextInfo>
#include <QScriptEngine>
#include <QScriptProgram>
#include <QScriptValue>
#include <QScriptValueList>
#include <QSet>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QString>
#include <QStringList>
#include <QTabBar>
#include <QToolBar>
#include <QVariant>
#include <QVariant>
#include <QVariantMap>
#include <QVBoxLayout>

#include <climits>

static QScriptValue requireForJS(QScriptContext *context, QScriptEngine *engine)
{
    QString requiredPath;
    bool found = false;
    QString param = context->argument(0).toString();
    if (param.split(".").last() != "js")
        param += ".js";
    QStringList paths = engine->globalObject().property("require").property("paths").toVariant().toStringList();
    QFileInfo requiredFileInfo(param);
    QScriptContextInfo contextInfo(context->parentContext());
    QString parentFileName(contextInfo.fileName());
    paths.prepend(QFileInfo(parentFileName).dir().path());
    foreach(QString includePath, paths) {
        QFileInfo includePathInfo(includePath);
        if (includePathInfo.exists()) {
            QDir includePathDir(includePath);
            includePathDir.cd(requiredFileInfo.dir().path());
            requiredPath = QDir::cleanPath(includePathDir.absoluteFilePath(requiredFileInfo.fileName()));
            if(QFileInfo(requiredPath).exists()) {
                found = true;
                break;
            }
        }
    }
    if (!found) {
        qDebug() << "require() file not found: " << param;
        return engine->undefinedValue();
    }
    QScriptValue modules = engine->globalObject().property("$MODULES");
    QScriptValue existedModule = modules.property(requiredPath);
    if (existedModule.isValid()) {
        return existedModule;
    } else {
        QFile requireFile(requiredPath);
        requireFile.open(QIODevice::ReadOnly);
        QScriptContext *newContext = engine->pushContext();
        QScriptValue moduleObject = engine->newObject();
        QScriptValue exportsObject = engine->newObject();
        moduleObject.setProperty("exports", exportsObject);
        newContext->activationObject().setProperty("module", moduleObject);
        newContext->activationObject().setProperty("exports", exportsObject);
        engine->evaluate(requireFile.readAll(), requiredPath);
        exportsObject = newContext->activationObject().property("exports");
        engine->popContext();
        requireFile.close();
        modules.setProperty(requiredPath, /*exportsObject*/moduleObject.property("exports"));
        return exportsObject;
    }
}

/*============================================================================
================================ SpontaneousEventEater =======================
============================================================================*/

class SpontaneousEventEater : public QObject
{
public:
    explicit SpontaneousEventEater(BAbstractCodeEditorDocument *doc);
public:
    bool eventFilter(QObject *o, QEvent *e);
};

/*============================================================================
================================ SpontaneousEventEater =======================
============================================================================*/

/*============================== Public constructors =======================*/

SpontaneousEventEater::SpontaneousEventEater(BAbstractCodeEditorDocument *doc)
{
    doc->installInnerEventFilter(this);
    doc->installInnerViewportEventFilter(this);
}

/*============================== Public methods ============================*/

bool SpontaneousEventEater::eventFilter(QObject *o, QEvent *e)
{
    typedef QList<int> IntList;
    init_once(IntList, mouseEvents, IntList()) {
        mouseEvents << QEvent::MouseButtonDblClick;
        mouseEvents << QEvent::MouseButtonPress;
        mouseEvents << QEvent::MouseButtonRelease;
        mouseEvents << QEvent::MouseMove;
        mouseEvents << QEvent::MouseTrackingChange;

    }
    bool viewport = o->findChildren<QWidget *>().isEmpty();
    if ((viewport && mouseEvents.contains(e->type())) || (!viewport && e->spontaneous())) {
        e->ignore();
        return true;
    } else {
        return false;
    }
}

/*============================================================================
================================ MacrosEditorModule ==========================
============================================================================*/

/*============================== Public constructors =======================*/

PretexEditorModule::PretexEditorModule(QObject *parent) :
    BAbstractEditorModule(parent)
{
#if defined(BUILTIN_RESOURCES)
    Q_INIT_RESOURCE(pretexeditormodule);
    Q_INIT_RESOURCE(pretexeditormodule_transtations);
#endif
    mrunning = false;
    mterminate = false;
    mlastN = 1;
    mrecModule = new RecordingModule(this);
    //
    mactStartStop = new QAction(this);
      connect(mactStartStop, SIGNAL(triggered()), this, SLOT(startStopRecording()));
    mactClear = new QAction(this);
      mactClear->setIcon(BApplication::icon("editclear"));
      connect(mactClear, SIGNAL(triggered()), this, SLOT(clear()));
    mactRun = new QAction(this);
      mactRun->setIcon(BApplication::icon("player_play"));
      connect(mactRun, SIGNAL(triggered()), this, SLOT(run()));
      connect(mactRun, SIGNAL(triggered()), this, SLOT(terminate()));
      QMenu *mnu = new QMenu;
        mactRun5 = new QAction(this);
          connect(mactRun5, SIGNAL(triggered()), this, SLOT(run5()));
        mnu->addAction(mactRun5);
        mactRun10 = new QAction(this);
          connect(mactRun10, SIGNAL(triggered()), this, SLOT(run10()));
        mnu->addAction(mactRun10);
        mactRun20 = new QAction(this);
          connect(mactRun20, SIGNAL(triggered()), this, SLOT(run20()));
        mnu->addAction(mactRun20);
        mactRun50 = new QAction(this);
          connect(mactRun50, SIGNAL(triggered()), this, SLOT(run50()));
        mnu->addAction(mactRun50);
        mactRun100 = new QAction(this);
          connect(mactRun100, SIGNAL(triggered()), this, SLOT(run100()));
        mnu->addAction(mactRun100);
        mactRunN = new QAction(this);
          connect(mactRunN, SIGNAL(triggered()), this, SLOT(runN()));
        mnu->addAction(mactRunN);
      mactRun->setMenu(mnu);
    mactLoad = new QAction(this);
      mactLoad->setIcon(BApplication::icon("fileopen"));
      connect(mactLoad, SIGNAL(triggered()), this, SLOT(load()));
    mactSaveAs = new QAction(this);
      mactSaveAs->setIcon(BApplication::icon("filesaveas"));
      connect(mactSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
    mactOpenDir = new QAction(this);
      mactOpenDir->setIcon(BApplication::icon("folder_open"));
      connect(mactOpenDir, SIGNAL(triggered()), this, SLOT(openUserDir()));
    mspltr = new QSplitter(Qt::Horizontal);
      QWidget *wgt = new QWidget;
        QVBoxLayout *vlt = new QVBoxLayout(wgt);
          QToolBar *tbar = new QToolBar;
          vlt->addWidget(tbar);
          mcedtr = new BCodeEditor(BCodeEditor::SimpleDocument);
            mcedtr->addFileType(new PreTeXFileType);
            mcedtr->setPreferredFileType(mcedtr->fileType("JavaScript"));
            mcedtr->setMaximumFileSize(5 * BeQt::Megabyte);
            QAction *act = mcedtr->module(BCodeEditor::OpenSaveModule)->action(BOpenSaveEditorModule::NewFileAction);
            act->setShortcut(QKeySequence());
            tbar->addAction(act);
            tbar->addAction(mactLoad);
            tbar->addSeparator();
            act = mcedtr->module(BCodeEditor::OpenSaveModule)->action(BOpenSaveEditorModule::SaveFileAction);
            act->setShortcut(QKeySequence());
            tbar->addAction(act);
            tbar->addAction(mactSaveAs);
            tbar->addSeparator();
            tbar->addAction(mactOpenDir);
            tbar->addSeparator();
            tbar->addAction(mactStartStop);
            tbar->addAction(mactRun);
            tbar->addAction(mactClear);
            qobject_cast<BLocalDocumentDriver *>(mcedtr->driver())->setDefaultDir(
                        BDirTools::findResource("pretex", BDirTools::UserOnly));
            connect(mcedtr, SIGNAL(currentDocumentChanged(BAbstractCodeEditorDocument *)),
                    this, SLOT(cedtrCurrentDocumentChanged(BAbstractCodeEditorDocument *)));
            connect(mcedtr, SIGNAL(documentAboutToBeAdded(BAbstractCodeEditorDocument *)),
                    this, SLOT(cedtrDocumentAboutToBeAdded(BAbstractCodeEditorDocument *)));
            connect(mcedtr, SIGNAL(documentAboutToBeRemoved(BAbstractCodeEditorDocument *)),
                    this, SLOT(cedtrDocumentAboutToBeRemoved(BAbstractCodeEditorDocument *)));
            connect(mcedtr, SIGNAL(currentDocumentFileNameChanged(QString)),
                    this, SLOT(cedtrCurrentDocumentFileNameChanged(QString)));
          vlt->addWidget(mcedtr);
          mstbar = new QStatusBar;
            mstbar->setSizeGripEnabled(false);
            BAbstractEditorModule *mdl = mcedtr->module(BCodeEditor::IndicatorsModule);
            mstbar->addPermanentWidget(mdl->widget(BIndicatorsEditorModule::FileTypeIndicator));
            mstbar->addPermanentWidget(mdl->widget(BIndicatorsEditorModule::CursorPositionIndicator));
            mstbar->addPermanentWidget(mdl->widget(BIndicatorsEditorModule::EncodingIndicator));
          vlt->addWidget(mstbar);
      mspltr->addWidget(wgt);
      mlstwgt = new QListWidget;
        connect(mlstwgt, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
                this, SLOT(lstwgtCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));
      mspltr->addWidget(mlstwgt);
    //
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
    retranslateUi();
    reload();
}
PretexEditorModule::~PretexEditorModule()
{
    if (!mspltr.isNull())
        delete mspltr;
#if defined(BUILTIN_RESOURCES)
    Q_CLEANUP_RESOURCE(pretexeditormodule);
    Q_CLEANUP_RESOURCE(pretexeditormodule_transtations);
#endif
}

/*============================== Public methods ============================*/

QAction *PretexEditorModule::action(int type)
{
    switch (type) {
    case StartStopRecordingAction:
        return mactStartStop;
    case ClearAction:
        return mactClear;
    case RunAction:
        return mactRun;
    case LoadAction:
        return mactLoad;
    case SaveAsAction:
        return mactSaveAs;
    case OpenUserDirAction:
        return mactOpenDir;
    default:
        return 0;
    }
}

QList<QAction *> PretexEditorModule::actions(bool extended)
{
    QList<QAction *> list;
    list << action(StartStopRecordingAction);
    list << action(RunAction);
    list << action(ClearAction);
    if (extended) {
        list << action(LoadAction);
        list << action(SaveAsAction);
        list << action(OpenUserDirAction);
    }
    list.removeAll(0);
    return list;
}

QObject *PretexEditorModule::closeHandler() const
{
    return !mcedtr.isNull() ? mcedtr->closeHandler() : 0;
}

QObject *PretexEditorModule::dropHandler() const
{
    return !mcedtr.isNull() ? mcedtr->dropHandler() : 0;
}

QString PretexEditorModule::id() const
{
    return "plugin/pretex";
}

bool PretexEditorModule::isRunning() const
{
    return mrunning;
}

void PretexEditorModule::restoreState(const QByteArray &state)
{
    QVariantMap m = BeQt::deserialize(state).toMap();
    if (!mspltr.isNull())
        mspltr->restoreState(m.value("splitter_state").toByteArray());
    bool ok = false;
    int n = m.value("last_n", 1).toInt(&ok);
    if (ok && n > 0)
        mlastN = n;
}

QByteArray PretexEditorModule::saveState() const
{
    QVariantMap m;
    m.insert("splitter_state", !mspltr.isNull() ? mspltr->saveState() : QByteArray());
    m.insert("last_n", mlastN);
    return BeQt::serialize(m);
}

bool PretexEditorModule::shouldTerminate() const
{
    return mterminate;
}

QWidget *PretexEditorModule::widget(int type)
{
    switch (type) {
    case PretexEditorWidget:
        return mspltr;
    default:
        return 0;
    }
}

/*============================== Public slots ==============================*/

void PretexEditorModule::clear()
{
    if (mrunning)
        return;
    if (!mcedtr.isNull() && mcedtr->currentDocument())
        mcedtr->currentDocument()->setText("");
    checkActions();
}

bool PretexEditorModule::load(const QString &fileName)
{
    if (mrunning || mrecModule->isRecording() || mcedtr.isNull())
        return false;
    return !fileName.isEmpty() ? (bool) mcedtr->openDocument(fileName) : !mcedtr->openDocuments().isEmpty();
}

void PretexEditorModule::openUserDir()
{
    bApp->openLocalFile(BDirTools::findResource("pretex"));
}

void PretexEditorModule::reload()
{
    if (mcedtr.isNull())
        return;
    mcedtr->closeAllDocuments();
    foreach (const QString &path, BApplication::locations("pretex")) {
        foreach (const QString &fn, BDirTools::entryList(path, QStringList() << "*.js", QDir::Files))
            mcedtr->openDocument(fn);
    }
}

void PretexEditorModule::run(int n)
{
    if (n <= 0)
        n = 1;
    BAbstractCodeEditorDocument *doc = currentDocument();
    if (!doc || mrunning || mrecModule->isRecording() || mcedtr.isNull())
        return;
    mterminate = false;
    BAbstractCodeEditorDocument *pdoc = !mcedtr.isNull() ? mcedtr->currentDocument() : 0;
    if (!pdoc)
        return;
    if (!mstbar.isNull())
        mstbar->clearMessage();
    editor()->findChild<QTabBar *>()->setEnabled(false);
    mrunning = true;
    checkActions();
    resetStartStopRecordingAction();
    QScriptEngine engine;
    engine.setProcessEventsInterval(500);
    QScriptValue modules = engine.newObject();
    engine.globalObject().setProperty("$MODULES", modules);
    QScriptValue require = engine.newFunction(requireForJS);
    engine.globalObject().setProperty("require", require);
    QScriptValue paths = engine.newArray();
    QScriptValue push = paths.property("push");
    push.call(paths, QScriptValueList() << QScriptValue(":/pretexeditormodule/lib"));
    require.setProperty("paths", paths);
    PretexObject pretexQt(doc);
    QScriptValue pretex = engine.newQObject(&pretexQt);
    engine.globalObject().setProperty("Pretex", pretex, QScriptValue::ReadOnly | QScriptValue::Undeletable
                                      | QScriptValue::SkipInEnumeration);
    QScriptProgram prog(pdoc->text(true), pdoc->fileName());
    SpontaneousEventEater eater(doc);
    Q_UNUSED(eater)
    for (int i = 0; i < n; ++i) {
        if (mterminate)
            break;
        QScriptValue val = engine.evaluate(prog);
        if (engine.hasUncaughtException()) {
            mrunning = false;
            checkActions();
            resetStartStopRecordingAction();
            editor()->findChild<QTabBar *>()->setEnabled(true);
            mcedtr->setCurrentDocument(pdoc);
            int line = engine.uncaughtExceptionLineNumber() - 1;
            pdoc->selectLines(line, line);
            QString err = engine.uncaughtException().toString() + "\n" + engine.uncaughtExceptionBacktrace().join("\n");
            showErrorMessage(doc, err, line, pdoc->fileName());
            return;
        }
    }
    mrunning = false;
    checkActions();
    resetStartStopRecordingAction();
    if (!editor())
        return;
    editor()->findChild<QTabBar *>()->setEnabled(true);
}

void PretexEditorModule::run5()
{
    run(5);
}

void PretexEditorModule::run10()
{
    run(10);
}

void PretexEditorModule::run20()
{
    run(20);
}

void PretexEditorModule::run50()
{
    run(50);
}

void PretexEditorModule::run100()
{
    run(100);
}

void PretexEditorModule::runN()
{
    bool ok = false;
    int n = QInputDialog::getInt(editor(), tr("Enter a number", "idlg title"), tr("Number of iterations:", "lbl text"),
                                 mlastN, 1, INT_MAX, 1, &ok);
    if (!ok)
        return;
    mlastN = n;
    run(n);
}

bool PretexEditorModule::saveAs()
{
    if (mrecModule->isRecording() || mcedtr.isNull() || !mcedtr->currentDocument())
        return false;
    return mcedtr->saveCurrentDocumentAs();
}

void PretexEditorModule::startStopRecording()
{
    if (mrunning)
        return;
    if (mrecModule->isRecording()) {
        mrecModule->stopRecording();
        editor()->findChild<QTabBar *>()->setEnabled(true);
        BAbstractCodeEditorDocument *doc = !mcedtr.isNull() ? mcedtr->addDocument() : 0;
        if (doc) {
            doc->setText(mrecModule->commands().join("\n"));
            doc->setModification(true);
        }
    } else {
        editor()->findChild<QTabBar *>()->setEnabled(false);
        mrecModule->setDocument(currentDocument());
        mrecModule->startRecording();
    }
    resetStartStopRecordingAction();
    checkActions();
}

void PretexEditorModule::terminate()
{
    if (!mrunning || mterminate)
        return;
    mterminate = true;
    resetStartStopRunningAction();
    emit terminated();
}

/*============================== Protected methods =========================*/

void PretexEditorModule::editorSet(BCodeEditor *)
{
    resetStartStopRecordingAction();
    checkActions();
}

void PretexEditorModule::editorUnset(BCodeEditor *)
{
    if (!mspltr.isNull()) {
        mspltr->setParent(0);
        mspltr->hide();
    }
    terminate();
    resetStartStopRecordingAction();
    checkActions();
}

void PretexEditorModule::currentDocumentChanged(BAbstractCodeEditorDocument *)
{
    resetStartStopRecordingAction();
    checkActions();
}

/*============================== Static private methods ====================*/

QListWidgetItem *PretexEditorModule::findItemByFileName(QListWidget *lwgt, const QString &fn)
{
    if (!lwgt)
        return 0;
    for (int i = 0; i < lwgt->count(); ++i) {
        if (lwgt->item(i)->data(Qt::ToolTipRole).toString() == fn)
            return lwgt->item(i);
    }
    return 0;
}

void PretexEditorModule::showErrorMessage(BAbstractCodeEditorDocument *doc, const QString &err, int pos,
                                          const QString &fn)
{
    qDebug() << "Error" << err << "at position" << QString::number(pos + 1) << fn;
    return;
    QMessageBox msgbox(doc);
    msgbox.setWindowTitle(tr("Error", "msgbox windowTitle"));
    msgbox.setIcon(QMessageBox::Critical);
    msgbox.setStandardButtons(QMessageBox::Ok);
    msgbox.setText(tr("Error:", "msgbox text") + " " + err + " " + tr("at position", "msgbox text") + " "
                   + QString::number(pos + 1));
    if (!fn.isEmpty())
        msgbox.setInformativeText(tr("File:", "magbox informativeText") + " " + fn);
    msgbox.exec();
}

/*============================== Private methods ===========================*/

void PretexEditorModule::checkActions()
{
    if (!mactClear.isNull())
        mactClear->setEnabled(!mrunning);
    resetStartStopRunningAction();
    if (!mactSaveAs.isNull())
        mactSaveAs->setEnabled(!mrecModule->isRecording());
}

void PretexEditorModule::resetStartStopRecordingAction()
{
    if (mactStartStop.isNull())
        return;
    mactStartStop->setEnabled(currentDocument() && !mrunning);
    if (mrecModule->isRecording()) {
        mactStartStop->setIcon(BApplication::icon("player_stop"));
        mactStartStop->setText(tr("Stop recording", "act text"));
        mactStartStop->setToolTip(tr("Stop recording commands", "act toolTip"));
        mactStartStop->setWhatsThis(tr("Use this action to finish recording commands", "act whatsThis"));
    }
    else {
        mactStartStop->setIcon(BApplication::icon("player_record"));
        mactStartStop->setText(tr("Start recording", "act text"));
        mactStartStop->setToolTip(tr("Start recording commands", "act toolTip"));
        mactStartStop->setWhatsThis(tr("Use this action to begin recording commands", "act whatsThis"));
    }
}

void PretexEditorModule::resetStartStopRunningAction()
{
    if (mactRun.isNull())
        return;
    if (mrunning) {
        if (mterminate)
            mactRun->setEnabled(false);
        else
            mactRun->setEnabled(true);
    } else {
        mactRun->setEnabled(currentDocument() && !mrecModule->isRecording());
    }
    if (!mrunning || mterminate) {
        mactRun->setIcon(BApplication::icon("player_play"));
        mactRun->setText(tr("Run", "act text"));
        mactRun->setToolTip(tr("Run current document", "act toolTip"));
        mactRun->setWhatsThis(tr("Use this action to activate previously loaded or recorded file", "act whatsThis"));
    } else {
        mactRun->setIcon(BApplication::icon("player_stop"));
        mactRun->setText(tr("Stop", "act text"));
        mactRun->setToolTip(tr("Stop execution", "act toolTip"));
        mactRun->setWhatsThis(tr("Use this action to stop execution of a PreTeX program", "act whatsThis"));
    }
}

/*============================== Private slots =============================*/

void PretexEditorModule::cedtrCurrentDocumentChanged(BAbstractCodeEditorDocument *doc)
{
    if (mlstwgt.isNull())
        return;
    mlstwgt->setCurrentItem(findItemByFileName(mlstwgt, doc ? doc->fileName() : QString()));
}

void PretexEditorModule::cedtrCurrentDocumentFileNameChanged(const QString &fileName)
{
    if (mlstwgt.isNull())
        return;
    QListWidgetItem *lwi = mlstwgt->currentItem();
    if (!lwi)
        return;
    lwi->setText(QFileInfo(fileName).baseName());
    lwi->setData(Qt::ToolTipRole, fileName);
}

void PretexEditorModule::cedtrDocumentAboutToBeAdded(BAbstractCodeEditorDocument *doc)
{
    if (!doc || mlstwgt.isNull())
        return;
    QFileInfo fi(doc->fileName());
    QListWidgetItem *lwi = new QListWidgetItem(fi.baseName());
    lwi->setData(Qt::ToolTipRole, fi.filePath());
    mlstwgt->addItem(lwi);
}

void PretexEditorModule::cedtrDocumentAboutToBeRemoved(BAbstractCodeEditorDocument *doc)
{
    if (!doc || mlstwgt.isNull())
        return;
    delete findItemByFileName(mlstwgt, doc->fileName());
}

void PretexEditorModule::lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *)
{
    if (mcedtr.isNull())
        return;
    if (current) {
        foreach (BAbstractCodeEditorDocument *doc, mcedtr->documents()) {
            if (doc->fileName() == current->data(Qt::ToolTipRole).toString()) {
                mcedtr->setCurrentDocument(doc);
                break;
            }
        }
    } else {
        mcedtr->setCurrentDocument(0);
    }
}

void PretexEditorModule::retranslateUi()
{
    if (!mactClear.isNull()) {
        mactClear->setText(tr("Clear document", "act text"));
        mactClear->setToolTip(tr("Clear current document", "act toolTip"));
        mactClear->setWhatsThis(tr("Use this action to clear currently loaded or recorded document. "
                                   "The corresponding file will not be deleted", "act whatsThis"));
    }
    if (!mactRun5.isNull())
        mactRun5->setText(tr("Run 5 times", "act text"));
    if (!mactRun10.isNull())
        mactRun10->setText(tr("Run 10 times", "act text"));
    if (!mactRun20.isNull())
        mactRun20->setText(tr("Run 20 times", "act text"));
    if (!mactRun50.isNull())
        mactRun50->setText(tr("Run 50 times", "act text"));
    if (!mactRun100.isNull())
        mactRun100->setText(tr("Run 100 times", "act text"));
    if (!mactRunN.isNull())
        mactRunN->setText(tr("Run N times", "act text"));
    if (!mactLoad.isNull()) {
        mactLoad->setText(tr("Load...", "act text"));
        mactLoad->setToolTip(tr("Load file", "act toolTip"));
        mactLoad->setWhatsThis(tr("Use this action to load previously saved file", "act whatsThis"));
    }
    if (!mactSaveAs.isNull()) {
        mactSaveAs->setText(tr("Save as...", "act text"));
        mactSaveAs->setToolTip(tr("Save current document as...", "act toolTip"));
        mactSaveAs->setWhatsThis(tr("Use this action to save current document", "act whatsThis"));
    }
    if (!mactOpenDir.isNull()) {
        mactOpenDir->setText(tr("Open user PreTeX directory", "act text"));
        mactOpenDir->setWhatsThis(tr("Use this action to open PreTeX user directory", "act whatsThis"));
    }
    if (!mcedtr.isNull())
        mcedtr->setDefaultFileName(tr("New document.js", "default document file name"));
    resetStartStopRecordingAction();
}
