class QWidget;

#include "src/mainwindow.h"
#include "src/consolewidget.h"
#include "src/symbolswidget.h"
#include "src/generalsettingstab.h"

#include <bcore.h>
#include <babstractsettingstab.h>
#include <btexteditor.h>

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
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QDomNode>
#include <QLocale>
#include <QLayout>
#include <QToolButton>
#include <QLayout>
#include <QMessageBox>
#include <QComboBox>
#include <QTextStream>

#include <QDebug>

const QString Sep = QDir::separator();
const QString GroupCore = "core";
  const QString KeyMultipleInstancesEnabled = "multiple_instances_enabled";
const QString GroupMainWingow = "main_window";

//

QString localeBasedFileName( const QString &fileName, const QString &defaultFileName,
                             const QString &possibleSuffix = QString() )
{
    if ( fileName.isEmpty() )
        return "";
    QFileInfo fi(fileName);
    QString bfn = fi.path() + "/" + fi.baseName();
    QString suff = fi.suffix();
    suff = suff.isEmpty() ? possibleSuffix : "";
    if ( !suff.isEmpty() )
        suff.prepend('.');
    QString lang = BCore::locale().name().left(2);
    QFile f(bfn + "_" + lang);
    if ( !f.exists() )
        f.setFileName(bfn + "_" + lang + suff);
    if ( !f.exists() )
        f.setFileName(defaultFileName);
    if ( !f.exists() )
        f.setFileName(defaultFileName + suff);
    if ( !f.exists() )
        return "";
    return f.fileName();
}

QString localeBasedDirName(const QString &dir)
{
    if ( dir.isEmpty() )
        return "";
    QDir d( dir + "/" + BCore::locale().name().left(2) );
    if ( !d.exists() )
        d.setPath(dir + "/" + "en");
    if ( !d.exists() )
        d.setPath(dir);
    if ( !d.exists() )
        return "";
    return d.path();
}

//

bool MainWindow::multipleInstancesEnabled()
{
    QScopedPointer<QSettings> s( BCore::newSettingsInstance() );
    if (!s)
        return false;
    s->beginGroup(GroupCore);
      return s->value(KeyMultipleInstancesEnabled).toBool();
    s->endGroup();
}

//

MainWindow::MainWindow() :
    BMainWindow()
{
    setAcceptDrops(true);
    setProperty( "help", QString("home.html") );
    setHelpIndex("home.html");
    //
    setAboutIcon(":/logo.png");
    initTextEditor();
    initDockWidgets();
    initMenuBar();
    loadSettings();
    retranslateUi();
    updateWindowTitle( QString() );
    mTextEditor->setFocus();
}

//

BTextEditor *MainWindow::textEditor() const
{
    return mTextEditor;
}

//

