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

#include "pretexeditormodule.h"
#include "executionstack.h"
#include "pretexeditormoduleplugin.h"
#include "lexicalanalyzer.h"
#include "global.h"
#include "executionmodule.h"
#include "parser.h"
#include "token.h"
#include "pretexfunction.h"
#include "recordingmodule.h"

#include <BAbstractEditorModule>
#include <BCodeEditor>
#include <BAbstractCodeEditorDocument>
#include <BDirTools>
#include <BeQt>
#include <BAbstractFileType>
#include <BSignalDelayProxy>
#include <BAbstractDocumentDriver>
#include <BLocalDocumentDriver>
#include <BOpenSaveEditorModule>
#include <BIndicatorsEditorModule>
#include <BApplication>
#include <BCodeEdit>

#include <QObject>
#include <QList>
#include <QString>
#include <QAction>
#include <QVariant>
#include <QPointer>
#include <QIcon>
#include <QFileDialog>
#include <QApplication>
#include <QKeyEvent>
#include <QEvent>
#include <QStringList>
#include <QVBoxLayout>
#include <QLayout>
#include <QKeySequence>
#include <QVariant>
#include <QRegExp>
#include <QTextCursor>
#include <QTextBlock>
#include <QFileInfo>
#include <QProcess>
#include <QInputDialog>
#include <QMenu>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QColor>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSet>
#include <QVariantMap>
#include <QByteArray>
#include <QToolBar>
#include <QStatusBar>
#include <QSettings>
#include <QTabBar>
#include <QMessageBox>

#include <QDebug>

#include <climits>

/*============================================================================
================================ SpontaneousEventEater =======================
============================================================================*/

class SpontaneousEventEater : public QObject
{
public:
    explicit SpontaneousEventEater(BAbstractCodeEditorDocument *doc);
public:
    bool eventFilter(QObject *o, QEvent *e);
private:
    QPlainTextEdit *mptedt;
};

/*============================================================================
================================ SpontaneousEventEater =======================
============================================================================*/

/*============================== Public constructors =======================*/

SpontaneousEventEater::SpontaneousEventEater(BAbstractCodeEditorDocument *doc)
{
    mptedt = doc ? doc->findChild<QPlainTextEdit *>() : 0;
    if (!mptedt)
        return;
    mptedt->installEventFilter(this);
    mptedt->viewport()->installEventFilter(this);
}

/*============================== Public methods ============================*/

bool SpontaneousEventEater::eventFilter(QObject *o, QEvent *e)
{
    typedef QList<int> IntList;
    init_once(IntList, mouseEvents, IntList())
    {
        mouseEvents << QEvent::MouseButtonDblClick;
        mouseEvents << QEvent::MouseButtonPress;
        mouseEvents << QEvent::MouseButtonRelease;
        mouseEvents << QEvent::MouseMove;
        mouseEvents << QEvent::MouseTrackingChange;

    }
    if ((o != mptedt && mouseEvents.contains(e->type())) || (o == mptedt && e->spontaneous()))
    {
        e->ignore();
        return true;
    }
    else
    {
        return false;
    }
}

/*============================================================================
================================ PreTeXFileType ==============================
============================================================================*/

class PreTeXFileType : public BAbstractFileType
{
    Q_DECLARE_TR_FUNCTIONS(PreTeXFileType)
public:
    PreTeXFileType();
    ~PreTeXFileType();
public:
    QString id() const;
    QString name() const;
    QString description() const;
    QStringList suffixes() const;
    bool matchesFileName(const QString &fileName) const;
    BracketPairList brackets() const;
protected:
    void highlightBlock(const QString &text);
private:
    Q_DISABLE_COPY(PreTeXFileType)
};

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static QListWidgetItem *findItemByFileName(QListWidget *lwgt, const QString &fn)
{
    if (!lwgt)
        return 0;
    for (int i = 0; i < lwgt->count(); ++i)
        if (lwgt->item(i)->data(Qt::ToolTipRole).toString() == fn)
            return lwgt->item(i);
    return 0;
}

