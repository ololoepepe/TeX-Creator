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

class BSpellChecker;

class QWidget;

#include "mainwindow.h"
#include "consolewidget.h"
#include "symbolswidget.h"
#include "texsamplewidget.h"
#include "application.h"
#include "maindocumenteditormodule.h"
#include "global.h"

#include <BCodeEditor>
#include <BAbstractEditorModule>
#include <BIndicatorsEditorModule>
#include <BSearchEditorModule>
#include <BOpenSaveEditorModule>
#include <BEditEditorModule>
#include <BBookmarksEditorModule>
#include <BAbstractDocumentDriver>
#include <BLocalDocumentDriver>
#include <BAbstractFileType>
#include <BCodeEdit>
#include <BDirTools>
#include <BGuiTools>

#include <QString>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include <QMenuBar>
#include <QSizePolicy>
#include <QDockWidget>
#include <QStatusBar>
#include <QLabel>
#include <QStringList>
#include <QLayout>
#include <QSettings>
#include <QByteArray>
#include <QRect>
#include <QFile>
#include <QEvent>
#include <QMainWindow>
#include <QWindowStateChangeEvent>
#include <QApplication>
#include <QFileInfo>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>
#include <QSignalMapper>
#include <QToolBar>
#include <QIcon>
#include <QPoint>
#include <QScopedPointer>
#include <QProcess>
#include <QSize>
#include <QLocale>
#include <QToolButton>
#include <QMessageBox>
#include <QComboBox>
#include <QTextStream>
#include <QTimer>
#include <QCloseEvent>
#include <QTextBlock>
#include <QRegExp>
#include <QDesktopWidget>
#include <QPointer>

#include <QDebug>

/*============================================================================
================================ EditEditorModule ============================
============================================================================*/

class EditEditorModule : public BEditEditorModule
{
public:
    explicit EditEditorModule();
public:
    QString id() const;
    void setAutotextMenu(QMenu *mnu);
    void checkAutotext();
protected:
    void editorSet(BCodeEditor *edr);
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void documentCutAvailableChanged(bool available);
    void documentCopyAvailableChanged(bool available);
    void documentPasteAvailableChanged(bool available);
    void documentUndoAvailableChanged(bool available);
    void documentRedoAvailableChanged(bool available);
private:
    QPointer<QMenu> mmnuAutotext;
};

/*============================================================================
================================ LaTeXFileType ===============================
============================================================================*/

class LaTeXFileType : public BAbstractFileType
{
    Q_DECLARE_TR_FUNCTIONS(LaTeXFileType)
public:
    LaTeXFileType();
    ~LaTeXFileType();
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
    Q_DISABLE_COPY(LaTeXFileType)
};

/*============================================================================
================================ EditEditorModule ============================
============================================================================*/

/*============================== Public constructors =======================*/

EditEditorModule::EditEditorModule()
{
    //
}

/*============================== Public methods ============================*/

QString EditEditorModule::id() const
{
    return "edit";
}

void EditEditorModule::setAutotextMenu(QMenu *mnu)
{
    mmnuAutotext = mnu;
    checkAutotext();
}

void EditEditorModule::checkAutotext()
{
    if (mmnuAutotext.isNull() || mmnuAutotext->isEmpty())
        return;
    action(PasteAction)->setEnabled(currentDocument());
}

/*============================== Protected methods =========================*/

void EditEditorModule::editorSet(BCodeEditor *edr)
{
    BEditEditorModule::editorSet(edr);
    checkAutotext();
}

void EditEditorModule::currentDocumentChanged(BAbstractCodeEditorDocument *doc)
{
    BEditEditorModule::currentDocumentChanged(doc);
    checkAutotext();
}

void EditEditorModule::documentCutAvailableChanged(bool available)
{
    BEditEditorModule::documentCutAvailableChanged(available);
    checkAutotext();
}

void EditEditorModule::documentCopyAvailableChanged(bool available)
{
    BEditEditorModule::documentCopyAvailableChanged(available);
    checkAutotext();
}

