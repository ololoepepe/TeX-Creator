#include "macroseditormodule.h"
#include "application.h"
#include "global.h"
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
    QRegExp rx("(\\\\(multi|for|if|wait|defF?|undef|defined|setF?|get|call|var|bin|un|c"
               "|press|insert|find|replace(Sel|Doc)?|exec(F|D|FD)?)\\b)+");
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

void MacrosEditorModule::saveMacroStack()
{
    bSettings->setValue("Macros/stack_state", mstack()->save());
}

void MacrosEditorModule::loadMacroStack()
{
    mstack()->restore(bSettings->value("Macros/stack_state").toByteArray());
}

/*============================== Public constructors =======================*/

MacrosEditorModule::MacrosEditorModule(QObject *parent) :
    BAbstractEditorModule(parent)
{
    mplaying = false;
    mrecording = false;
    mprevDoc = 0;
    mproxy = new BSignalDelayProxy(this);
    connect(mproxy, SIGNAL(triggered()), this, SLOT(ptedtTextChanged()));
    //
    mactStartStop = new QAction(this);
      connect(mactStartStop.data(), SIGNAL(triggered()), this, SLOT(startStopRecording()));
    mactClear = new QAction(this);
      mactClear->setIcon(Application::icon("editclear"));
      connect(mactClear.data(), SIGNAL(triggered()), this, SLOT(clearMacro()));
    mactPlay = new QAction(this);
      mactPlay->setIcon(Application::icon("player_play"));
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
      mactLoad->setIcon(Application::icon("fileopen"));
      connect(mactLoad.data(), SIGNAL(triggered()), this, SLOT(loadMacro()));
    mactSaveAs = new QAction(this);
      mactSaveAs->setIcon(Application::icon("filesaveas"));
      connect(mactSaveAs.data(), SIGNAL(triggered()), this, SLOT(saveMacroAs()));
    mactOpenDir = new QAction(this);
      mactOpenDir->setIcon(Application::icon("folder_open"));
      connect(mactOpenDir.data(), SIGNAL(triggered()), this, SLOT(openUserDir()));
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
            tbar->addAction(mactClear.data());
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
MacrosEditorModule::~MacrosEditorModule()
{
    if (!mspltr.isNull())
        delete mspltr;
}

/*============================== Public methods ============================*/

QString MacrosEditorModule::id() const
{
    return "macros";
}

QAction *MacrosEditorModule::action(int type)
{
    switch (type)
    {
    case StartStopRecordingAction:
        return mactStartStop.data();
    case ClearAction:
        return mactClear.data();
    case PlayAction:
        return mactPlay.data();
    case LoadAction:
        return mactLoad.data();
    case SaveAsAction:
        return mactSaveAs.data();
    case OpenUserMacrosDirAction:
        return mactOpenDir.data();
    default:
        return 0;
    }
}

QList<QAction *> MacrosEditorModule::actions(bool extended)
{
    QList<QAction *> list;
    list << action(StartStopRecordingAction);
    list << action(PlayAction);
    list << action(ClearAction);
    if (extended)
    {
        list << action(LoadAction);
        list << action(SaveAsAction);
        list << action(OpenUserMacrosDirAction);
    }
    return list;
}

QWidget *MacrosEditorModule::widget(int type)
{
    switch (type)
    {
    case MacrosEditorWidget:
        return mspltr.data();
    default:
        return 0;
    }
}

bool MacrosEditorModule::eventFilter(QObject *, QEvent *e)
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

QByteArray MacrosEditorModule::saveState() const
{
    QVariantMap m;
    m.insert("splitter_state", !mspltr.isNull() ? mspltr->saveState() : QByteArray());
    return BeQt::serialize(m);
}

void MacrosEditorModule::restoreState(const QByteArray &state)
{
    QVariantMap m = BeQt::deserialize(state).toMap();
    if (!mspltr.isNull())
        mspltr->restoreState(m.value("splitter_state").toByteArray());
}

/*============================== Public slots ==============================*/

void MacrosEditorModule::startStopRecording()
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

void MacrosEditorModule::clearMacro()
{
    if (mplaying)
        return;
    mmacro.clear();
    if (!mcedtr.isNull())
        clearPtedt();
    checkActions();
}

void MacrosEditorModule::playMacro(int n)
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

void MacrosEditorModule::playMacro5()
{
    playMacro(5);
}

void MacrosEditorModule::playMacro10()
{
    playMacro(10);
}

void MacrosEditorModule::playMacro20()
{
    playMacro(20);
}

void MacrosEditorModule::playMacro50()
{
    playMacro(50);
}

void MacrosEditorModule::playMacro100()
{
    playMacro(100);
}

void MacrosEditorModule::playMacroN()
{
    bool ok = false;
    int n = QInputDialog::getInt(editor(), tr("Enter a number", "idlg title"), tr("Number of iterations:", "lbl text"),
                                 1, 1, INT_MAX, 1, &ok);
    if (!ok)
        return;
    playMacro(n);
}

bool MacrosEditorModule::loadMacro(const QString &fileName)
{
    if (mplaying || mrecording || mcedtr.isNull())
        return false;
    return !fileName.isEmpty() ? (bool) mcedtr->openDocument(fileName) : !mcedtr->openDocuments().isEmpty();
}

bool MacrosEditorModule::saveMacroAs()
{
    if (mrecording || !mmacro.isValid() || mcedtr.isNull() || !mcedtr->currentDocument())
        return false;
    return mcedtr->saveCurrentDocumentAs();
}

void MacrosEditorModule::openUserDir()
{
    bApp->openLocalFile(BDirTools::findResource("macros"));
}

void MacrosEditorModule::reloadMacros()
{
    if (mcedtr.isNull())
        return;
    mcedtr->closeAllDocuments();
    foreach (const QString &path, Application::locations("macros"))
    {
        foreach (const QString &fn, BDirTools::entryList(path, QStringList() << "*.tcm", QDir::Files))
            mcedtr->openDocument(fn);
    }
}

/*============================== Protected methods =========================*/

void MacrosEditorModule::editorSet(BCodeEditor *)
{
    resetStartStopAction();
    checkActions();
}

void MacrosEditorModule::editorUnset(BCodeEditor *)
{
    if (!mspltr.isNull())
    {
        mspltr->setParent(0);
        mspltr->hide();
    }
    resetStartStopAction();
    checkActions();
}

void MacrosEditorModule::currentDocumentChanged(BAbstractCodeEditorDocument *doc)
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

QString MacrosEditorModule::fileDialogFilter()
{
    return tr("TeX Creator macros", "fdlg filter") + " (*.tcm)";
}

/*============================== Private methods ===========================*/

void MacrosEditorModule::resetStartStopAction()
{
    if (mactStartStop.isNull())
        return;
    mactStartStop->setEnabled(currentDocument() && !mplaying);
    if (mrecording)
    {
        mactStartStop->setIcon( Application::icon("player_stop") );
        mactStartStop->setText( tr("Stop recording", "act text") );
        mactStartStop->setToolTip( tr("Stop recording macro", "act toolTip") );
        //TODO: whatsThis
    }
    else
    {
        mactStartStop->setIcon( Application::icon("player_record") );
        mactStartStop->setText( tr("Start recording", "act text") );
        mactStartStop->setToolTip( tr("Start recording macro", "act toolTip") );
        //TODO: whatsThis
    }
}

void MacrosEditorModule::checkActions()
{
    bool b = currentDocument();
    if (!mactClear.isNull())
        mactClear->setEnabled(!mplaying);
    if (!mactPlay.isNull())
        mactPlay->setEnabled(b && !mplaying && !mrecording && mmacro.isValid() && !mmacro.isEmpty());
    if (!mactSaveAs.isNull())
        mactSaveAs->setEnabled(!mrecording && mmacro.isValid());
}

void MacrosEditorModule::appendPtedtText(const QString &text)
{
    if (mcedtr.isNull() || !mcedtr->currentDocument())
        return;
    mcedtr->currentDocument()->setText(mcedtr->currentDocument()->text() + "\n" + text);
}

void MacrosEditorModule::setPtedtText(const QString &text)
{
    if (mcedtr.isNull() || !mcedtr->currentDocument())
        return;
    mcedtr->currentDocument()->setText(text);
}

void MacrosEditorModule::clearPtedt()
{
    if (mcedtr.isNull() || !mcedtr->currentDocument())
        return;
    mcedtr->currentDocument()->setText("");
}

/*============================== Private slots =============================*/

void MacrosEditorModule::retranslateUi()
{
    if (!mactClear.isNull())
    {
        mactClear->setText(tr("Clear", "act text"));
        mactClear->setToolTip(tr("Clear current macro", "act toolTip"));
        mactClear->setWhatsThis(tr("Use this action to clear currntly loaded or recorded macro. "
                                   "The corresponding file will not be deleted", "act whatsThis"));
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
        mactOpenDir->setText(tr("Open user macros dir", "act text"));
        //TODO: toolTip and whatsThis
    }
    if (!mcedtr.isNull())
        mcedtr->setDefaultFileName(tr("New macro.tcm", "default document file name"));
    resetStartStopAction();
}

void MacrosEditorModule::ptedtTextChanged()
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

void MacrosEditorModule::lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *)
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

