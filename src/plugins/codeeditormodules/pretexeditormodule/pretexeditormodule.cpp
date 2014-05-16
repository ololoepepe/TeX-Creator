/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
**
** This file is part of the MacrosEditorModule plugin of TeX Creator.
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
#include "macro.h"
#include "macroexecutionstack.h"

#include <BAbstractEditorModule>
#include <BCodeEditor>
#include <BAbstractCodeEditorDocument>
#include <BDirTools>
#include <BeQt>
#include <BAbstractFileType>
#include <BSignalDelayProxy>
#include <BAbstractDocumentDriver>
#include <BLocalDocumentDirver>
#include <BOpenSaveEditorModule>
#include <BIndicatorsEditorModule>
#include <BApplication>

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

#include <QDebug>

#include <climits>

Q_GLOBAL_STATIC(MacroExecutionStack, mstack)

/*============================================================================
================================ TeXCreatorMacroFileType =====================
============================================================================*/

class TeXCreatorMacroFileType : public BAbstractFileType
{
    Q_DECLARE_TR_FUNCTIONS(TeXCreatorMacroFileType)
public:
    TeXCreatorMacroFileType();
    ~TeXCreatorMacroFileType();
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
    Q_DISABLE_COPY(TeXCreatorMacroFileType)
};

/*============================================================================
================================ Static functions ============================
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
================================ TeXCreatorMacroFileType =====================
============================================================================*/

/*============================== Public constructors =======================*/

TeXCreatorMacroFileType::TeXCreatorMacroFileType()
{
    //
}

TeXCreatorMacroFileType::~TeXCreatorMacroFileType()
{
    //
}

/*============================== Public methods ============================*/

QString TeXCreatorMacroFileType::id() const
{
    return "TeX Creator macro";
}

QString TeXCreatorMacroFileType::name() const
{
    return tr("TeX Creator macro", "name");
}

QString TeXCreatorMacroFileType::description() const
{
    return tr("TeX Creator macro files", "description");
}

QStringList TeXCreatorMacroFileType::suffixes() const
{
    return QStringList() << "tcm";
}

bool TeXCreatorMacroFileType::matchesFileName(const QString &fileName) const
{
    return suffixes().contains(QFileInfo(fileName).suffix(), Qt::CaseInsensitive);
}

BAbstractFileType::BracketPairList TeXCreatorMacroFileType::brackets() const
{
    BracketPairList list;
    list << createBracketPair("{", "}", "\\");
    list << createBracketPair("[", "]", "\\");
    return list;
}

/*============================== Protected methods =========================*/

void TeXCreatorMacroFileType::highlightBlock(const QString &text)
{
    //comments
    int comInd = text.indexOf('%');
    while (comInd > 0 && text.at(comInd - 1) == '\\')
        comInd = text.indexOf('%', comInd + 1);
    BCodeEdit::setBlockComment(currentBlock(), comInd);
    if (comInd >= 0)
        setFormat(comInd, text.length() - comInd, QColor(Qt::darkGray));
    QString ntext = text.left(comInd);
    //commands
    QRegExp rx("(\\\\(multi|for|while|doWhile|until|doUntil|if|wait|defF?|undef|defined|setF?|get|call|var|binM?|un|c"
               "|press|insert|format|find|replace(Sel|Doc)?|exec(F|D|FD)?)\\b)+");
    int pos = rx.indexIn(ntext);
    while (pos >= 0)
    {
        int len = rx.matchedLength();
        setFormat(pos, len, QColor(Qt::red).lighter(70));
        pos = rx.indexIn(ntext, pos + len);
    }
}

/*============================================================================
================================ MacrosEditorModule ==========================
============================================================================*/

/*============================== Static public methods =====================*/

void PretexEditorModule::saveMacroStack()
{
    bSettings->setValue("Macros/stack_state", mstack()->save());
}

void PretexEditorModule::loadMacroStack()
{
    mstack()->restore(bSettings->value("Macros/stack_state").toByteArray());
}