void EditEditorModule::documentPasteAvailableChanged(bool available)
{
    BEditEditorModule::documentPasteAvailableChanged(available);
    checkAutotext();
}


void EditEditorModule::documentUndoAvailableChanged(bool available)
{
    BEditEditorModule::documentUndoAvailableChanged(available);
    checkAutotext();
}

void EditEditorModule::documentRedoAvailableChanged(bool available)
{
    BEditEditorModule::documentRedoAvailableChanged(available);
    checkAutotext();
}

/*============================================================================
================================ LaTeXFileType ===============================
============================================================================*/

/*============================== Public constructors =======================*/

LaTeXFileType::LaTeXFileType()
{
    //
}

LaTeXFileType::~LaTeXFileType()
{
    //
}

/*============================== Public methods ============================*/

QString LaTeXFileType::id() const
{
    return "LaTeX";
}

QString LaTeXFileType::name() const
{
    return "LaTeX"; //No need to translate
}

QString LaTeXFileType::description() const
{
    return tr("LaTeX files", "description");
}

QStringList LaTeXFileType::suffixes() const
{
    return QStringList() << "tex" << "inp" << "pic" << "sty";
}

bool LaTeXFileType::matchesFileName(const QString &fileName) const
{
    return suffixes().contains(QFileInfo(fileName).suffix(), Qt::CaseInsensitive);
}

BAbstractFileType::BracketPairList LaTeXFileType::brackets() const
{
    BracketPairList list;
    list << createBracketPair("{", "}", "\\");
    return list;
}

/*============================== Protected methods =========================*/

void LaTeXFileType::highlightBlock(const QString &text)
{
    //comments
    int comInd = text.indexOf('%');
    while (comInd > 0 && text.at(comInd - 1) == '\\')
        comInd = text.indexOf('%', comInd + 1);
    setCurrentBlockSkipIntervals();
    addCurrentBlockSkipInterval(comInd);
    if (comInd >= 0)
        setFormat(comInd, text.length() - comInd, QColor(Qt::darkGray));
    QString ntext = text.left(comInd);
    //commands
    QRegExp rx("(\\\\[a-zA-Z]*|\\\\#|\\\\\\$|\\\\%|\\\\&|\\\\_|\\\\\\{|\\\\\\})+");
    int pos = rx.indexIn(ntext);
    while (pos >= 0)
    {
        int len = rx.matchedLength();
        setFormat(pos, len, QColor(Qt::red).lighter(70));
        pos = rx.indexIn(ntext, pos + len);
    }
    //multiline (math mode)
    setCurrentBlockState(!ntext.isEmpty() ? 0 : previousBlockState());
    int startIndex = 0;
    bool firstIsStart = false;
    if (previousBlockState() != 1)
    {
        startIndex = Global::indexOfHelper(ntext, "$");
        firstIsStart = true;
    }
    while (startIndex >= 0)
    {
        int endIndex = Global::indexOfHelper(ntext, "$", startIndex + (firstIsStart ? 1 : 0));
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = ntext.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + 1;
        }
        setFormat(startIndex, commentLength, QColor(Qt::darkGreen));
        startIndex = Global::indexOfHelper(ntext, "$", startIndex + commentLength);
    }
}

/*============================================================================
================================ MainWindow ==================================
============================================================================*/

/*============================== Public constructors =======================*/

MainWindow::MainWindow() :
    QMainWindow(0)
{
    setAcceptDrops(true);
    setDockOptions(dockOptions() | QMainWindow::ForceTabbedDocks);
    setGeometry(QApplication::desktop()->availableGeometry().adjusted(100, 100, -100, -100)); //The default
    restoreGeometry(getWindowGeometry());
    //
    mmprAutotext = new QSignalMapper(this);
    mmprOpenFile = new QSignalMapper(this);
    connect(mmprOpenFile, SIGNAL(mapped(QString)), bApp, SLOT(openLocalFile(QString)));
    connect(bApp, SIGNAL(reloadAutotexts()), this, SLOT(reloadAutotext()));
    //
    initCodeEditor();
    initDockWidgets();
    initMenus();
    retranslateUi();
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
    updateWindowTitle( QString() );
    restoreState(getWindowState());
}

