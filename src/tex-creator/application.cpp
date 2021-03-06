/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
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

#include "application.h"

#include "consolewidget.h"
#include "mainwindow.h"
#include "settings.h"
#include "settingstab/codeeditorsettingstab.h"
#include "settingstab/consolesettingstab.h"
#include "settingstab/generalsettingstab.h"
#include "settingstab/networksettingstab.h"
#include "settingstab/texsamplesettingstab.h"
#include "texsample/texsamplecore.h"

#include <CodeEditorModulePluginInterface>

#include <TApplication>

#include <BAboutDialog>
#include <BAbstractDocumentDriver>
#include <BAbstractSettingsTab>
#include <BCodeEditor>
#include <BDirTools>
#include <BGuiPluginInterface>
#include <BGuiTools>
#include <BHelpBrowser>
#include <BLocationProvider>
#include <BPasswordWidget>
#include <BPluginInterface>
#include <BPluginsSettingsTab>
#include <BPluginWrapper>
#include <BSettingsDialog>
#include <BSignalDelayProxy>
#include <BSpellChecker>
#include <BSpellCheckerDictionary>
#include <BVersion>

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFileSystemWatcher>
#include <QList>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QNetworkProxyQuery>
#include <QObject>
#include <QRegExp>
#include <QSettings>
#include <QSize>
#include <QUrl>
#include <QVariant>

/*============================================================================
================================ Application =================================
============================================================================*/

/*============================== Public constructors =======================*/

Application::Application(int &argc, char **argv, const QString &applicationName, const QString &organizationName) :
    TApplication(argc, argv, applicationName, organizationName)
{
#if defined(BUILTIN_RESOURCES)
    Q_INIT_RESOURCE(tex_creator_1);
    Q_INIT_RESOURCE(tex_creator_2);
    Q_INIT_RESOURCE(tex_creator_3);
    Q_INIT_RESOURCE(tex_creator_4);
    Q_INIT_RESOURCE(tex_creator_5);
    Q_INIT_RESOURCE(tex_creator_6);
    Q_INIT_RESOURCE(tex_creator_dictionaries_1);
    Q_INIT_RESOURCE(tex_creator_dictionaries_2);
    Q_INIT_RESOURCE(tex_creator_dictionaries_3);
    Q_INIT_RESOURCE(tex_creator_dictionaries_4);
    Q_INIT_RESOURCE(tex_creator_doc_01);
    Q_INIT_RESOURCE(tex_creator_doc_02);
    Q_INIT_RESOURCE(tex_creator_doc_03);
    Q_INIT_RESOURCE(tex_creator_doc_04);
    Q_INIT_RESOURCE(tex_creator_doc_05);
    Q_INIT_RESOURCE(tex_creator_doc_06);
    Q_INIT_RESOURCE(tex_creator_doc_07);
    Q_INIT_RESOURCE(tex_creator_doc_08);
    Q_INIT_RESOURCE(tex_creator_doc_09);
    Q_INIT_RESOURCE(tex_creator_doc_10);
    Q_INIT_RESOURCE(tex_creator_symbols);
    Q_INIT_RESOURCE(tex_creator_translations);
#endif
    setApplicationVersion("4.0.3");
    setOrganizationDomain("http://sourceforge.net/projects/tex-creator");
    setApplicationCopyrightPeriod("2012-2015");
    BLocationProvider *prov = new BLocationProvider;
    prov->addLocation("autotext");
    prov->addLocation("dictionaries");
    prov->createLocationPath("autotext", UserResource);
    prov->createLocationPath("dictionaries", UserResource);
    installLocationProvider(prov);
    compatibility();
    resetProxy();
    QFont fnt = font();
    fnt.setPointSize(10);
    setFont(fnt);
    setThemedIconsEnabled(false);
    setPreferredIconFormats(QStringList() << "png");
    setWindowIcon(icon("tex"));
    installBeqtTranslator("qt");
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    installBeqtTranslator("qtbase");
#endif
    installBeqtTranslator("beqt");
    installBeqtTranslator("texsample");
    installBeqtTranslator("tex-creator");
    BAboutDialog::setDefaultMinimumSize(800, 400);
    setApplicationDescriptionFile(findResource("description", BDirTools::GlobalOnly) + "/DESCRIPTION.txt");
    setApplicationChangeLogFile(findResource("changelog", BDirTools::GlobalOnly) + "/ChangeLog.txt");
    setApplicationLicenseFile(findResource("copying", BDirTools::GlobalOnly) + "/COPYING.txt");
    setApplicationAuthorsFile(findResource("infos/authors.beqt-info", BDirTools::GlobalOnly));
    setApplicationTranslationsFile(findResource("infos/translators.beqt-info", BDirTools::GlobalOnly));
    setApplicationThanksToFile(findResource("infos/thanks-to.beqt-info", BDirTools::GlobalOnly));
    aboutDialogInstance()->setupWithApplicationData();
    mtexsampleCore = new TexsampleCore;
    mspellChecker = new BSpellChecker;
    reloadDictionaries();
    mspellChecker->setUserDictionary(location(DataPath, UserResource) + "/dictionaries/ignored.txt");
    mspellChecker->ignoreImplicitlyRegExp(QRegExp("\\\\|\\\\\\w+"));
    mspellChecker->considerLeftSurrounding(1);
    mspellChecker->considerRightSurrounding(0);
    mfsWatcher = new QFileSystemWatcher;
    foreach (const QString &s, QStringList() << "autotext" << "dictionaries") {
        foreach (const QString &path, locations(s)) {
            if (path.startsWith(":"))
                continue;
            mfsWatcher->addPath(path);
        }
    }
    BSignalDelayProxy *sdp = new BSignalDelayProxy(BeQt::Second, 2 * BeQt::Second, mfsWatcher);
    sdp->setStringConnection(mfsWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));
    connect(this, SIGNAL(pluginActivated(BPluginWrapper *)), this, SLOT(pluginActivatedSlot(BPluginWrapper *)));
    connect(this, SIGNAL(pluginAboutToBeDeactivated(BPluginWrapper *)),
            this, SLOT(pluginAboutToBeDeactivatedSlot(BPluginWrapper *)));
    createInitialWindow();
    loadPlugins(QStringList() << "editor-module");
    setHelpBrowserDefaultGeometry(BGuiTools::centerOnScreenGeometry(1000, 800, 100, 50));
    BHelpBrowser::setUserUrlHandlerFunction(&handleHelpBrowserUrl);
}