void PretexEditorModule::clearMacroStack()
{
    mstack()->clear();
}

/*============================== Public constructors =======================*/

PretexEditorModule::PretexEditorModule(QObject *parent) :
    BAbstractEditorModule(parent)
{
#if defined(BUILTIN_RESOURCES)
    Q_INIT_RESOURCE(pretexeditormodule);
    Q_INIT_RESOURCE(pretexeditormodule_transtations);
#endif
    mplaying = false;
    mrecording = false;
    mprevDoc = 0;
    mproxy = new BSignalDelayProxy(this);
    mlastN = 1;
    connect(mproxy, SIGNAL(triggered()), this, SLOT(ptedtTextChanged()));
    //
    mactStartStop = new QAction(this);
      connect(mactStartStop.data(), SIGNAL(triggered()), this, SLOT(startStopRecording()));
    mactClearMacro = new QAction(this);
      mactClearMacro->setIcon(BApplication::icon("editclear"));
      connect(mactClearMacro.data(), SIGNAL(triggered()), this, SLOT(clearMacro()));
    mactPlay = new QAction(this);
      mactPlay->setIcon(BApplication::icon("player_play"));
      connect(mactPlay.data(), SIGNAL(triggered()), this, SLOT(playMacro()));
      QMenu *mnu = new QMenu;
        mactPlay5 = new QAction(this);
          connect(mactPlay5.data(), SIGNAL(triggered()), this, SLOT(playMacro5()));
        mnu->addAction(mactPlay5.data());
        mactPlay10 = new QAction(this);
          connect(mactPlay10.data(), SIGNAL(triggered()), this, SLOT(playMacro10()));
        mnu->addAction(mactPlay10.data());
        mactPlay20 = new QAction(this);
          connect(mactPlay20.data(), SIGNAL(triggered()), this, SLOT(playMacro20()));
        mnu->addAction(mactPlay20.data());
        mactPlay50 = new QAction(this);
          connect(mactPlay50.data(), SIGNAL(triggered()), this, SLOT(playMacro50()));
        mnu->addAction(mactPlay50.data());
        mactPlay100 = new QAction(this);
          connect(mactPlay100.data(), SIGNAL(triggered()), this, SLOT(playMacro100()));
        mnu->addAction(mactPlay100.data());
        mactPlayN = new QAction(this);
          connect(mactPlayN.data(), SIGNAL(triggered()), this, SLOT(playMacroN()));
        mnu->addAction(mactPlayN.data());
      mactPlay->setMenu(mnu);
    mactLoad = new QAction(this);
      mactLoad->setIcon(BApplication::icon("fileopen"));
      connect(mactLoad.data(), SIGNAL(triggered()), this, SLOT(loadMacro()));
    mactSaveAs = new QAction(this);
      mactSaveAs->setIcon(BApplication::icon("filesaveas"));
      connect(mactSaveAs.data(), SIGNAL(triggered()), this, SLOT(saveMacroAs()));
    mactOpenDir = new QAction(this);
      mactOpenDir->setIcon(BApplication::icon("folder_open"));
      connect(mactOpenDir.data(), SIGNAL(triggered()), this, SLOT(openUserDir()));
    mactClearStack = new QAction(this);
      mactClearStack->setIcon(BApplication::icon("trash_empty"));
      connect(mactClearStack.data(), SIGNAL(triggered()), this, SLOT(clearMacroStackSlot()));
    mspltr = new QSplitter(Qt::Horizontal);
      QWidget *wgt = new QWidget;
        QVBoxLayout *vlt = new QVBoxLayout(wgt);
          QToolBar *tbar = new QToolBar;
          vlt->addWidget(tbar);
          mcedtr = new BCodeEditor(BCodeEditor::SimpleDocument);
            mcedtr->addFileType(new TeXCreatorMacroFileType);
            mcedtr->setPreferredFileType(mcedtr->fileType("TeX Creator macro"));
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
            tbar->addAction(mactPlay.data());
            tbar->addAction(mactClearMacro.data());
            tbar->addSeparator();
            tbar->addAction(mactClearStack.data());
            qobject_cast<BLocalDocumentDriver *>(mcedtr->driver())->setDefaultDir(
                        BDirTools::findResource("macros", BDirTools::UserOnly));
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
    reloadMacros();
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

QString PretexEditorModule::id() const
{
    return "pretex_editor_module";
}

QAction *PretexEditorModule::action(int type)
{
    switch (type)
    {
    case StartStopRecordingAction:
        return mactStartStop.data();
    case ClearMacroAction:
        return mactClearMacro.data();
    case PlayAction:
        return mactPlay.data();
    case LoadAction:
        return mactLoad.data();
    case SaveAsAction:
        return mactSaveAs.data();
    case OpenUserMacrosDirAction:
        return mactOpenDir.data();
    case ClearMacroStackAction:
        return mactClearStack.data();
    default:
        return 0;
    }
}

QList<QAction *> PretexEditorModule::actions(bool extended)
{
    QList<QAction *> list;
    list << action(StartStopRecordingAction);
    list << action(PlayAction);
    list << action(ClearMacroAction);
    if (extended)
    {
        list << action(LoadAction);
        list << action(SaveAsAction);
        list << action(OpenUserMacrosDirAction);
        list << action(ClearMacroStackAction);
    }
    return list;
}

QWidget *PretexEditorModule::widget(int type)
{
    switch (type)
    {
    case MacrosEditorWidget:
        return mspltr.data();
    default:
        return 0;
    }
}

bool PretexEditorModule::eventFilter(QObject *, QEvent *e)
{
    if (!mrecording)
        return false;
    if (!e || e->type() != QEvent::KeyPress)
        return false;
    QString err;
    if (!mmacro.recordKeyPress(static_cast<QKeyEvent *>(e), &err))
    {
        //show message
    }
    return false;
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

bool PretexEditorModule::isPlaying() const
{
    return mplaying;
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
    if (mplaying)
        return;
    mrecording = !mrecording;
    if (mrecording)
        clearMacro();
    else
        setPtedtText(mmacro.toText());
    resetStartStopAction();
    checkActions();
}

void PretexEditorModule::clearMacro()
{
    if (mplaying)
        return;
    mmacro.clear();
    if (!mcedtr.isNull())
        clearPtedt();
    checkActions();
}

void PretexEditorModule::playMacro(int n)
{
    if (n <= 0)
        n = 1;
    BAbstractCodeEditorDocument *doc = currentDocument();
    if (!doc || mplaying || mrecording || !mmacro.isValid() || mcedtr.isNull())
        return;
    mplaying = true;
    checkActions();
    resetStartStopAction();
    for (int i = 0; i < n; ++i)
    {
        QString err;
        MacroExecutionStack iterationStack(mstack());
        mmacro.execute(doc, &iterationStack, mcedtr.data(), &err);
        if (!err.isEmpty())
        {
            if (!mstbar.isNull())
                mstbar->showMessage("Error: " + err);
            break;
        }
    }
    mplaying = false;
    checkActions();
    resetStartStopAction();
}

void PretexEditorModule::playMacro5()
{
    playMacro(5);
}

void PretexEditorModule::playMacro10()
{
    playMacro(10);
}

void PretexEditorModule::playMacro20()
{
    playMacro(20);
}

void PretexEditorModule::playMacro50()
{
    playMacro(50);
}

void PretexEditorModule::playMacro100()
{
    playMacro(100);
}

void PretexEditorModule::playMacroN()
{
    bool ok = false;
    int n = QInputDialog::getInt(editor(), tr("Enter a number", "idlg title"), tr("Number of iterations:", "lbl text"),
                                 mlastN, 1, INT_MAX, 1, &ok);
    if (!ok)
        return;
    mlastN = n;
    playMacro(n);
}

bool PretexEditorModule::loadMacro(const QString &fileName)
{
    if (mplaying || mrecording || mcedtr.isNull())
        return false;
    return !fileName.isEmpty() ? (bool) mcedtr->openDocument(fileName) : !mcedtr->openDocuments().isEmpty();
}

bool PretexEditorModule::saveMacroAs()
{
    if (mrecording || !mmacro.isValid() || mcedtr.isNull() || !mcedtr->currentDocument())
        return false;
    return mcedtr->saveCurrentDocumentAs();
}

void PretexEditorModule::openUserDir()
{
    bApp->openLocalFile(BDirTools::findResource("macros"));
}

void PretexEditorModule::reloadMacros()
{
    if (mcedtr.isNull())
        return;
    mcedtr->closeAllDocuments();
    foreach (const QString &path, BApplication::locations("macros"))
    {
        foreach (const QString &fn, BDirTools::entryList(path, QStringList() << "*.tcm", QDir::Files))
            mcedtr->openDocument(fn);
    }
}

/*============================== Protected methods =========================*/

void PretexEditorModule::editorSet(BCodeEditor *)
{
    resetStartStopAction();
    checkActions();
}

void PretexEditorModule::editorUnset(BCodeEditor *)
{
    if (!mspltr.isNull())
    {
        mspltr->setParent(0);
        mspltr->hide();
    }
    resetStartStopAction();
    checkActions();
}

void PretexEditorModule::currentDocumentChanged(BAbstractCodeEditorDocument *doc)
{
    if (mprevDoc)
        mprevDoc->findChild<QPlainTextEdit *>()->removeEventFilter(this);
    mprevDoc = doc;
    if (doc)
        doc->findChild<QPlainTextEdit *>()->installEventFilter(this);
    if (mplaying)
        mplaying = false;
    if (mrecording)
        startStopRecording();
    resetStartStopAction();
    checkActions();
}

/*============================== Static private methods ====================*/

QString PretexEditorModule::fileDialogFilter()
{
    return tr("TeX Creator macros", "fdlg filter") + " (*.tcm)";
}

/*============================== Private methods ===========================*/

void PretexEditorModule::resetStartStopAction()
{
    if (mactStartStop.isNull())
        return;
    mactStartStop->setEnabled(currentDocument() && !mplaying);
    if (mrecording)
    {
        mactStartStop->setIcon(BApplication::icon("player_stop"));
        mactStartStop->setText(tr("Stop recording", "act text"));
        mactStartStop->setToolTip(tr("Stop recording macro", "act toolTip"));
        mactStartStop->setWhatsThis(tr("Use this action to finish recording macro", "act whatsThis"));
    }
    else
    {
        mactStartStop->setIcon(BApplication::icon("player_record"));
        mactStartStop->setText(tr("Start recording", "act text"));
        mactStartStop->setToolTip(tr("Start recording macro", "act toolTip"));
        mactStartStop->setWhatsThis(tr("Use this action to begin recording macro", "act whatsThis"));
    }
}

void PretexEditorModule::checkActions()
{
    bool b = currentDocument();
    if (!mactClearMacro.isNull())
        mactClearMacro->setEnabled(!mplaying);
    if (!mactClearStack.isNull())
        mactClearStack->setEnabled(!mplaying);
    if (!mactPlay.isNull())
        mactPlay->setEnabled(b && !mplaying && !mrecording && mmacro.isValid() && !mmacro.isEmpty());
    if (!mactSaveAs.isNull())
        mactSaveAs->setEnabled(!mrecording && mmacro.isValid());
}

void PretexEditorModule::appendPtedtText(const QString &text)
{
    if (mcedtr.isNull() || !mcedtr->currentDocument())
        return;
    mcedtr->currentDocument()->setText(mcedtr->currentDocument()->text() + "\n" + text);
}

void PretexEditorModule::setPtedtText(const QString &text)
{
    if (mcedtr.isNull() || !mcedtr->currentDocument())
        return;
    mcedtr->currentDocument()->setText(text);
}

void PretexEditorModule::clearPtedt()
{
    if (mcedtr.isNull() || !mcedtr->currentDocument())
        return;
    mcedtr->currentDocument()->setText("");
}

/*============================== Private slots =============================*/

void PretexEditorModule::retranslateUi()
{
    if (!mactClearMacro.isNull())
    {
        mactClearMacro->setText(tr("Clear macro", "act text"));
        mactClearMacro->setToolTip(tr("Clear current macro", "act toolTip"));
        mactClearMacro->setWhatsThis(tr("Use this action to clear currently loaded or recorded macro. "
                                        "The corresponding file will not be deleted", "act whatsThis"));
    }
    if (!mactClearStack.isNull())
    {
        mactClearStack->setText(tr("Clear stack", "act text"));
        mactClearStack->setToolTip(tr("Clear macros stack", "act toolTip"));
        mactClearStack->setWhatsThis(tr("Use this action to clear the macros stack, "
                                        "i.e. to undefine all global variables and functions", "act whatsThis"));
    }
    if (!mactPlay.isNull())
    {
        mactPlay->setText(tr("Play", "act text"));
        mactPlay->setToolTip(tr("Play current macro", "act toolTip"));
        mactPlay->setWhatsThis(tr("Use this action to activate previously loaded or recorded macro", "act whatsThis"));
    }
    if (!mactPlay5.isNull())
        mactPlay5->setText(tr("Play 5 times", "act text"));
    if (!mactPlay10.isNull())
        mactPlay10->setText(tr("Play 10 times", "act text"));
    if (!mactPlay20.isNull())
        mactPlay20->setText(tr("Play 20 times", "act text"));
    if (!mactPlay50.isNull())
        mactPlay50->setText(tr("Play 50 times", "act text"));
    if (!mactPlay100.isNull())
        mactPlay100->setText(tr("Play 100 times", "act text"));
    if (!mactPlayN.isNull())
        mactPlayN->setText(tr("Play N times", "act text"));
    if (!mactLoad.isNull())
    {
        mactLoad->setText(tr("Load...", "act text"));
        mactLoad->setToolTip(tr("Load macro", "act toolTip"));
        mactLoad->setWhatsThis(tr("Use this action to load previously saved macro from file", "act whatsThis"));
    }
    if (!mactSaveAs.isNull())
    {
        mactSaveAs->setText(tr("Save as...", "act text"));
        mactSaveAs->setToolTip(tr("Save current macro as...", "act toolTip"));
        mactSaveAs->setWhatsThis(tr("Use this action to save current macro to a file", "act whatsThis"));
    }
    if (!mactOpenDir.isNull())
    {
        mactOpenDir->setText(tr("Open user macros directory", "act text"));
        mactOpenDir->setWhatsThis(tr("Use this action to open macros user directory", "act whatsThis"));
    }
    if (!mcedtr.isNull())
        mcedtr->setDefaultFileName(tr("New macro.tcm", "default document file name"));
    resetStartStopAction();
}

void PretexEditorModule::ptedtTextChanged()
{
    if (mcedtr.isNull() || !mcedtr->currentDocument())
        return;
    QString err;
    mmacro.fromText(mcedtr->currentDocument()->text(), &err);
    if (!mstbar.isNull())
    {
        if (err.isEmpty())
        {
            if (mmacro.isEmpty())
                mstbar->showMessage("Macro is empty");
            else
                mstbar->showMessage("Macro is ready for use");
        }
        else
            mstbar->showMessage("Macro is invalid: " + err);
    }
    checkActions();
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
    mproxy->trigger();
}

void PretexEditorModule::cedtrDocumentAboutToBeAdded(BAbstractCodeEditorDocument *doc)
{
    if (!doc || mlstwgt.isNull())
        return;
    QFileInfo fi(doc->fileName());
    QListWidgetItem *lwi = new QListWidgetItem(fi.baseName());
    lwi->setData(Qt::ToolTipRole, fi.filePath());
    mlstwgt->addItem(lwi);
    connect(doc->findChild<QPlainTextEdit *>(), SIGNAL(textChanged()), mproxy, SLOT(trigger()));
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

void PretexEditorModule::clearMacroStackSlot()
{
    clearMacroStack();
}