MainWindow::~MainWindow()
{
    //
}

/*============================== Static public methods =====================*/

QByteArray MainWindow::getWindowGeometry()
{
    return bSettings->value("MainWindow/geomery").toByteArray();
}

QByteArray MainWindow::getWindowState()
{
    return bSettings->value("MainWindow/state").toByteArray();
}

void MainWindow::setWindowGeometry(const QByteArray &geometry)
{
    bSettings->setValue("MainWindow/geomery", geometry);
}

void MainWindow::setWindowState(const QByteArray &state)
{
    bSettings->setValue("MainWindow/state", state);
}

/*============================== Public methods ============================*/

BCodeEditor *MainWindow::codeEditor() const
{
    return mcedtr;
}

ConsoleWidget *MainWindow::consoleWidget() const
{
    return mconsoleWgt;
}

/*============================== Protected methods =========================*/

void MainWindow::closeEvent(QCloseEvent *e)
{
    setWindowGeometry(saveGeometry());
    setWindowState(saveState());
    Global::setDocumentDriverState(mcedtr->driver()->saveState());
    Global::setSearchModuleState(mcedtr->module(BCodeEditor::SearchModule)->saveState());
    Application::windowAboutToClose(this);
    return QMainWindow::closeEvent(e);
}

/*============================== Private methods ===========================*/

void MainWindow::initCodeEditor()
{
    mcedtr = new BCodeEditor(Global::editorDocumentType(), this);
    mcedtr->setMaximumFileSize(Global::maxDocumentSize());
    if (!Global::editorSpellCheckEnabled())
        mcedtr->setSpellChecker(Application::spellChecker());
    mcedtr->removeModule(mcedtr->module(BCodeEditor::EditModule));
    mcedtr->addModule(new EditEditorModule);
    mcedtr->addModule(BCodeEditor::BookmarksModule);
    mcedtr->addModule(new MainDocumentEditorModule);
    mcedtr->addFileType(new LaTeXFileType);
    mcedtr->setPreferredFileType("LaTeX");
    mcedtr->setEditFont(Global::editFont());
    mcedtr->setDefaultCodec(Global::defaultCodec());
    mcedtr->setEditLineLength(Global::editLineLength());
    mcedtr->setEditTabWidth(Global::editTabWidth());
    mcedtr->setFileHistory(Global::fileHistory());
    mcedtr->driver()->restoreState(Global::documentDriverState());
    mcedtr->module(BCodeEditor::SearchModule)->restoreState(Global::searchModuleState());
    //
    connect(mcedtr, SIGNAL(currentDocumentModificationChanged(bool)), this, SLOT(setWindowModified(bool)));
    connect(mcedtr, SIGNAL(currentDocumentFileNameChanged(QString)), this, SLOT(updateWindowTitle(QString)));
    connect(static_cast<BSearchEditorModule *>(mcedtr->module(BCodeEditor::SearchModule)), SIGNAL(message(QString)),
            statusBar(), SLOT(showMessage(QString)));
    connect(mmprAutotext, SIGNAL(mapped(QString)), mcedtr, SLOT(insertTextIntoCurrentDocument(QString)));
    setCentralWidget(mcedtr);
    installEventFilter(mcedtr->dropHandler());
    installEventFilter(mcedtr->closeHandler());
    BAbstractEditorModule *mdl = mcedtr->module(BCodeEditor::IndicatorsModule);
    statusBar()->addPermanentWidget(mdl->widget(BIndicatorsEditorModule::FileTypeIndicator));
    statusBar()->addPermanentWidget(mdl->widget(BIndicatorsEditorModule::CursorPositionIndicator));
    statusBar()->addPermanentWidget(mdl->widget(BIndicatorsEditorModule::EncodingIndicator));
}

