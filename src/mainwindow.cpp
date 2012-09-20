class QWidget;

#include "src/mainwindow.h"
#include "src/consolewidget.h"
#include "src/symbolswidget.h"

#include <bcore.h>
#include <babstractsettingstab.h>
#include <btexteditor.h>
#include <babstractfiletype.h>

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

class LaTeX : public BAbstractFileType
{
public:
    LaTeX(QObject *parent = 0) :
        BAbstractFileType(parent)
    {
        loadSyntax(":/res/syntax/latex.xml");
    }
    //
    QString description() const
    {
        return tr("LaTeX files", "fileType description");
    }
    QStringList suffixes() const
    {
        return QStringList() << "tex" << "inp" << "pic" << "sty";
    }
    BAbstractFileType *clone() const
    {
        return new LaTeX( parent() );
    }
};

//

const QString Sep = QDir::separator();
const QString GroupMainWingow = "main_window";

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
    connect( BCore::instance(), SIGNAL( localeChanged() ), this, SLOT( retranslateUi() ) );
}

//

BTextEditor *MainWindow::textEditor() const
{
    return mTextEditor;
}

//

bool MainWindow::handleClosing()
{
    bool b = mTextEditor->askOnClose();
    if (b)
    {
        mTextEditor->saveSettings();
        saveSettings();
    }
    return b;
}

QMap<QString, BAbstractSettingsTab *> MainWindow::userSettingsTabMap() const
{
    QMap<QString, BAbstractSettingsTab *> m;
    BTextEditor::SettingsOptions opt;
    opt.macrosDir = false;
    m.insert( BTextEditor::SettingsTabId, mTextEditor->createSettingsTab(opt) );
    return m;
}

void MainWindow::handleUserSettings(const QMap<QString, QVariantMap> &settings)
{
    mTextEditor->applySettings( settings.value(BTextEditor::SettingsTabId) );
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

void MainWindow::initTextEditor()
{
    mTextEditor = new BTextEditor(this, true);
    mTextEditor->setObjectName("BTextEditor");
    mTextEditor->setProperty( "help", QString("editor.html") );
    mTextEditor->setUserFileTypes(QList<BAbstractFileType *>() << new LaTeX);
    mTextEditor->setDefaultMacrosDir( BCore::user("macros") );
    mTextEditor->loadSettings();
    mTextEditor->loadKeyboardLayoutMaps(":/res/klm");
    mTextEditor->loadTextMacros( QStringList() << BCore::shared("macros") << BCore::user("macros") );
    connect( mTextEditor, SIGNAL( currentDocumentChanged(QString) ), this, SLOT( updateWindowTitle(QString) ) );
    connect( mTextEditor, SIGNAL( showMessage(QString, int) ), statusBar(), SLOT( showMessage(QString, int) ) );
    setCentralWidget(mTextEditor);
    statusBar()->addPermanentWidget( mTextEditor->indicatorWidget() );
    installEventFilter(mTextEditor);
    QList<QToolBar *> tbars = mTextEditor->editorToolBars();
    for (int i = 0; i < tbars.size(); ++i)
    {
        tbars.at(i)->setAllowedAreas(Qt::TopToolBarArea);
        addToolBar( Qt::TopToolBarArea, tbars.at(i) );
    }
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
    addToMenu( MenuFile, mTextEditor->editorActions(BTextEditor::FileMenu) );
    addSeparatorToMenu(MenuFile);
    //MenuEdit
    addToMenu( MenuEdit, mTextEditor->editorActions(BTextEditor::EditMenu) );
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

void MainWindow::retranslateUi()
{
    //general
    setHelpDir( BCore::localeBasedDirName(":/res/help") );
    //TextEditor
    mTextEditor->setDefaultFileName(tr("New document", "textEditor fileName") + ".tex");
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
    setAboutChangeLog(BCore::localeBasedFileName(":/res/changelog/ChangeLog", ":/ChangeLog", "txt"), "UTF-8");
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
    if (BCore::currentLocale().language() == QLocale::Russian)
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
    setAboutLicense(BCore::localeBasedFileName(":/res/copying/COPYING", ":/COPYING", "txt"),
                    "UTF-8", ":/res/ico/OSI-Approved-License.png");
}

void MainWindow::updateWindowTitle(const QString &fileName)
{
    QString t = QApplication::applicationName();
    if ( !fileName.isEmpty() )
        t += ( ": " + QFileInfo(fileName).fileName() );
    setWindowTitle(t);
}
