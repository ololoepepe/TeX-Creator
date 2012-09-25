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
#include <QTimer>

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
    setDockOptions(dockOptions() | QMainWindow::ForceTabbedDocks);
    setHelpIndex("index.html");
    setContextualHelpEnabled(false);
    setAboutIcon(":/logo.png");
    mmapperLocations = new QSignalMapper(this);
      connect( mmapperLocations, SIGNAL( mapped(int) ), this, SLOT( openLocation(int) ) );
    initTextEditor();
    initDockWidgets();
    initMenuBar();
    loadSettings();
    retranslateUi();
    updateWindowTitle( QString() );
    mTextEditor->setFocus();
    connect( BCore::instance(), SIGNAL( localeChanged() ), this, SLOT( retranslateUi() ) );
    QTimer::singleShot( 1, this, SLOT( fillMnuView() ) );
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
    mTextEditor->setUserFileTypes(QList<BAbstractFileType *>() << new LaTeX);
    mTextEditor->setDefaultMacrosDir( BCore::user("macros") );
    mTextEditor->setMainDocumentActionVisible(true);
    mTextEditor->loadSettings();
    mTextEditor->loadKeyboardLayoutMaps( QStringList() << BCore::shared("layout-maps") << BCore::user("layout-maps") );
    mTextEditor->loadAutoText( QStringList() << BCore::shared("autotext") << BCore::user("autotext") );
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
      mDwgtSymbols->setAllowedAreas(Qt::LeftDockWidgetArea);
      mDwgtSymbols->setWidget(mSymbolsWgt);
    addDockWidget(Qt::LeftDockWidgetArea, mDwgtSymbols);
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
    mmnuView = new QMenu(this);
    insertMenu(mmnuView, MenuHelp);
    //MenuTools
    mMenuTools = new QMenu(this);
      mMenuTools->setObjectName("MenuTools");
      mMenuTools->addActions( mConsoleWidget->consoleActions() );
      mMenuTools->addSeparator();
      mmnuOpenDirShared = new QMenu(this);
        mactAutoTextShared = new QAction(this);
          mmapperLocations->setMapping(mactAutoTextShared, AutoTextSharedLocation);
          connect( mactAutoTextShared, SIGNAL( triggered() ), mmapperLocations, SLOT( map() ) );
        mmnuOpenDirShared->addAction(mactAutoTextShared);
        mactKLMShared = new QAction(this);
          mmapperLocations->setMapping(mactKLMShared, KLMSharedLocation);
          connect( mactKLMShared, SIGNAL( triggered() ), mmapperLocations, SLOT( map() ) );
        mmnuOpenDirShared->addAction(mactKLMShared);
        mactPluginsShared = new QAction(this);
          mmapperLocations->setMapping(mactPluginsShared, PluginsSharedLocation);
          connect( mactPluginsShared, SIGNAL( triggered() ), mmapperLocations, SLOT( map() ) );
        mmnuOpenDirShared->addAction(mactPluginsShared);
        mactTranslationsShared = new QAction(this);
          mmapperLocations->setMapping(mactTranslationsShared, TranslationsSharedLocation);
          connect( mactTranslationsShared, SIGNAL( triggered() ), mmapperLocations, SLOT( map() ) );
        mmnuOpenDirShared->addAction(mactTranslationsShared);
      mMenuTools->addMenu(mmnuOpenDirShared);
      //
      mmnuOpenDirUser = new QMenu(this);
        mactAutoTextUser = new QAction(this);
          mmapperLocations->setMapping(mactAutoTextUser, AutoTextUserLocation);
          connect( mactAutoTextUser, SIGNAL( triggered() ), mmapperLocations, SLOT( map() ) );
        mmnuOpenDirUser->addAction(mactAutoTextUser);
        mactKLMUser = new QAction(this);
          mmapperLocations->setMapping(mactKLMUser, KLMUserLocation);
          connect( mactKLMUser, SIGNAL( triggered() ), mmapperLocations, SLOT( map() ) );
        mmnuOpenDirUser->addAction(mactKLMUser);
        mactMacrosUser = new QAction(this);
          mmapperLocations->setMapping(mactMacrosUser, MacrosUserLocations);
          connect( mactMacrosUser, SIGNAL( triggered() ), mmapperLocations, SLOT( map() ) );
        mmnuOpenDirUser->addAction(mactMacrosUser);
        mactPluginsUser = new QAction(this);
          mmapperLocations->setMapping(mactPluginsUser, PluginsUserLocation);
          connect( mactPluginsUser, SIGNAL( triggered() ), mmapperLocations, SLOT( map() ) );
        mmnuOpenDirUser->addAction(mactPluginsUser);
        mactTranslationsUser = new QAction(this);
          mmapperLocations->setMapping(mactTranslationsUser, TranslationsUserLocation);
          connect( mactTranslationsUser, SIGNAL( triggered() ), mmapperLocations, SLOT( map() ) );
        mmnuOpenDirUser->addAction(mactTranslationsUser);
      mMenuTools->addMenu(mmnuOpenDirUser);
    insertMenu(mMenuTools, MenuHelp);
}