/*============================================================================
================================ PreTeXFileType ==============================
============================================================================*/

/*============================== Public constructors =======================*/

PreTeXFileType::PreTeXFileType()
{
    //
}

PreTeXFileType::~PreTeXFileType()
{
    //
}

/*============================== Public methods ============================*/

QString PreTeXFileType::id() const
{
    return "PreTeX";
}

QString PreTeXFileType::name() const
{
    return tr("PreTeX", "name");
}

QString PreTeXFileType::description() const
{
    return tr("PreTeX files", "description");
}

QStringList PreTeXFileType::suffixes() const
{
    return QStringList() << "pretex";
}

bool PreTeXFileType::matchesFileName(const QString &fileName) const
{
    return suffixes().contains(QFileInfo(fileName).suffix(), Qt::CaseInsensitive);
}

BAbstractFileType::BracketPairList PreTeXFileType::brackets() const
{
    BracketPairList list;
    list << createBracketPair("{", "}", "\\");
    list << createBracketPair("[", "]", "\\");
    return list;
}

/*============================== Protected methods =========================*/

void PreTeXFileType::highlightBlock(const QString &text)
{
    int i = 0;
    int lastBSPos = -1;
    setCurrentBlockState(0);
    clearCurrentBlockSkipSegments();
    int lastState = previousBlockState();
    if (1 == lastState)
    {
        bool matched = false;
        while (i < text.length())
        {
            if (text.at(i) == '%' && !LexicalAnalyzer::isEscaped(text, i, '%')
                    && text.length() > i + 1 && text.at(i + 1) == '%')
            {
                matched = true;
                break;
            }
            ++i;
        }
        if (matched)
        {
            i += 2;
            lastState = 0;
            setFormat(0, i, QColor(Qt::darkGray));
            addCurrentBlockSkipSegmentL(0, i);
        }
        else
        {
            setFormat(0, text.length(), QColor(Qt::darkGray));
            addCurrentBlockSkipSegment(0);
            lastState = 1;
        }
    }
    while (i < text.length())
    {
        int ml = 0;
        bool builtin = false;
        bool matchedBS = false;
        QString s = text.mid(i);
        if (s.at(0) == '%' && !LexicalAnalyzer::isEscaped(text, i, '%'))
        {
            if (s.length() > 1 && s.at(1) == '%')
            {
                if (lastState == 1)
                {
                    setFormat(0, i + 1, QColor(Qt::darkGray));
                    addCurrentBlockSkipSegmentL(0, i + 1);
                    lastState = 0;
                }
                else
                {
                    int j = 2;
                    bool matched = false;
                    while (j < s.length())
                    {
                        if (s.at(j) == '%' && !LexicalAnalyzer::isEscaped(s, j, '%')
                                && s.length() > j + 1 && s.at(j + 1) == '%')
                        {
                            matched = true;
                            break;
                        }
                        ++j;
                    }
                    if (matched)
                    {
                        lastState = 0;
                        setFormat(i, j + 2, QColor(Qt::darkGray));
                        addCurrentBlockSkipSegmentL(i, j + 2);
                        i += j + 1;
                    }
                    else
                    {
                        setFormat(i, text.length() - i, QColor(Qt::darkGray));
                        addCurrentBlockSkipSegment(i);
                        lastState = 1;
                        break;
                    }
                }
            }
            else if (lastState != 1)
            {
                setFormat(i, text.length() - i, QColor(Qt::darkGray));
                addCurrentBlockSkipSegment(i);
                break;
            }
        }
        else if (LexicalAnalyzer::matchString(s, ml))
        {
            setFormat(i, ml, QColor(51, 132, 43));
        }
        else if (LexicalAnalyzer::matchSpecFuncName(s, ml))
        {
            if (lastBSPos >= 0)
                setFormat(lastBSPos, 1, QColor(180, 140, 30));
            setFormat(i, ml, QColor(180, 140, 30));
        }
        else if (LexicalAnalyzer::matchFuncName(s, ml, &builtin))
        {
            if (builtin && lastBSPos >= 0)
                setFormat(lastBSPos, 1, QColor(180, 140, 30));
            setFormat(i, ml, builtin ? QColor(180, 140, 30) : QColor(Qt::red).lighter(70));
        }
        else if (LexicalAnalyzer::matchReal(s, ml) || LexicalAnalyzer::matchInteger(s, ml))
        {
            setFormat(i, ml, QColor(0, 0, 136));
        }
        else if (s.at(0) == '#')
        {
            setFormat(i, 1, QColor(51, 132, 43));
        }
        else if (s.at(0) == '\\')
        {
            matchedBS = true;
            setFormat(i, 1, QColor(Qt::red).lighter(70));
        }
        lastBSPos = matchedBS ? i : -1;
        i += ml ? ml : 1;
    }
    setCurrentBlockState(lastState);
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
    mlastN = 1;
    mrecModule = new RecordingModule(this);
    //
    mactStartStop = new QAction(this);
      connect(mactStartStop.data(), SIGNAL(triggered()), this, SLOT(startStopRecording()));
    mactClear = new QAction(this);
      mactClear->setIcon(BApplication::icon("editclear"));
      connect(mactClear.data(), SIGNAL(triggered()), this, SLOT(clear()));
    mactRun = new QAction(this);
      mactRun->setIcon(BApplication::icon("player_play"));
      connect(mactRun.data(), SIGNAL(triggered()), this, SLOT(run()));
      QMenu *mnu = new QMenu;
        mactRun5 = new QAction(this);
          connect(mactRun5.data(), SIGNAL(triggered()), this, SLOT(run5()));
        mnu->addAction(mactRun5.data());
        mactRun10 = new QAction(this);
          connect(mactRun10.data(), SIGNAL(triggered()), this, SLOT(run10()));
        mnu->addAction(mactRun10.data());
        mactRun20 = new QAction(this);
          connect(mactRun20.data(), SIGNAL(triggered()), this, SLOT(run20()));
        mnu->addAction(mactRun20.data());
        mactRun50 = new QAction(this);
          connect(mactRun50.data(), SIGNAL(triggered()), this, SLOT(run50()));
        mnu->addAction(mactRun50.data());
        mactRun100 = new QAction(this);
          connect(mactRun100.data(), SIGNAL(triggered()), this, SLOT(run100()));
        mnu->addAction(mactRun100.data());
        mactRunN = new QAction(this);
          connect(mactRunN.data(), SIGNAL(triggered()), this, SLOT(runN()));
        mnu->addAction(mactRunN.data());
      mactRun->setMenu(mnu);
    mactLoad = new QAction(this);
      mactLoad->setIcon(BApplication::icon("fileopen"));
      connect(mactLoad.data(), SIGNAL(triggered()), this, SLOT(load()));
    mactSaveAs = new QAction(this);
      mactSaveAs->setIcon(BApplication::icon("filesaveas"));
      connect(mactSaveAs.data(), SIGNAL(triggered()), this, SLOT(saveAs()));
    mactOpenDir = new QAction(this);
      mactOpenDir->setIcon(BApplication::icon("folder_open"));
      connect(mactOpenDir.data(), SIGNAL(triggered()), this, SLOT(openUserDir()));
    mactClearStack = new QAction(this);
      mactClearStack->setIcon(BApplication::icon("trash_empty"));
      connect(mactClearStack.data(), SIGNAL(triggered()), this, SLOT(clearStackSlot()));
    mspltr = new QSplitter(Qt::Horizontal);
      QWidget *wgt = new QWidget;
        QVBoxLayout *vlt = new QVBoxLayout(wgt);
          QToolBar *tbar = new QToolBar;
          vlt->addWidget(tbar);
          mcedtr = new BCodeEditor(BCodeEditor::SimpleDocument);
            mcedtr->addFileType(new PreTeXFileType);
            mcedtr->setPreferredFileType(mcedtr->fileType("PreTeX"));
            QAction *act = mcedtr->module(BCodeEditor::OpenSaveModule)->action(BOpenSaveEditorModule::NewFileAction);
            act->setShortcut(QKeySequence());
            tbar->addAction(act);
            tbar->addAction(mactLoad.data());
            tbar->addSeparator();
            act = mcedtr->module(BCodeEditor::OpenSaveModule)->action(BOpenSaveEditorModule::SaveFileAction);
            act->setShortcut(QKeySequence());
            tbar->addAction(act);
            tbar->addAction(mactSaveAs.data());
            tbar->addSeparator();
            tbar->addAction(mactOpenDir.data());
            tbar->addSeparator();
            tbar->addAction(mactStartStop.data());
            tbar->addAction(mactRun.data());
            tbar->addAction(mactClear.data());
            tbar->addSeparator();
            tbar->addAction(mactClearStack.data());
            qobject_cast<BLocalDocumentDriver *>(mcedtr->driver())->setDefaultDir(
                        BDirTools::findResource("pretex", BDirTools::UserOnly));
            connect(mcedtr.data(), SIGNAL(currentDocumentChanged(BAbstractCodeEditorDocument *)),
                    this, SLOT(cedtrCurrentDocumentChanged(BAbstractCodeEditorDocument *)));
            connect(mcedtr.data(), SIGNAL(documentAboutToBeAdded(BAbstractCodeEditorDocument *)),
                    this, SLOT(cedtrDocumentAboutToBeAdded(BAbstractCodeEditorDocument *)));
            connect(mcedtr.data(), SIGNAL(documentAboutToBeRemoved(BAbstractCodeEditorDocument *)),
                    this, SLOT(cedtrDocumentAboutToBeRemoved(BAbstractCodeEditorDocument *)));
            connect(mcedtr.data(), SIGNAL(currentDocumentFileNameChanged(QString)),
                    this, SLOT(cedtrCurrentDocumentFileNameChanged(QString)));
          vlt->addWidget(mcedtr.data());
          mstbar = new QStatusBar;
            mstbar->setSizeGripEnabled(false);
            BAbstractEditorModule *mdl = mcedtr->module(BCodeEditor::IndicatorsModule);
            mstbar->addPermanentWidget(mdl->widget(BIndicatorsEditorModule::FileTypeIndicator));
            mstbar->addPermanentWidget(mdl->widget(BIndicatorsEditorModule::CursorPositionIndicator));
            mstbar->addPermanentWidget(mdl->widget(BIndicatorsEditorModule::EncodingIndicator));
          vlt->addWidget(mstbar.data());
      mspltr->addWidget(wgt);
      mlstwgt = new QListWidget;
        connect(mlstwgt.data(), SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
                this, SLOT(lstwgtCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));
      mspltr->addWidget(mlstwgt.data());
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

/*============================== Static public methods =====================*/

ExecutionStack *PretexEditorModule::executionStack(PretexEditorModule *module)
{
    return mstacks.value((module && module->editor()) ? module->editor()->objectName() : QString());
}

/*============================== Public methods ============================*/

QString PretexEditorModule::id() const
{
    return "plugin/pretex";
}

QAction *PretexEditorModule::action(int type)
{
    switch (type)
    {
    case StartStopRecordingAction:
        return mactStartStop.data();
    case ClearAction:
        return mactClear.data();
    case RunAction:
        return mactRun.data();
    case LoadAction:
        return mactLoad.data();
    case SaveAsAction:
        return mactSaveAs.data();
    case OpenUserDirAction:
        return mactOpenDir.data();
    case ClearStackAction:
        return mactClearStack.data();
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
    if (extended)
    {
        list << action(LoadAction);
        list << action(SaveAsAction);
        list << action(OpenUserDirAction);
        list << action(ClearStackAction);
    }
    list.removeAll(0);
    return list;
}

QWidget *PretexEditorModule::widget(int type)
{
    switch (type)
    {
    case PretexEditorWidget:
        return mspltr;
    default:
        return 0;
    }
}

QByteArray PretexEditorModule::saveState() const
{
    QVariantMap m;
    m.insert("splitter_state", !mspltr.isNull() ? mspltr->saveState() : QByteArray());
    m.insert("last_n", mlastN);
    return BeQt::serialize(m);
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

bool PretexEditorModule::isRunning() const
{
    return mrunning;
}

QObject *PretexEditorModule::closeHandler() const
{
    return !mcedtr.isNull() ? mcedtr->closeHandler() : 0;
}

QObject *PretexEditorModule::dropHandler() const
{
    return !mcedtr.isNull() ? mcedtr->dropHandler() : 0;
}

/*============================== Public slots ==============================*/

void PretexEditorModule::startStopRecording()
{
    if (mrunning)
        return;
    if (mrecModule->isRecording())
    {
        mrecModule->stopRecording();
        editor()->findChild<QTabBar *>()->setEnabled(true);
        BAbstractCodeEditorDocument *doc = !mcedtr.isNull() ? mcedtr->addDocument() : 0;
        if (doc)
        {
            doc->setText(mrecModule->commands().join("\n"));
            doc->setModification(true);
        }
    }
    else
    {
        editor()->findChild<QTabBar *>()->setEnabled(false);
        mrecModule->setDocument(currentDocument());
        mrecModule->startRecording();
    }
    resetStartStopAction();
    checkActions();
}

void PretexEditorModule::clear()
{
    if (mrunning)
        return;
    if (!mcedtr.isNull() && mcedtr->currentDocument())
        mcedtr->currentDocument()->setText("");
    checkActions();
}

void PretexEditorModule::run(int n)
{
    if (n <= 0)
        n = 1;
    BAbstractCodeEditorDocument *doc = currentDocument();
    if (!doc || mrunning || mrecModule->isRecording() || mcedtr.isNull())
        return;
    BAbstractCodeEditorDocument *pdoc = !mcedtr.isNull() ? mcedtr->currentDocument() : 0;
    if (!pdoc)
        return;
    if (!mstbar.isNull())
        mstbar->clearMessage();
    editor()->findChild<QTabBar *>()->setEnabled(false);
    mrunning = true;
    checkActions();
    resetStartStopAction();
    bool ok = false;
    QString err;
    int pos;
    QString fn;
    QList<Token> tokens = LexicalAnalyzer(pdoc->text(true), pdoc->fileName(), pdoc->codec()).analyze(&ok, &err,
                                                                                                     &pos, &fn);
    if (!ok)
    {
        mrunning = false;
        checkActions();
        resetStartStopAction();
        editor()->findChild<QTabBar *>()->setEnabled(true);
        pdoc = !mcedtr.isNull() ? mcedtr->document(fn) : 0;
        if (pdoc)
        {
            mcedtr->setCurrentDocument(pdoc);
            pdoc->selectText(pos, pos);
        }
        showErrorMessage(doc, err, pos, fn);
        return;
    }
    ok = false;
    Token t;
    Token *prog = Parser(tokens).parse(&ok, &err, &t);
    if (!ok)
    {
        mrunning = false;
        checkActions();
        resetStartStopAction();
        editor()->findChild<QTabBar *>()->setEnabled(true);
        pdoc = !mcedtr.isNull() ? mcedtr->document(fn) : 0;
        if (pdoc)
        {
            mcedtr->setCurrentDocument(pdoc);
            pdoc->selectText(pos, pos);
        }
        showErrorMessage(doc, err, t.position());
        return;
    }
    SpontaneousEventEater eater(doc);
    Q_UNUSED(eater)
    for (int i = 0; i < n; ++i)
    {
        QString err;
        ExecutionStack stack(executionStack(this));
        if (!ExecutionModule(prog, doc, &stack).execute(&err))
        {
            if (!mcedtr.isNull() && pdoc)
                mcedtr->setCurrentDocument(pdoc);
            if (!mstbar.isNull())
                mstbar->showMessage(tr("Error:", "error") + " " + err);
            break;
        }
    }
    mrunning = false;
    checkActions();
    resetStartStopAction();
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

bool PretexEditorModule::load(const QString &fileName)
{
    if (mrunning || mrecModule->isRecording() || mcedtr.isNull())
        return false;
    return !fileName.isEmpty() ? (bool) mcedtr->openDocument(fileName) : !mcedtr->openDocuments().isEmpty();
}

bool PretexEditorModule::saveAs()
{
    if (mrecModule->isRecording() || mcedtr.isNull() || !mcedtr->currentDocument())
        return false;
    return mcedtr->saveCurrentDocumentAs();
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
    foreach (const QString &path, BApplication::locations("pretex"))
    {
        foreach (const QString &fn, BDirTools::entryList(path, QStringList() << "*.pretex", QDir::Files))
            mcedtr->openDocument(fn);
    }
}

/*============================== Protected methods =========================*/

void PretexEditorModule::editorSet(BCodeEditor *edr)
{
    if (edr)
    {
        if (!mstackRefs.contains(edr->objectName()))
        {
            ExecutionStack *s = new ExecutionStack;
            if (PretexEditorModulePlugin::saveExecutionStack())
                s->restoreState(PretexEditorModulePlugin::executionStackState(this));
            mstacks.insert(edr->objectName(), s);
            mstackRefs.insert(edr->objectName(), 1);
        }
        else
        {
            ++mstackRefs[edr->objectName()];
        }
    }
    resetStartStopAction();
    checkActions();
}

void PretexEditorModule::editorUnset(BCodeEditor *edr)
{
    if (edr)
    {
        --mstackRefs[edr->objectName()];
        if (!mstackRefs.value(edr->objectName()))
        {
            ExecutionStack *s = mstacks.take(edr->objectName());
            mstackRefs.remove(edr->objectName());
            if (PretexEditorModulePlugin::saveExecutionStack())
                PretexEditorModulePlugin::setExecutionStackState(s->saveState());
            delete s;
        }
    }
    if (!mspltr.isNull())
    {
        mspltr->setParent(0);
        mspltr->hide();
    }
    resetStartStopAction();
    checkActions();
}

void PretexEditorModule::currentDocumentChanged(BAbstractCodeEditorDocument *)
{
    resetStartStopAction();
    checkActions();
}

/*============================== Static private methods ====================*/

void PretexEditorModule::showErrorMessage(BAbstractCodeEditorDocument *doc, const QString &err, int pos,
                                          const QString &fn)
{
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

void PretexEditorModule::resetStartStopAction()
{
    if (mactStartStop.isNull())
        return;
    mactStartStop->setEnabled(currentDocument() && !mrunning);
    if (mrecModule->isRecording())
    {
        mactStartStop->setIcon(BApplication::icon("player_stop"));
        mactStartStop->setText(tr("Stop recording", "act text"));
        mactStartStop->setToolTip(tr("Stop recording commands", "act toolTip"));
        mactStartStop->setWhatsThis(tr("Use this action to finish recording commands", "act whatsThis"));
    }
    else
    {
        mactStartStop->setIcon(BApplication::icon("player_record"));
        mactStartStop->setText(tr("Start recording", "act text"));
        mactStartStop->setToolTip(tr("Start recording commands", "act toolTip"));
        mactStartStop->setWhatsThis(tr("Use this action to begin recording commands", "act whatsThis"));
    }
}

void PretexEditorModule::checkActions()
{
    bool b = currentDocument();
    if (!mactClear.isNull())
        mactClear->setEnabled(!mrunning);
    if (!mactClearStack.isNull())
        mactClearStack->setEnabled(!mrunning);
    if (!mactRun.isNull())
        mactRun->setEnabled(b && !mrunning && !mrecModule->isRecording());
    if (!mactSaveAs.isNull())
        mactSaveAs->setEnabled(!mrecModule->isRecording());
}

/*============================== Private slots =============================*/

void PretexEditorModule::retranslateUi()
{
    if (!mactClear.isNull())
    {
        mactClear->setText(tr("Clear document", "act text"));
        mactClear->setToolTip(tr("Clear current document", "act toolTip"));
        mactClear->setWhatsThis(tr("Use this action to clear currently loaded or recorded document. "
                                   "The corresponding file will not be deleted", "act whatsThis"));
    }
    if (!mactClearStack.isNull())
    {
        mactClearStack->setText(tr("Clear stack", "act text"));
        mactClearStack->setToolTip(tr("Clear PreTeX stack", "act toolTip"));
        mactClearStack->setWhatsThis(tr("Use this action to clear the PreTeX execution stack, "
                                        "i.e. to delete all global variables and functions", "act whatsThis"));
    }
    if (!mactRun.isNull())
    {
        mactRun->setText(tr("Run", "act text"));
        mactRun->setToolTip(tr("Run current document", "act toolTip"));
        mactRun->setWhatsThis(tr("Use this action to activate previously loaded or recorded file", "act whatsThis"));
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
    if (!mactLoad.isNull())
    {
        mactLoad->setText(tr("Load...", "act text"));
        mactLoad->setToolTip(tr("Load file", "act toolTip"));
        mactLoad->setWhatsThis(tr("Use this action to load previously saved file", "act whatsThis"));
    }
    if (!mactSaveAs.isNull())
    {
        mactSaveAs->setText(tr("Save as...", "act text"));
        mactSaveAs->setToolTip(tr("Save current document as...", "act toolTip"));
        mactSaveAs->setWhatsThis(tr("Use this action to save current document", "act whatsThis"));
    }
    if (!mactOpenDir.isNull())
    {
        mactOpenDir->setText(tr("Open user PreTeX directory", "act text"));
        mactOpenDir->setWhatsThis(tr("Use this action to open PreTeX user directory", "act whatsThis"));
    }
    if (!mcedtr.isNull())
        mcedtr->setDefaultFileName(tr("New document.pretex", "default document file name"));
    resetStartStopAction();
}

void PretexEditorModule::lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *)
{
    if (mcedtr.isNull())
        return;
    if (current)
    {
        foreach (BAbstractCodeEditorDocument *doc, mcedtr->documents())
        {
            if (doc->fileName() == current->data(Qt::ToolTipRole).toString())
            {
                mcedtr->setCurrentDocument(doc);
                break;
            }
        }
    }
    else
    {
        mcedtr->setCurrentDocument(0);
    }
}

void PretexEditorModule::cedtrCurrentDocumentChanged(BAbstractCodeEditorDocument *doc)
{
    if (mlstwgt.isNull())
        return;
    mlstwgt->setCurrentItem(findItemByFileName(mlstwgt.data(), doc ? doc->fileName() : QString()));
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
    delete findItemByFileName(mlstwgt.data(), doc->fileName());
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

void PretexEditorModule::clearStackSlot()
{
    ExecutionStack *s = mstacks[editor() ? editor()->objectName() : QString()];
    if (s)
        s->clear();
    PretexEditorModulePlugin::clearExecutionStack(this);
}

/*============================== Static private members ====================*/

QMap<QString, ExecutionStack *> PretexEditorModule::mstacks = QMap<QString, ExecutionStack *>();
QMap<QString, int> PretexEditorModule::mstackRefs = QMap<QString, int>();