Application::~Application()
{
    delete mtexsampleCore;
    delete mspellChecker;
    delete mfsWatcher;
    if (BPasswordWidget::savePassword(Settings::Texsample::passwordWidgetState()))
        Settings::Texsample::savePassword();
#if defined(BUILTIN_RESOURCES)
    Q_CLEANUP_RESOURCE(tex_creator_1);
    Q_CLEANUP_RESOURCE(tex_creator_2);
    Q_CLEANUP_RESOURCE(tex_creator_3);
    Q_CLEANUP_RESOURCE(tex_creator_4);
    Q_CLEANUP_RESOURCE(tex_creator_5);
    Q_CLEANUP_RESOURCE(tex_creator_6);
    Q_CLEANUP_RESOURCE(tex_creator_dictionaries_1);
    Q_CLEANUP_RESOURCE(tex_creator_dictionaries_2);
    Q_CLEANUP_RESOURCE(tex_creator_dictionaries_3);
    Q_CLEANUP_RESOURCE(tex_creator_dictionaries_4);
    Q_CLEANUP_RESOURCE(tex_creator_doc_01);
    Q_CLEANUP_RESOURCE(tex_creator_doc_02);
    Q_CLEANUP_RESOURCE(tex_creator_doc_03);
    Q_CLEANUP_RESOURCE(tex_creator_doc_04);
    Q_CLEANUP_RESOURCE(tex_creator_doc_05);
    Q_CLEANUP_RESOURCE(tex_creator_doc_06);
    Q_CLEANUP_RESOURCE(tex_creator_doc_07);
    Q_CLEANUP_RESOURCE(tex_creator_doc_08);
    Q_CLEANUP_RESOURCE(tex_creator_doc_09);
    Q_CLEANUP_RESOURCE(tex_creator_doc_10);
    Q_CLEANUP_RESOURCE(tex_creator_symbols);
    Q_CLEANUP_RESOURCE(tex_creator_translations);
#endif
}

/*============================== Static public methods =====================*/

