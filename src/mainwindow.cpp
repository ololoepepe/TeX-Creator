class QWidget;

#include "mainwindow.h"
#include "consolewidget.h"
#include "symbolswidget.h"
#include "sampleswidget.h"
#include "application.h"
#include "maindocumenteditormodule.h"
#include "codeeditorsettingstab.h"

#include <BCodeEditor>
#include <BAbstractEditorModule>
#include <BIndicatorsEditorModule>
#include <BSearchEditorModule>
#include <BOpenSaveEditorModule>
#include <BEditEditorModule>
#include <BBookmarksEditorModule>
#include <BAbstractDocumentDriver>
#include <BLocalDocumentDirver>
#include <BAbstractFileType>
#include <BCodeEdit>

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
#include <QScopedPointer>
#include <QLocale>
#include <QLayout>
#include <QToolButton>
#include <QLayout>
#include <QMessageBox>
#include <QComboBox>
#include <QTextStream>
#include <QTimer>
#include <QCloseEvent>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QRegExp>
#include <QDesktopWidget>

#include <QDebug>

/*============================================================================
================================ LaTeXHighlighter ============================
============================================================================*/

class LaTeXHighlighter : public QSyntaxHighlighter
{
public:
    explicit LaTeXHighlighter(QObject *parent);
    ~LaTeXHighlighter();
protected:
    void highlightBlock(const QString &text);
private:
    Q_DISABLE_COPY(LaTeXHighlighter)
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
    QSyntaxHighlighter *createHighlighter(QObject *parent) const;
    QList<BCodeEdit::BracketPair> brackets() const;
private:
    Q_DISABLE_COPY(LaTeXFileType)
};

/*============================================================================
================================ LaTeXHighlighter ============================
============================================================================*/

/*============================== Public constructors =======================*/

LaTeXHighlighter::LaTeXHighlighter(QObject *parent) :
    QSyntaxHighlighter(parent)
{
    //
}

LaTeXHighlighter::~LaTeXHighlighter()
{
    //
}

/*============================== Public methods ============================*/

