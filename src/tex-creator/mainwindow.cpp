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

#include "mainwindow.h"

#include "application.h"
#include "consolewidget.h"
#include "editeditormodule.h"
#include "latexfiletype.h"
#include "maindocumenteditormodule.h"
#include "settings.h"
#include "symbolswidget.h"
#include "texsample/texsamplewidget.h"

#include <BAbstractDocumentDriver>
#include <BAbstractEditorModule>
#include <BAbstractFileType>
#include <BBookmarksEditorModule>
#include <BCodeEditor>
#include <BDirTools>
#include <BEditEditorModule>
#include <BGuiTools>
#include <BIndicatorsEditorModule>
#include <BOpenSaveEditorModule>
#include <BSearchEditorModule>

#include <QAction>
#include <QByteArray>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QIcon>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QRect>
#include <QSignalMapper>
#include <QStatusBar>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

/*============================================================================
================================ MainWindow ==================================
============================================================================*/

/*============================== Public constructors =======================*/

MainWindow::MainWindow() :
    QMainWindow(0)
{
    setAcceptDrops(true);
    setDockOptions(dockOptions() | QMainWindow::ForceTabbedDocks);
    setGeometry(Application::desktop()->availableGeometry().adjusted(100, 100, -100, -100)); //The default
    restoreGeometry(Settings::MainWindow::windowGeometry());
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
    updateWindowTitle(QString());
    //NOTE: Qt bug. Window state not restored without some delay (500 ms should be enough)
    QTimer::singleShot(500, this, SLOT(restoreStateWorkaround()));
}

MainWindow::~MainWindow()
{
    //
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

/*============================== Public slots ==============================*/

void MainWindow::showStatusBarMessage(const QString &message)
{
    statusBar()->showMessage(message);
}

/*============================== Protected methods =========================*/

void MainWindow::closeEvent(QCloseEvent *e)
{
    Settings::MainWindow::setWindowGeometry(saveGeometry());
    Settings::MainWindow::setWindowState(saveState());
    Settings::CodeEditor::setSpellCheckEnabled(mcedtr->spellChecker());
    Settings::CodeEditor::setDocumentDriverState(mcedtr->driver()->saveState());
    Settings::CodeEditor::setSearchModuleState(mcedtr->module(BCodeEditor::SearchModule)->saveState());
    Application::windowAboutToClose(this);
    return QMainWindow::closeEvent(e);
}

/*============================== Private methods ===========================*/

void MainWindow::initCodeEditor()
{
    mcedtr = new BCodeEditor(Settings::CodeEditor::documentType(), this);
    mcedtr->setMaximumFileSize(Settings::CodeEditor::maximumFileSize());
    if (Settings::CodeEditor::spellCheckEnabled())
        mcedtr->setSpellChecker(bApp->spellChecker());
    mcedtr->removeModule(mcedtr->module(BCodeEditor::EditModule));
    mcedtr->addModule(new EditEditorModule);
    mcedtr->addModule(BCodeEditor::BookmarksModule);
    mcedtr->addModule(new MainDocumentEditorModule);
    mcedtr->addFileType(new LatexFileType);
    mcedtr->setPreferredFileType("LaTeX");
    mcedtr->setEditFont(Settings::CodeEditor::editFont());
    mcedtr->setDefaultCodec(Settings::CodeEditor::defaultCodec());
    mcedtr->setEditLineLength(Settings::CodeEditor::editLineLength());
    mcedtr->setEditTabWidth(Settings::CodeEditor::editTabWidth());
    mcedtr->setFileHistory(Settings::CodeEditor::fileHistory());
    mcedtr->driver()->restoreState(Settings::CodeEditor::documentDriverState());
    mcedtr->module(BCodeEditor::SearchModule)->restoreState(Settings::CodeEditor::searchModuleState());
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
    mactQuit->setIcon(Application::icon("exit"));
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
                Application::location("autotext", Application::UserResource));
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

void MainWindow::resetActSpellCheck()
{
    if (mcedtr->spellChecker()) {
        mactSpellCheck->setIcon(Application::icon("spellcheck"));
        mactSpellCheck->setText(tr("Spell check: enabled", "act text"));
        mactSpellCheck->setToolTip(tr("Disable spell check", "act toolTip"));
    } else {
        mactSpellCheck->setIcon(Application::icon("spellcheck_disabled"));
        mactSpellCheck->setText(tr("Spell check: disabled", "act text"));
        mactSpellCheck->setToolTip(tr("Enable spell check", "act toolTip"));
    }
}

/*============================== Private slots =============================*/

void MainWindow::reloadAutotext()
{
    mmnuAutotext->clear();
    QStringList list;
    foreach (const QString &path, Application::locations("autotext")) {
        foreach (const QString &fn, QDir(path).entryList(QStringList() << "*.txt", QDir::Files)) {
            if (list.contains(fn))
                continue;
            list << fn;
            QString text = BDirTools::readTextFile(path + "/" + fn, "UTF-8");
            if (text.isEmpty())
                continue;
            bSetMapping(mmprAutotext, mmnuAutotext->addAction(QFileInfo(fn).baseName()), SIGNAL(triggered()), text);
        }
    }
    static_cast<EditEditorModule *>(mcedtr->module("edit"))->checkAutotext();
}

void MainWindow::restoreStateWorkaround()
{
    restoreState(Settings::MainWindow::windowState());
}

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
    resetActSpellCheck();
    mmnuTexsample->setTitle(tr("TeXSample", "mnuTitle"));
    mmnuHelp->setTitle(tr("Help", "mnuTitle"));
    //toolbars
    mtbarOpen->setWindowTitle(tr("Open", "tbar windowTitle"));
    mtbarSave->setWindowTitle(tr("Save", "tbar windowTitle"));
    mtbarUndoRedo->setWindowTitle(tr("Undo/Redo", "tbar windowTitle"));
    mtbarClipboard->setWindowTitle(tr("Clipboard", "tbar windowTitle"));
    mtbarDocument->setWindowTitle(tr("Document", "tbar windowTitle"));
    mtbarSearch->setWindowTitle(tr("Search", "tbar windowTitle"));
    //menu view
    mmnuView->clear();
    QMenu *mnu = createPopupMenu();
    if (mnu) {
        mmnuView->addActions(mnu->actions());
        delete mnu;
    }
}

void MainWindow::switchSpellCheck()
{
    if (mcedtr->spellChecker())
        mcedtr->setSpellChecker(0);
    else
        mcedtr->setSpellChecker(bApp->spellChecker());
    Settings::CodeEditor::setSpellCheckEnabled(mcedtr->spellChecker());
    resetActSpellCheck();
}

void MainWindow::updateWindowTitle(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        setWindowTitle("");
        setWindowFilePath(fileName);
    } else {
        setWindowFilePath("");
        setWindowTitle(Application::applicationName());
    }
}