void Application::resetProxy()
{
    switch (Settings::Network::proxyMode()) {
    case Settings::Network::NoProxy:
        QNetworkProxy::setApplicationProxy(QNetworkProxy());
        break;
    case Settings::Network::SystemProxy: {
        QNetworkProxyQuery query = QNetworkProxyQuery(QUrl("http://www.google.com"));
        QList<QNetworkProxy> list = QNetworkProxyFactory::systemProxyForQuery(query);
        if (!list.isEmpty())
            QNetworkProxy::setApplicationProxy(list.first());
        else
            QNetworkProxy::setApplicationProxy(QNetworkProxy());
        break;
    }
    case Settings::Network::UserProxy: {
        QString host = Settings::Network::proxyHost();
        quint16 port = Settings::Network::proxyPort();
        QString login = Settings::Network::proxyLogin();
        QString password = Settings::Network::proxyPassword();
        QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, host, port, login, password));
        break;
    }
    default:
        break;
    }
}

void Application::windowAboutToClose(MainWindow *mw)
{
    if (!mw)
        return;
    foreach (BPluginWrapper *pw, pluginWrappers("editor-module")) {
        if (!pw || !pw->isActivated())
            continue;
        CodeEditorModulePluginInterface *interface = qobject_cast<CodeEditorModulePluginInterface *>(pw->instance());
        if (!interface)
            continue;
        interface->uninstallModule(mw->codeEditor(), mw);
    }
}

/*============================== Public methods ============================*/

QList<BCodeEditor *> Application::codeEditors() const
{
    QList<BCodeEditor *> list;
    foreach (MainWindow *mw, mmainWindows)
        list << mw->codeEditor();
    return list;
}

QList<ConsoleWidget *> Application::consoleWidgets() const
{
    QList<ConsoleWidget *> list;
    foreach (MainWindow *mw, mmainWindows)
        list << mw->consoleWidget();
    return list;
}

bool Application::mergeWindows()
{
    if (mmainWindows.size() < 2)
        return true;
    QList<MainWindow *> list = mmainWindows.values();
    MainWindow *first = list.takeFirst();
    foreach (MainWindow *mw, list) {
        first->codeEditor()->mergeWith(mw->codeEditor());
        mw->close();
    }
    first->activateWindow();
    return true;
}

MainWindow *Application::mostSuitableWindow() const
{
    QWidget *wgt = activeWindow();
    QList<MainWindow *> list = mmainWindows.values();
    foreach (MainWindow *mw, list) {
        if (mw == wgt)
            return mw;
    }
    return !list.isEmpty() ? list.first() : 0;
}

bool Application::showSettings(SettingsType type, QWidget *parent)
{
    BAbstractSettingsTab *tab = 0;
    switch (type) {
    case TexsampleSettings:
        tab = new TexsampleSettingsTab;
        break;
    case ConsoleSettings:
        tab = new ConsoleSettingsTab;
        break;
    default:
        break;
    }
    if (!tab)
        return false;
    return (BSettingsDialog(tab, parent ? parent : mostSuitableWindow()).exec() == BSettingsDialog::Accepted);
}

BSpellChecker *Application::spellChecker() const
{
    return mspellChecker;
}

TexsampleCore *Application::texsampleCore() const
{
    return mtexsampleCore;
}

void Application::updateCodeEditorSettings()
{
    foreach (BCodeEditor *ce, codeEditors()) {
        ce->setAutoCodecDetectionEnabled(Settings::CodeEditor::autoCodecDetectionEnabled());
        ce->setDefaultCodec(Settings::CodeEditor::defaultCodec());
        ce->setDocumentType(Settings::CodeEditor::documentType());
        ce->setEditFont(Settings::CodeEditor::editFont());
        ce->setEditLineLength(Settings::CodeEditor::editLineLength());
        ce->setEditTabWidth(Settings::CodeEditor::editTabWidth());
        ce->setMaximumFileSize(Settings::CodeEditor::maximumFileSize());
    }
}

void Application::updateConsoleSettings()
{
    foreach (ConsoleWidget *cw, consoleWidgets())
        cw->updateSwitchCompilerAction();
}

/*============================== Public slots ==============================*/

void Application::messageReceived(const QStringList &args)
{
    if (Settings::General::multipleWindowsEnabled()) {
        addMainWindow(args);
    } else {
        MainWindow *mw = !mmainWindows.isEmpty() ? mmainWindows.values().first() : 0;
        if (!mw)
            return;
        mw->codeEditor()->openDocuments(args);
    }
}

bool Application::showConsoleSettings(QWidget *parent)
{
    return showSettings(ConsoleSettings, parent);
}

void Application::showStatusBarMessage(const QString &message)
{
    MainWindow *mw = mostSuitableWindow();
    if (!mw)
        return;
    mw->showStatusBarMessage(message);
}

/*============================== Protected methods =========================*/