void MainWindow::initDockWidgets()
{
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    //Symbols
    msymbolsWgt = new SymbolsWidget;
      connect(msymbolsWgt, SIGNAL(insertText(QString)), mcedtr, SLOT(insertTextIntoCurrentDocument(QString)));
    QDockWidget *dwgt = new QDockWidget;
      dwgt->setObjectName("DockWidgetSymbols");
      dwgt->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
      dwgt->setWidget(msymbolsWgt);
    addDockWidget(Qt::LeftDockWidgetArea, dwgt);
    //Samples
    mtexsampleWgt = new TexsampleWidget(this);
    connect(mtexsampleWgt, SIGNAL(message(QString)), this->statusBar(), SLOT(showMessage(QString)));
    statusBar()->insertPermanentWidget(0, mtexsampleWgt->indicator());
    dwgt = new QDockWidget;
      dwgt->setObjectName("DockWidgeSamples");
      dwgt->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
      dwgt->setWidget(mtexsampleWgt);
    addDockWidget(Qt::RightDockWidgetArea, dwgt);
    //Console
    mconsoleWgt = new ConsoleWidget(mcedtr);
    dwgt = new QDockWidget;
      dwgt->setObjectName("DockWidgetConsole");
      dwgt->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
      dwgt->setWidget(mconsoleWgt);
    addDockWidget(Qt::BottomDockWidgetArea, dwgt);
}