void MacrosEditorModule::cedtrCurrentDocumentChanged(BAbstractCodeEditorDocument *doc)
{
    if (mlstwgt.isNull())
        return;
    mlstwgt->setCurrentItem(findItemByFileName(mlstwgt.data(), doc ? doc->fileName() : QString()));
    mproxy->trigger();
}

void MacrosEditorModule::cedtrDocumentAboutToBeAdded(BAbstractCodeEditorDocument *doc)
{
    if (!doc || mlstwgt.isNull())
        return;
    QFileInfo fi(doc->fileName());
    QListWidgetItem *lwi = new QListWidgetItem(fi.baseName());
    lwi->setData(Qt::ToolTipRole, fi.filePath());
    mlstwgt->addItem(lwi);
    connect(doc->findChild<QPlainTextEdit *>(), SIGNAL(textChanged()), mproxy, SLOT(trigger()));
}

void MacrosEditorModule::cedtrDocumentAboutToBeRemoved(BAbstractCodeEditorDocument *doc)
{
    if (!doc || mlstwgt.isNull())
        return;
    delete findItemByFileName(mlstwgt.data(), doc->fileName());
}

void MacrosEditorModule::cedtrCurrentDocumentFileNameChanged(const QString &fileName)
{
    if (mlstwgt.isNull())
        return;
    QListWidgetItem *lwi = mlstwgt->currentItem();
    if (!lwi)
        return;
    lwi->setText(QFileInfo(fileName).baseName());
    lwi->setData(Qt::ToolTipRole, fileName);
}