QList<BAbstractSettingsTab *> Application::createSettingsTabs() const
{
    QList<BAbstractSettingsTab *> list;
    list << new GeneralSettingsTab;
    list << new CodeEditorSettingsTab;
    list << new ConsoleSettingsTab;
    list << new NetworkSettingsTab;
    list << new TexsampleSettingsTab;
    list << new BPluginsSettingsTab;
    return list;
}

/*============================== Static private methods ====================*/

bool Application::handleHelpBrowserUrl(const QUrl &url)
{
    static const QString Path = "qrc:/pretexeditormodule/doc/ru/index.html";
    if (!url.toString().compare(Path, Qt::CaseInsensitive)) {
        BPluginWrapper *pw = pluginWrapper("editor-module/pretex");
        BGuiPluginInterface *gpi = pw ? qobject_cast<BGuiPluginInterface *>(pw->instance()) : 0;
        if (!gpi)
            return true;
        QString index = gpi->helpIndex();
        BHelpBrowser *hb = new BHelpBrowser(gpi->helpSearchPaths(), index, index);
        hb->setAttribute(Qt::WA_DeleteOnClose, true);
        hb->setGeometry(BApplication::helpBrowserDefaultGeometry());
        hb->show();
        return true;
    }
    return false;
}

bool Application::testAppInit()
{
    return bTest(bApp, "Application", "There must be an Application instance");
}

/*============================== Private methods ===========================*/

void Application::addMainWindow(const QStringList &fileNames)
{
    MainWindow *mw = new MainWindow;
    mw->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(mw, SIGNAL(destroyed(QObject *)), this, SLOT(mainWindowDestroyed(QObject *)));
    BCodeEditor *edr = mw->codeEditor();
    connect(edr, SIGNAL(fileHistoryChanged(QStringList)), this, SLOT(fileHistoryChanged(QStringList)));
    if (!fileNames.isEmpty())
        edr->openDocuments(fileNames);
    mmainWindows.insert(mw, mw);
    foreach (BPluginWrapper *pw, pluginWrappers("editor-module")) {
        if (!pw || !pw->isActivated())
            continue;
        CodeEditorModulePluginInterface *interface = qobject_cast<CodeEditorModulePluginInterface *>(pw->instance());
        if (!interface)
            continue;
        interface->installModule(mw->codeEditor(), mw);
    }
    mw->show();
}

void Application::compatibility()
{
    bSettings->setValue("Global/version", BVersion(applicationVersion()));
}

void Application::createInitialWindow()
{
    if (!testAppInit())
        return;
    addMainWindow(arguments().mid(1));
}

void Application::reloadDictionaries()
{
    foreach (BSpellCheckerDictionary *scd, mspellChecker->dictionaries())
        mspellChecker->removeDictionary(scd->locale());
    foreach (const QString &path, locations("dictionaries")) {
        foreach (const QString &p, BDirTools::entryList(path, QStringList() << "??_??", QDir::Dirs))
            mspellChecker->addDictionary(p);
    }
}

/*============================== Private slots =============================*/

void Application::directoryChanged(const QString &path)
{
    if (locations("autotext").contains(path))
        emit reloadAutotexts();
    else if (locations("dictionaries").contains(path))
        reloadDictionaries();
    mfsWatcher->addPath(path);
}

void Application::fileHistoryChanged(const QStringList &history)
{
    Settings::CodeEditor::setFileHistory(history);
    foreach (BCodeEditor *ce, codeEditors())
        ce->setFileHistory(history);
}

void Application::mainWindowDestroyed(QObject *obj)
{
    mmainWindows.remove(obj);
}

void Application::pluginAboutToBeDeactivatedSlot(BPluginWrapper *pw)
{
    if (!pw)
        return;
    if (pw->type() == "editor-module") {
        CodeEditorModulePluginInterface *interface = qobject_cast<CodeEditorModulePluginInterface *>(pw->instance());
        if (!interface)
            return;
        foreach (MainWindow *mw, mmainWindows)
            interface->uninstallModule(mw->codeEditor(), mw);
    }
}

void Application::pluginActivatedSlot(BPluginWrapper *pw)
{
    if (!pw)
        return;
    if (pw->type() == "editor-module") {
        CodeEditorModulePluginInterface *interface = qobject_cast<CodeEditorModulePluginInterface *>(pw->instance());
        if (!interface)
            return;
        foreach (MainWindow *mw, mmainWindows) {
            interface->installModule(mw->codeEditor(), mw);
            mw->restoreState(Settings::MainWindow::windowState()); //NOTE: Hack
        }
    }
}