void MainWindow::initMenus()
{
    BAbstractEditorModule *osmdl = mcedtr->module(BCodeEditor::OpenSaveModule);
    BAbstractEditorModule *emdl = mcedtr->module("edit");
    BAbstractEditorModule *smdl = mcedtr->module(BCodeEditor::SearchModule);
    BAbstractEditorModule *bmdl = mcedtr->module(BCodeEditor::BookmarksModule);
    BAbstractEditorModule *mdmdl = mcedtr->module("main_document");
    //File
    mmnuFile = menuBar()->addMenu("");
    mmnuFile->setObjectName("MenuFile");
    mmnuFile->addActions(osmdl->actions(BOpenSaveEditorModule::OpenActionGroup, true));
    mmnuFile->addMenu(static_cast<BOpenSaveEditorModule *>(osmdl)->fileHistoryMenu());
    mmnuFile->addSeparator();
    mmnuFile->addActions(osmdl->actions(BOpenSaveEditorModule::SaveActionGroup, true));
    mmnuFile->addSeparator();
    mmnuFile->addActions(osmdl->actions(BOpenSaveEditorModule::CloseActionGroup, true));
    mmnuFile->addSeparator();
    mactQuit = mmnuFile->addAction("");
    mactQuit->setObjectName("ActionQuit");
    mactQuit->setMenuRole(QAction::QuitRole);
    mactQuit->setIcon(BApplication::icon("exit"));
    mactQuit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(mactQuit, SIGNAL(triggered()), this, SLOT(close()));
    //Edit
    mmnuEdit = menuBar()->addMenu("");
    mmnuEdit->setObjectName("MenuEdit");
    mmnuEdit->addActions(emdl->actions(BEditEditorModule::UndoRedoActionGroup));
    mmnuEdit->addSeparator();
    mmnuEdit->addActions(emdl->actions(BEditEditorModule::ClipboardActionGroup));
    mmnuAutotext = mmnuEdit->addMenu(Application::icon("editpaste"), "");
    mmnuAutotext->setObjectName("MenuAutotext");
    static_cast<EditEditorModule *>(emdl)->setAutotextMenu(mmnuAutotext);
    reloadAutotext();
    mmnuEdit->addSeparator();
    mmnuEdit->addSeparator();
    mmnuEdit->addActions(smdl->actions());
    mmnuEdit->addSeparator();
    QAction *act = BGuiTools::createStandardAction(BGuiTools::SettingsAction);
    act->setShortcut(QKeySequence("Ctrl+P"));
    mmnuEdit->addAction(act);
    //Document
    mmnuDocument = menuBar()->addMenu("");
    mmnuDocument->setObjectName("MenuDocument");
    mmnuDocument->addActions(bmdl->actions());
    mmnuDocument->addSeparator();
    mmnuDocument->addActions(mdmdl->actions());
    mmnuDocument->addSeparator();
    mmnuDocument->addAction(emdl->action(BEditEditorModule::SwitchModeAction));
    emdl->action(BEditEditorModule::SwitchModeAction)->setShortcut(QKeySequence("Ctrl+Shift+B"));
    mactSpellCheck = mmnuDocument->addAction("");
    mactSpellCheck->setObjectName("ActionSpellCheck");
    connect(mactSpellCheck, SIGNAL(triggered()), this, SLOT(switchSpellCheck()));
    switchSpellCheck();
    //View
    mmnuView = menuBar()->addMenu("");
    mmnuView->setObjectName("MenuView");
    //Console
    mmnuConsole = menuBar()->addMenu("");
    mmnuConsole->addActions(mconsoleWgt->consoleActions(true));
    mmnuConsole->setObjectName("MenuConsole");
    //Tools
    mmnuTools = menuBar()->addMenu("");
    mmnuTools->setObjectName("MenuTools");
    mactOpenAutotextUserFolder = mmnuTools->addAction("");
    mactOpenAutotextUserFolder->setObjectName("MenuOpenAutotextUserFolder");
    mactOpenAutotextUserFolder->setIcon(Application::icon("folder_open"));
    bSetMapping(mmprOpenFile, mactOpenAutotextUserFolder, SIGNAL(triggered()),
                Application::location("autotext", BApplication::UserResource));
    //Texsample
    mmnuTexsample = menuBar()->addMenu("");
    mmnuTexsample->setObjectName("MenuTexsample");
    mmnuTexsample->addActions(mtexsampleWgt->toolBarActions());
    //Help
    mmnuHelp = menuBar()->addMenu("");
    mmnuHelp->setObjectName("MenuHelp");
    mmnuHelp->addAction(BGuiTools::createStandardAction(BGuiTools::HomepageAction));
    mmnuHelp->addSeparator();
    act = BGuiTools::createStandardAction(BGuiTools::HelpContentsAction);
    act->setShortcut(QKeySequence("F1"));
    mmnuHelp->addAction(act);
    mmnuHelp->addAction(BGuiTools::createStandardAction(BGuiTools::WhatsThisAction));
    mmnuHelp->addSeparator();
    mmnuHelp->addAction(BGuiTools::createStandardAction(BGuiTools::AboutAction));
    //Toolbars
    mtbarOpen = addToolBar("");
    mtbarOpen->setObjectName("ToolBarOpen");
    mtbarOpen->addActions(osmdl->actions(BOpenSaveEditorModule::OpenActionGroup));
    mtbarSave = addToolBar("");
    mtbarSave->setObjectName("ToolBarSave");
    mtbarSave->addActions(osmdl->actions(BOpenSaveEditorModule::SaveActionGroup));
    mtbarUndoRedo = addToolBar("");
    mtbarUndoRedo->setObjectName("ToolBarUndoRedo");
    mtbarUndoRedo->addActions(emdl->actions(BEditEditorModule::UndoRedoActionGroup));
    mtbarClipboard = addToolBar("");
    mtbarClipboard->setObjectName("ToolBarClipboard");
    mtbarClipboard->addActions(emdl->actions(BEditEditorModule::ClipboardActionGroup));
    QToolButton *tbtn = BGuiTools::toolButtonForAction(mtbarClipboard, emdl->action(BEditEditorModule::PasteAction));
    tbtn->setMenu(mmnuAutotext);
    tbtn->setPopupMode(QToolButton::MenuButtonPopup);
    mtbarDocument = addToolBar("");
    mtbarDocument->setObjectName("ToolBarDocument");
    mtbarDocument->addActions(bmdl->actions());
    mtbarDocument->addSeparator();
    mtbarDocument->addActions(mdmdl->actions());
    mtbarDocument->addSeparator();
    mtbarDocument->addAction(emdl->action(BEditEditorModule::SwitchModeAction));
    mtbarDocument->addAction(mactSpellCheck);
    mtbarSearch = addToolBar("");
    mtbarSearch->setObjectName("ToolBarSearch");
    mtbarSearch->addActions(smdl->actions());
}