void LaTeXHighlighter::highlightBlock(const QString &text)
{
    //comments
    int comInd = text.indexOf("%");
    BCodeEdit::setBlockComment(currentBlock(), comInd);
    if (comInd >= 0)
        setFormat( comInd, text.length() - comInd, QColor(Qt::darkGray) );
    QString ntext = text.left(comInd);
    //commands
    QRegExp rx("(\\\\[a-zA-Z]*|\\\\#|\\\\\\$|\\\\%|\\\\&|\\\\_|\\\\\\{|\\\\\\})+");
    int pos = rx.indexIn(ntext);
    while (pos >= 0)
    {
        int len = rx.matchedLength();
        setFormat( pos, len, QColor(Qt::darkRed) );
        pos = rx.indexIn(ntext, pos + len);
    }
    //multiline (math mode)
    setCurrentBlockState( !ntext.isEmpty() ? 0 : previousBlockState() );
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = ntext.indexOf('$');
    while (startIndex >= 0)
    {
        int endIndex = ntext.indexOf('$', startIndex + 1);
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
        setFormat( startIndex, commentLength, QColor(Qt::darkGreen) );
        startIndex = ntext.indexOf('$', startIndex + commentLength);
    }
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

QSyntaxHighlighter *LaTeXFileType::createHighlighter(QObject *parent) const
{
    return new LaTeXHighlighter(parent);
}

QList<BCodeEdit::BracketPair> LaTeXFileType::brackets() const
{
    QList<BCodeEdit::BracketPair> list;
    list << createBracketPair("{", "}", "\\");
    return list;
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
    setGeometry( QApplication::desktop()->availableGeometry().adjusted(100, 100, -100, -100) ); //The default
    restoreGeometry( getWindowGeometry() );
    restoreState( getWindowState() );
    //
    mmprAutotext = new QSignalMapper(this);
    mmprOpenFile = new QSignalMapper(this);
    connect( mmprOpenFile, SIGNAL( mapped(QString) ), bApp, SLOT( openLocalFile(QString) ) );
    //
    initCodeEditor();
    initDockWidgets();
    initMenus();
    retranslateUi();
    connect( bApp, SIGNAL( languageChanged() ), this, SLOT( retranslateUi() ) );
    updateWindowTitle( QString() );
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
    return cedtr;
}

ConsoleWidget *MainWindow::consoleWidget() const
{
    return cwgt;
}

/*============================== Purotected methods ========================*/

void MainWindow::closeEvent(QCloseEvent *e)
{
    if ( cedtr->closeAllDocuments() )
    {
        setWindowGeometry( saveGeometry() );
        setWindowState( saveState() );
        CodeEditorSettingsTab::setDocumentDriverState( cedtr->driver()->saveState() );
        CodeEditorSettingsTab::setSearchModuleState( cedtr->module(BCodeEditor::SearchModule)->saveState() );
        return QMainWindow::closeEvent(e);
    }
    else
    {
        e->ignore();
    }
}

/*============================== Private methods ===========================*/

void MainWindow::initCodeEditor()
{
    cedtr = new BCodeEditor(this);
    cedtr->addModule(BCodeEditor::BookmarksModule);
    cedtr->addModule(new MainDocumentEditorModule);
    cedtr->addFileType(new LaTeXFileType);
    cedtr->setPreferredFileType("LaTeX");
    cedtr->setEditFont( CodeEditorSettingsTab::getEditFont() );
    cedtr->setDefaultCodec( CodeEditorSettingsTab::getDefaultCodec() );
    cedtr->setEditLineLength( CodeEditorSettingsTab::getEditLineLength() );
    cedtr->setEditTabWidth( CodeEditorSettingsTab::getEditTabWidth() );
    cedtr->setFileHistory( CodeEditorSettingsTab::getFileHistory() );
    cedtr->driver()->restoreState( CodeEditorSettingsTab::getDocumentDriverState() );
    cedtr->module(BCodeEditor::SearchModule)->restoreState( CodeEditorSettingsTab::getSearchModuleState() );
    //
    connect( cedtr, SIGNAL( currentDocumentModificationChanged(bool) ), this, SLOT( setWindowModified(bool) ) );
    connect( cedtr, SIGNAL( currentDocumentFileNameChanged(QString) ), this, SLOT( updateWindowTitle(QString) ) );
    connect( mmprAutotext, SIGNAL( mapped(QString) ), cedtr, SLOT( insertTextIntoCurrentDocument(QString) ) );
    setCentralWidget(cedtr);
    installEventFilter( cedtr->dropHandler() );
    BAbstractEditorModule *mdl = cedtr->module(BCodeEditor::IndicatorsModule);
    statusBar()->addPermanentWidget( mdl->widget(BIndicatorsEditorModule::FileTypeIndicator) );
    statusBar()->addPermanentWidget( mdl->widget(BIndicatorsEditorModule::CursorPositionIndicator) );
    statusBar()->addPermanentWidget( mdl->widget(BIndicatorsEditorModule::EncodingIndicator) );
}

void MainWindow::initDockWidgets()
{
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    //
    swgt = new SymbolsWidget;
      connect( swgt, SIGNAL( insertText(QString) ), cedtr, SLOT( insertTextIntoCurrentDocument(QString) ) );
    dwgtSymbols = new QDockWidget;
      dwgtSymbols->setObjectName("DockWidgetSymbols");
      dwgtSymbols->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
      dwgtSymbols->setWidget(swgt);
    addDockWidget(Qt::LeftDockWidgetArea, dwgtSymbols);
    //
    smpwgt = new SamplesWidget(this);
    dwgtSamples = new QDockWidget;
      dwgtSamples->setObjectName("DockWidgeSamples");
      dwgtSamples->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
      dwgtSamples->setWidget(smpwgt);
    addDockWidget(Qt::RightDockWidgetArea, dwgtSamples);
    //
    cwgt = new ConsoleWidget(cedtr);
    dwgtConsole = new QDockWidget;
      dwgtConsole->setObjectName("DockWidgetConsole");
      dwgtConsole->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
      dwgtConsole->setWidget(cwgt);
    addDockWidget(Qt::BottomDockWidgetArea, dwgtConsole);
}

void MainWindow::initMenus()
{
    BAbstractEditorModule *osmdl = cedtr->module(BCodeEditor::OpenSaveModule);
    BAbstractEditorModule *emdl = cedtr->module(BCodeEditor::EditModule);
    BAbstractEditorModule *smdl = cedtr->module(BCodeEditor::SearchModule);
    BAbstractEditorModule *bmdl = cedtr->module(BCodeEditor::BookmarksModule);
    BAbstractEditorModule *mdmdl = cedtr->module("main_document");
    //File
    mnuFile = menuBar()->addMenu("");
    mnuFile->addActions( osmdl->actions(BOpenSaveEditorModule::OpenActionGroup, true) );
    mnuFile->addMenu( static_cast<BOpenSaveEditorModule *>(osmdl)->fileHistoryMenu() );
    mnuFile->addSeparator();
    mnuFile->addActions( osmdl->actions(BOpenSaveEditorModule::SaveActionGroup, true) );
    mnuFile->addSeparator();
    mnuFile->addActions( osmdl->actions(BOpenSaveEditorModule::CloseActionGroup, true) );
    mnuFile->addSeparator();
    actQuit = mnuFile->addAction("");
    actQuit->setMenuRole(QAction::QuitRole);
    actQuit->setIcon( BApplication::icon("exit") );
    actQuit->setShortcut( QKeySequence("Ctrl+Q") );
    connect( actQuit, SIGNAL( triggered() ), this, SLOT( close() ) );
    //Edit
    mnuEdit = menuBar()->addMenu("");
    mnuEdit->addActions( emdl->actions(BEditEditorModule::UndoRedoActionGroup) );
    mnuEdit->addSeparator();
    mnuEdit->addActions( emdl->actions(BEditEditorModule::ClipboardActionGroup) );
    mmnuAutotext = mnuEdit->addMenu(Application::icon("editpaste"), "");
    checkAutotextMenu( cedtr->currentDocument() );
    reloadAutotext();
    connect( cedtr, SIGNAL( documentAvailableChanged(bool) ), this, SLOT( checkAutotextMenu(bool) ) );
    mnuEdit->addSeparator();
    mnuEdit->addActions( smdl->actions() );
    mnuEdit->addSeparator();
    QAction *act = BApplication::createStandardAction(BApplication::SettingsAction);
    act->setShortcut( QKeySequence("Ctrl+P") );
    mnuEdit->addAction(act);
    //Document
    mmnuDocument = menuBar()->addMenu("");
    mmnuDocument->addActions( bmdl->actions() );
    mmnuDocument->addSeparator();
    mmnuDocument->addActions( mdmdl->actions() );
    mmnuDocument->addSeparator();
    mmnuDocument->addAction( emdl->action(BEditEditorModule::SwitchModeAction) );
    emdl->action(BEditEditorModule::SwitchModeAction)->setShortcut( QKeySequence("Ctrl+Shift+B") );
    //View
    mnuView = menuBar()->addMenu("");
    //Tools
    mmnuTools = menuBar()->addMenu("");
    mmnuTools->addActions( cwgt->consoleActions() );
    mmnuTools->addSeparator();
    mactReloadAutotext = new QAction(this);
    mactReloadAutotext->setIcon( Application::icon("reload") );
    connect( mactReloadAutotext, SIGNAL( triggered() ), this, SLOT( reloadAutotext() ) );
    mmnuTools->addAction(mactReloadAutotext);
    mmnuTools->addSeparator();
    mactOpenAutotextUserFolder = mmnuTools->addAction("");
    Application::setMapping( mmprOpenFile, mactOpenAutotextUserFolder, SIGNAL( triggered() ),
                             Application::location("autotext", BApplication::UserResources) );
    //Texsample
    mnuTexsample = menuBar()->addMenu("");
    mnuTexsample->addActions( smpwgt->toolBarActions() );
    //Help
    mnuHelp = menuBar()->addMenu("");
    mnuHelp->addAction( BApplication::createStandardAction(BApplication::HomepageAction) );
    mnuHelp->addSeparator();
    act = BApplication::createStandardAction(BApplication::HelpContentsAction);
    act->setShortcut( QKeySequence("F1") );
    mnuHelp->addAction(act);
    mnuHelp->addAction( BApplication::createStandardAction(BApplication::WhatsThisAction) );
    mnuHelp->addSeparator();
    mnuHelp->addAction( BApplication::createStandardAction(BApplication::AboutAction) );
    //Toolbars
    tbarOpen = addToolBar("");
    tbarOpen->setObjectName("ToolBarOpen");
    tbarOpen->addActions( osmdl->actions(BOpenSaveEditorModule::OpenActionGroup) );
    tbarSave = addToolBar("");
    tbarSave->setObjectName("ToolBarSave");
    tbarSave->addActions( osmdl->actions(BOpenSaveEditorModule::SaveActionGroup) );
    tbarUndoRedo = addToolBar("");
    tbarUndoRedo->setObjectName("ToolBarUndoRedo");
    tbarUndoRedo->addActions( emdl->actions(BEditEditorModule::UndoRedoActionGroup) );
    tbarClipboard = addToolBar("");
    tbarClipboard->setObjectName("ToolBarClipboard");
    tbarClipboard->addActions( emdl->actions(BEditEditorModule::ClipboardActionGroup) );
    QToolButton *tbtn = Application::toolButtonForAction( tbarClipboard,
                                                          emdl->action(BEditEditorModule::PasteAction) );
    tbtn->setMenu(mmnuAutotext);
    tbtn->setPopupMode(QToolButton::MenuButtonPopup);
    tbarDocument = addToolBar("");
    tbarDocument->setObjectName("ToolBarDocument");
    tbarDocument->addActions( bmdl->actions() );
    tbarDocument->addSeparator();
    tbarDocument->addActions( mdmdl->actions() );
    tbarDocument->addSeparator();
    tbarDocument->addAction( emdl->action(BEditEditorModule::SwitchModeAction) );
    tbarSearch = addToolBar("");
    tbarSearch->setObjectName("ToolBarSearch");
    tbarSearch->addActions( smdl->actions() );
}

/*============================== Private slots =============================*/

void MainWindow::retranslateUi()
{
    //code editor
    cedtr->setDefaultFileName( tr("New document.tex", "cedtr defaultFileName") );
    //dock widgets
    dwgtSymbols->setWindowTitle( tr("LaTeX symbols", "dwgt windowTitle") );
    dwgtSamples->setWindowTitle( tr("TeXSample viewer", "dwgt windowTitle") );
    dwgtConsole->setWindowTitle( tr("Console", "dwgt windowTitle") );
    //menus
    mnuFile->setTitle( tr("File", "mnu title") );
    actQuit->setText( tr("Quit", "act text") );
    mnuEdit->setTitle( tr("Edit", "mnu title") );
    mmnuAutotext->setTitle( tr("Insert autotext", "mnu title") );
    mnuView->setTitle( tr("View", "mnu title") );
    mmnuTools->setTitle( tr("Tools", "mnu title") );
    mactReloadAutotext->setText( tr("Reload autotext files", "act text") );
    mactOpenAutotextUserFolder->setText( tr("Open user autotext folder", "act text") );
    mmnuDocument->setTitle( tr("Document", "mnu title") );
    mnuTexsample->setTitle( tr("TeXSample", "mnuTitle") );
    mnuHelp->setTitle( tr("Help", "mnuTitle") );
    //toolbars
    tbarOpen->setWindowTitle( tr("Open", "tbar windowTitle") );
    tbarSave->setWindowTitle( tr("Save", "tbar windowTitle") );
    tbarUndoRedo->setWindowTitle( tr("Undo/Redo", "tbar windowTitle") );
    tbarClipboard->setWindowTitle( tr("Clipboard", "tbar windowTitle") );
    tbarDocument->setWindowTitle( tr("Document", "tbar windowTitle") );
    tbarSearch->setWindowTitle( tr("Search", "tbar windowTitle") );
    //menu view
    mnuView->clear();
    QMenu *mnu = createPopupMenu();
    mnuView->addActions( mnu->actions() );
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

void MainWindow::checkAutotextMenu(bool documentAvailable)
{
    mmnuAutotext->setEnabled( documentAvailable && !mmnuAutotext->isEmpty() );
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
            QFile f(path + "/" + fn);
            if ( !f.open(QFile::ReadOnly) )
                continue;
            QTextStream in(&f);
            in.setCodec("UTF-8");
            QString text = in.readAll();
            f.close();
            if ( text.isEmpty() )
                continue;
            Application::setMapping(mmprAutotext, mmnuAutotext->addAction( QFileInfo(fn).baseName() ),
                                    SIGNAL( triggered() ), text);
        }
    }
    checkAutotextMenu( cedtr->currentDocument() );
}