void MainWindow::retranslateUi()
{
    //general
    setHelpDir( localeBasedDirName(":/res/help") );
    //TextEditor
    BTextEditor::FileTypeInfoList ftil;
    BTextEditor::FileTypeInfo fti;
    fti.description = tr("TeX files", "fileType");
    fti.fileName = tr("New document", "fileName");
    fti.suffixes << "tex";
    fti.suffixes << "inp";
    fti.suffixes << "pic";
    fti.suffixes << "sty";
    ftil << fti;
    fti.description = tr("All files", "fileType");
    fti.fileName = tr("New document", "fileName");
    fti.suffixes << "*";
    ftil << fti;
    mTextEditor->setAcceptableFileTypes(ftil);
    //MenuView
    mMenuView->setTitle( tr("View", "menu title") );
    mMenuView->clear();
    QList<QAction *> al = createPopupMenu()->actions();
    al.at(0)->setShortcut( QKeySequence("Ctrl+Shift+Y") );
    al.at(1)->setShortcut( QKeySequence("Ctrl+Shift+C") );
    mMenuView->addActions(al);
    //MenuTools
    mMenuTools->setTitle( tr("Tools", "menu title") );
    //DockWidgetConsole
    mDwgtSymbols->setWindowTitle( tr("Symbols", "dockWidget windowTitle") );
    mDockWidgetConsole->setWindowTitle( tr("Console", "dockWidget windowTitle") );
    //AboutDialog
    QString at = "<center><font size=4>TeX Creator.</font></center><br>";
    at += tr("Minimalistic, cross-platform TeX editor.", "aboutWidget aboutText") + "<br><br>";
    QString od = QApplication::organizationDomain();
    at += "Copyright &copy; 2012 " + QApplication::organizationName() + ".<br>";
    at += "<center><a href=\"" + od + "\">" + od + "</center>";
    setAboutText(at, true);
    setAboutChangeLog(localeBasedFileName(":/res/changelog/ChangeLog", ":/ChangeLog", "txt"), "UTF-8");
    PersonInfoList pil;
    PersonInfo pi;
    pi.name = tr("Yuri Melnikov", "aboutWidget infoName");
    pi.mail = "melnikov@k66.ru";
    pi.site = "";
    pi.role = tr("Original author", "aboutWidget infoRole");
    pil << pi;
    pi.name = tr("Andrey Bogdanov", "aboutWidget infoName");
    pi.mail = "the-dark-angel@yandex.ru";
    pi.site = "";
    pi.role = tr("Main developer", "aboutWidget infoRole");
    pil << pi;
    setAboutAuthors(pil);
    pil.clear();
    if (BCore::locale().language() == QLocale::Russian)
    {
        pi.name = tr("Andrey Bogdanov", "aboutWidget infoName");
        pi.mail = "the-dark-angel@yandex.ru";
        pi.site = "";
        pi.role = tr("Translator", "aboutWidget infoRole");
        pil << pi;
        setAboutTranslators(pil);
        pil.clear();
    }
    else
    {
        setAboutTranslators( PersonInfoList() );
    }
    pi.name = "Pascal Brachet";
    pi.mail = "";
    pi.site = "http://www.xm1math.net/";
    pi.role = tr("LaTeX symbols are taken from his Texmaker project", "aboutWidget infoRole");
    pil << pi;
    setAboutThanksTo(pil);
    pil.clear();
    setAboutLicense(localeBasedFileName(":/res/copying/COPYING", ":/COPYING", "txt"),
                    "UTF-8", ":/res/ico/OSI-Approved-License.png");
}

bool MainWindow::handleClosing()
{
    bool b = mTextEditor->askOnApplicationQuit();
    if (b)
        saveSettings();
    return b;
}

QMap<QString, BAbstractSettingsTab *> MainWindow::getSettingsTabMap() const
{
    QMap<QString, BAbstractSettingsTab *> m;
    m.insert( "text_editor", mTextEditor->settingsTab() );
    return m;
}

void MainWindow::handleSettings(const QMap<QString, QVariantMap> &settings)
{
    if ( settings.contains(BCore::GeneralSettingsTabId) )
    {
        QVariantMap m = settings.value(BCore::GeneralSettingsTabId);
        if ( m.contains(GeneralSettingsTab::IdMultipleInstancesEnabled) )
            setMultipleInstancesEnabled( m.value(GeneralSettingsTab::IdMultipleInstancesEnabled).toBool() );
    }
    if ( settings.contains("text_editor") )
        mTextEditor->applySettingsGlobal( settings.value("text_editor") );
}

BAbstractSettingsTab *MainWindow::generalSettingsTab() const
{
    GeneralSettingsTab *gst = new GeneralSettingsTab;
    gst->setMultipleInstancesEnabled( multipleInstancesEnabled() );
    return gst;
}

//

void MainWindow::saveSettings()
{
    mConsoleWidget->saveSettings();
}

void MainWindow::loadSettings()
{
    //
}

void MainWindow::setMultipleInstancesEnabled(bool enabled) const
{
    QScopedPointer<QSettings> s( BCore::newSettingsInstance() );
    if (!s)
        return;
    s->remove(GroupCore);
    s->beginGroup(GroupCore);
      s->setValue(KeyMultipleInstancesEnabled, enabled);
    s->endGroup();
}