//

void MainWindow::retranslateUi()
{
    //general
    setHelpDir( BCore::localeBasedDirName(":/res/help") );
    //TextEditor
    mTextEditor->setDefaultFileName(tr("New document", "textEditor fileName") + ".tex");
    //ActionView
    mmnuView->setTitle( tr("View", "mnu title") );
    fillMnuView();
    //MenuTools
    mMenuTools->setTitle( tr("Tools", "menu title") );
    mmnuOpenDirShared->setTitle( tr("Open shared dir", "mnu title") );
      mactAutoTextShared->setText( tr("Autotext", "act text") );
      mactKLMShared->setText( tr("Keyboard layout maps", "act text") );
      mactPluginsShared->setText( tr("Plugins", "act text") );
      mactTranslationsShared->setText( tr("Translations", "act text") );
    mmnuOpenDirUser->setTitle( tr("Open user dir", "mnu title") );
      mactAutoTextUser->setText( mactAutoTextShared->text() );
      mactKLMUser->setText( mactKLMShared->text() );
      mactMacrosUser->setText( tr("Macros", "act text") );
      mactPluginsUser->setText( mactPluginsShared->text() );
      mactTranslationsUser->setText( mactTranslationsShared->text() );
    //DockWidgetConsole
    mDwgtSymbols->setWindowTitle( tr("Symbols", "dockWidget windowTitle") );
    mDockWidgetConsole->setWindowTitle( tr("Console", "dockWidget windowTitle") );
    //AboutDialog
    QString at = "<center><font size=4>TeX Creator</font></center><br>";
    at += "<center>" + tr("Minimalistic, cross-platform TeX editor", "aboutWidget aboutText") + "</center><br>";
    QString od = QApplication::organizationDomain();
    at += "<center>Copyright &copy; 2012 " + QApplication::organizationName() + "</center><br>";
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
        t += (": " + fileName);
#if defined(Q_OS_WIN)
    t.replace('/', '\\');
#endif
    setWindowTitle(t);
}

void MainWindow::fillMnuView()
{
    mmnuView->clear();
    QList<QAction *> list = createPopupMenu()->actions();
    list.at(0)->setShortcut( QKeySequence("Ctrl+Shift+Y") ); //TODO: it's very unsafe
    list.at(1)->setShortcut( QKeySequence("Ctrl+Shift+C") ); //TODO: it's very unsafe
    mmnuView->addActions(list);
}

void MainWindow::openLocation(int id)
{
    QString dir;
    switch (id)
    {
    //shared
    case AutoTextSharedLocation:
        dir = BCore::shared("autotext");
        break;
    case KLMSharedLocation:
        dir = BCore::shared("layout-maps");
        break;
    case PluginsSharedLocation:
        dir = BCore::pluginsDir(false);
        break;
    case TranslationsSharedLocation:
        dir = BCore::translationsDir(false);
        break;
    //user
    case AutoTextUserLocation:
        dir = BCore::user("autotext");
        break;
    case KLMUserLocation:
        dir = BCore::user("layout-maps");
        break;
    case MacrosUserLocations:
        dir = BCore::user("macros");
        break;
    case PluginsUserLocation:
        dir = BCore::pluginsDir(true);
        break;
    case TranslationsUserLocation:
        dir = BCore::translationsDir(true);
        break;
    default:
        break;
    }
    if ( !dir.isEmpty() )
        QDesktopServices::openUrl( QUrl::fromLocalFile(dir) );
}