void MainWindow::retranslateActSpellCheck()
{
    if (mcedtr->spellChecker())
    {
        mactSpellCheck->setText(tr("Spell check: enabled", "act text"));
        mactSpellCheck->setToolTip(tr("Disable spell check", "act toolTip"));
    }
    else
    {
        mactSpellCheck->setText(tr("Spell check: disabled", "act text"));
        mactSpellCheck->setToolTip(tr("Enable spell check", "act toolTip"));
    }
}

/*============================== Private slots =============================*/

void MainWindow::retranslateUi()
{
    //code editor
    mcedtr->setDefaultFileName(tr("New document.tex", "cedtr defaultFileName"));
    //dock widgets
    msymbolsWgt->parentWidget()->setWindowTitle(tr("LaTeX symbols", "dwgt windowTitle"));
    mtexsampleWgt->parentWidget()->setWindowTitle(tr("TeXSample client", "dwgt windowTitle"));
    mconsoleWgt->parentWidget()->setWindowTitle(tr("Console", "dwgt windowTitle"));
    //menus
    mmnuFile->setTitle(tr("File", "mnu title"));
    mactQuit->setText(tr("Quit", "act text"));
    mmnuEdit->setTitle(tr("Edit", "mnu title"));
    mmnuAutotext->setTitle(tr("Insert autotext", "mnu title"));
    mmnuView->setTitle(tr("View", "mnu title"));
    mmnuConsole->setTitle(tr("Console", "mnu title"));
    mmnuTools->setTitle(tr("Tools", "mnu title"));
    mactOpenAutotextUserFolder->setText(tr("Open user autotext folder", "act text"));
    mmnuDocument->setTitle(tr("Document", "mnu title"));
    retranslateActSpellCheck();
    mmnuTexsample->setTitle(tr("TeXSample", "mnuTitle"));
    mmnuHelp->setTitle(tr("Help", "mnuTitle"));
    //toolbars
    mtbarOpen->setWindowTitle(tr("Open", "tbar windowTitle"));
    mtbarSave->setWindowTitle(tr("Save", "tbar windowTitle") );
    mtbarUndoRedo->setWindowTitle(tr("Undo/Redo", "tbar windowTitle"));
    mtbarClipboard->setWindowTitle(tr("Clipboard", "tbar windowTitle"));
    mtbarDocument->setWindowTitle(tr("Document", "tbar windowTitle"));
    mtbarSearch->setWindowTitle(tr("Search", "tbar windowTitle"));
    //menu view
    mmnuView->clear();
    QMenu *mnu = createPopupMenu();
    if (!mnu)
        return;
    mmnuView->addActions(mnu->actions());
    mnu->deleteLater();
}

void MainWindow::updateWindowTitle(const QString &fileName)
{
    if ( !fileName.isEmpty() )
    {
        setWindowTitle("");
        setWindowFilePath(fileName);
    }
    else
    {
        setWindowFilePath("");
        setWindowTitle( QApplication::applicationName() );
    }
}

void MainWindow::reloadAutotext()
{
    mmnuAutotext->clear();
    QStringList list;
    foreach ( const QString &path, Application::locations("autotext") )
    {
        foreach ( const QString &fn, QDir(path).entryList(QStringList() << "*.txt", QDir::Files) )
        {
            if ( list.contains(fn) )
                continue;
            list << fn;
            QString text = BDirTools::readTextFile(path + "/" + fn, "UTF-8");
            if ( text.isEmpty() )
                continue;
            bSetMapping(mmprAutotext, mmnuAutotext->addAction(QFileInfo(fn).baseName()), SIGNAL(triggered()), text);
        }
    }
    static_cast<EditEditorModule *>(mcedtr->module("edit"))->checkAutotext();
}

void MainWindow::switchSpellCheck()
{
    if (mcedtr->spellChecker())
    {
        mcedtr->setSpellChecker(0);
        mactSpellCheck->setIcon(Application::icon("spellcheck_disabled"));
    }
    else
    {
        mcedtr->setSpellChecker(Application::spellChecker());
        mactSpellCheck->setIcon(Application::icon("spellcheck"));
    }
    Global::setEditorSpellCheckEnabled(mcedtr->spellChecker());
    retranslateActSpellCheck();
}