void MainWindow::initTextEditor()
{
    mTextEditor = new BTextEditor(this);
    mTextEditor->setObjectName("BTextEditor");
    mTextEditor->setMacrosDefaultDir( BCore::user("macros") );
    mTextEditor->setProperty( "help", QString("editor.html") );
    if (QLocale::system().language() == QLocale::Russian)
        mTextEditor->loadKeyboardLayoutsMap(":/res/en-ru.klm", "UTF-8");
    mTextEditor->loadSyntaxes(":/res/syntax");
    mTextEditor->loadTextMacros( BCore::shared("macros") );
    mTextEditor->loadTextMacros( BCore::user("macros") );
    connect( mTextEditor, SIGNAL( currentDocumentChanged(QString) ), this, SLOT( updateWindowTitle(QString) ) );
    connect( mTextEditor, SIGNAL( showMessage(QString, int) ), statusBar(), SLOT( showMessage(QString, int) ) );
    setCentralWidget(mTextEditor);
    statusBar()->addPermanentWidget( mTextEditor->indicatorWidget() );
    installEventFilter(mTextEditor);
    QList<QToolBar *> tbars = mTextEditor->toolBars();
    for (int i = 0; i < tbars.size(); ++i)
        addToolBar( Qt::TopToolBarArea, tbars.at(i) );
}

void MainWindow::initDockWidgets()
{
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    initSymbolsWidget();
    initConsoleWidget();
}

void MainWindow::initSymbolsWidget()
{
    mSymbolsWgt = new SymbolsWidget;
      connect( mSymbolsWgt, SIGNAL( insertText(QString) ), mTextEditor, SLOT( insertText(QString) ) );
    mDwgtSymbols = new QDockWidget;
      mDwgtSymbols->setObjectName("DockWidgetSymbols");
      mDwgtSymbols->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
      mDwgtSymbols->setWidget(mSymbolsWgt);
    addDockWidget(Qt::TopDockWidgetArea, mDwgtSymbols);
}

void MainWindow::initConsoleWidget()
{
    mConsoleWidget = new ConsoleWidget(GroupMainWingow);
      mConsoleWidget->setTextEditor(mTextEditor);
    mDockWidgetConsole = new QDockWidget;
      mDockWidgetConsole->setObjectName("DockWidgetConsole");
      mDockWidgetConsole->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
      mDockWidgetConsole->setWidget(mConsoleWidget);
    addDockWidget(Qt::BottomDockWidgetArea, mDockWidgetConsole);
}

void MainWindow::initMenuBar()
{
    //MenuFile
    addToMenu( MenuFile, mTextEditor->editorActions(BTextEditor::NewOpenToolBar) );
    addSeparatorToMenu(MenuFile);
    addToMenu( MenuFile, mTextEditor->editorActions(BTextEditor::SaveToolBar) );
    addSeparatorToMenu(MenuFile);
    addToMenu( MenuFile, mTextEditor->editorActions(BTextEditor::CloseToolBar) );
    addSeparatorToMenu(MenuFile);
    //MenuEdit
    addToMenu( MenuEdit, mTextEditor->editorActions(BTextEditor::UndoRedoToolBar) );
    addSeparatorToMenu(MenuEdit);
    addToMenu( MenuEdit, mTextEditor->editorActions(BTextEditor::ClipboardToolBar) );
    addSeparatorToMenu(MenuEdit);
    addToMenu( MenuEdit, mTextEditor->editorActions(BTextEditor::SearchToolBar) );
    addSeparatorToMenu(MenuEdit);
    //MenuDocument
    insertMenu(mTextEditor->editorMenu(BTextEditor::DocumentMenu), MenuHelp);
    //MenuMacros
    insertMenu(mTextEditor->editorMenu(BTextEditor::MacrosMenu), MenuHelp);
    //MenuView
    mMenuView = new QMenu(this);
    insertMenu(mMenuView, MenuHelp);
    //MenuTools
    mMenuTools = new QMenu(this);
      mMenuTools->setObjectName("MenuTools");
      mMenuTools->addActions( mConsoleWidget->consoleActions() );
    insertMenu(mMenuTools, MenuHelp);
}

//

void MainWindow::updateWindowTitle(const QString &fileName)
{
    QString t = QApplication::applicationName();
    if ( !fileName.isEmpty() )
        t += ( ": " + QFileInfo(fileName).fileName() );
    setWindowTitle(t);
}
