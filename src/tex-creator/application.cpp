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

#include "application.h"

#include "client.h"
#include "consolewidget.h"
#include "global.h"
#include "mainwindow.h"
#include "settings.h"
#include "settingstab/codeeditorsettingstab.h"
#include "settingstab/consolesettingstab.h"
#include "settingstab/generalsettingstab.h"
#include "settingstab/networksettingstab.h"
#include "settingstab/texsamplesettingstab.h"

#include <CodeEditorModulePluginInterface>

#include <TApplication>
#include <TClientInfo>
#include <TGetLatestAppVersionReplyData>
#include <TGetLatestAppVersionRequestData>
#include <TOperation>
#include <TReply>
#include <TUserInfo>
#include <TUserInfoWidget>

#include <BAboutDialog>
#include <BAbstractDocumentDriver>
#include <BAbstractSettingsTab>
#include <BCodeEditor>
#include <BDialog>
#include <BDirTools>
#include <BGuiTools>
#include <BLocationProvider>
#include <BLoginWidget>
#include <BNetworkConnection>
#include <BNetworkOperation>
#include <BOperationProgressDialog>
#include <BPasswordWidget>
#include <BPluginsSettingsTab>
#include <BPluginWrapper>
#include <BSettingsDialog>
#include <BSignalDelayProxy>
#include <BSpellChecker>
#include <BSpellCheckerDictionary>
#include <BTranslation>
#include <BVersion>

#include <QByteArray>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QFutureWatcher>
#include <QList>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QNetworkProxyQuery>
#include <QObject>
#include <QProgressDialog>
#include <QPushButton>
#include <QRegExp>
#include <QSettings>
#include <QSize>
#include <QtConcurrentRun>
#include <QTimer>
#include <QUrl>
#include <QVariant>

/*============================================================================
================================ CheckForNewVersionResult ====================
============================================================================*/

struct CheckForNewVersionResult
{
public:
    bool persistent;
    bool success;
    QUrl url;
    BVersion version;
public:
    explicit CheckForNewVersionResult(bool persistent = false);
};

/*============================================================================
================================ Global typedefs =============================
============================================================================*/

typedef QFuture<CheckForNewVersionResult> Future;
typedef QFutureWatcher<CheckForNewVersionResult> Watcher;

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static CheckForNewVersionResult checkForNewVersionFunction(bool persistent)
{
    CheckForNewVersionResult result(persistent);
    TGetLatestAppVersionRequestData request;
    request.setClientInfo(TClientInfo::create());
    Client *client = bApp->client();
    if (!client->isValid(true))
        return result;
    TReply reply = client->performAnonymousOperation(TOperation::GetLatestAppVersion, request);
    result.success = reply.success();
    if (result.success) {
        TGetLatestAppVersionReplyData data = reply.data().value<TGetLatestAppVersionReplyData>();
        result.url = data.downloadUrl();
        result.version = data.version();
    }
    return result;
}

/*============================================================================
================================ CheckForNewVersionResult ====================
============================================================================*/

/*============================== Public constructors =======================*/

CheckForNewVersionResult::CheckForNewVersionResult(bool persistent)
{
    this->persistent = persistent;
    success = false;
}

/*============================================================================
================================ Application =================================
============================================================================*/

/*============================== Public constructors =======================*/

Application::Application(int &argc, char **argv, const QString &applicationName, const QString &organizationName) :
    TApplication(argc, argv, applicationName, organizationName)
{
#if defined(BUILTIN_RESOURCES)
    Q_INIT_RESOURCE(tex_creator);
    Q_INIT_RESOURCE(tex_creator_doc);
    Q_INIT_RESOURCE(tex_creator_symbols);
    Q_INIT_RESOURCE(tex_creator_translations);
#endif
    setApplicationVersion("3.5.0-beta");
    setOrganizationDomain("https://github.com/TeXSample-Team/TeX-Creator");
    setApplicationCopyrightPeriod("2012-2014");
    BLocationProvider *prov = new BLocationProvider;
    prov->addLocation("autotext");
    prov->addLocation("dictionaries");
    prov->addLocation("texsample");
    prov->createLocationPath("autotext", UserResource);
    prov->createLocationPath("dictionaries", UserResource);
    installLocationProvider(prov);
    resetProxy();
    compatibility();
    QFont fnt = font();
    fnt.setPointSize(10);
    setFont(fnt);
    setThemedIconsEnabled(false);
    setPreferredIconFormats(QStringList() << "png");
    setWindowIcon(icon("tex"));
    installBeqtTranslator("qt");
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
    mclient = new Client(this);
    mclient->setShowMessageFunction(&showMessageFunction);
    mclient->setWaitForConnectedFunction(&waitForConnectedFunction);
    mclient->setWaitForFinishedFunction(&waitForFinishedFunction);
    mclient->setWaitForConnectedDelay(BeQt::Second / 2);
    mclient->setWaitForFinishedDelay(BeQt::Second / 2);
    mclient->setWaitForConnectedTimeout(10 * BeQt::Second);
    Settings::Texsample::loadPassword();
    updateClientSettings();
    mspellChecker = new BSpellChecker(this);
    reloadDictionaries();
    mspellChecker->setUserDictionary(location(DataPath, UserResource) + "/dictionaries/ignored.txt");
    mspellChecker->ignoreImplicitlyRegExp(QRegExp("\\\\|\\\\\\w+"));
    mspellChecker->considerLeftSurrounding(1);
    mspellChecker->considerRightSurrounding(0);
    mfsWatcher = new QFileSystemWatcher(this);
    foreach (const QString &s, QStringList() << "autotext" << "dictionaries") {
        foreach (const QString &path, locations(s)) {
            if (path.startsWith(":"))
                continue;
            mfsWatcher->addPath(path);
        }
    }
    BSignalDelayProxy *sdp = new BSignalDelayProxy(BeQt::Second, 2 * BeQt::Second, this);
    sdp->setStringConnection(mfsWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));
    connect(this, SIGNAL(pluginActivated(BPluginWrapper *)), this, SLOT(pluginActivatedSlot(BPluginWrapper *)));
    connect(this, SIGNAL(pluginAboutToBeDeactivated(BPluginWrapper *)),
            this, SLOT(pluginAboutToBeDeactivatedSlot(BPluginWrapper *)));
    createInitialWindow();
    loadPlugins(QStringList() << "editor-module");
    texsample();
}

Application::~Application()
{
    while (!mfutureWatchers.isEmpty()) {
        Watcher *w = dynamic_cast<Watcher *>(mfutureWatchers.takeLast());
        if (!w)
            continue;
        w->waitForFinished();
    }
    delete mspellChecker;
    if (BPasswordWidget::savePassword(Settings::Texsample::passwordWidgetState()))
        Settings::Texsample::savePassword();
#if defined(BUILTIN_RESOURCES)
    Q_CLEANUP_RESOURCE(tex_creator);
    Q_CLEANUP_RESOURCE(tex_creator_doc);
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

Client *Application::client() const
{
    return mclient;
}

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

void Application::handleExternalRequest(const QStringList &args)
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

bool Application::showRegisterDialog(QWidget *parent)
{
    if (!mclient->isValid(true) && !showSettings(TexsampleSettings, parent))
        return false;
    if (!mclient->isValid(true))
        return false;
    BDialog dlg(parent ? parent : mostSuitableWindow());
    dlg.setWindowTitle(tr("Registration", "dlg windowTitle"));
    TUserInfoWidget *wgt = new TUserInfoWidget(TUserInfoWidget::RegisterMode);
    wgt->setClient(mclient);
    wgt->restorePasswordWidgetState(Settings::Texsample::passwordWidgetState());
    dlg.setWidget(wgt);
    QPushButton *btnOk = dlg.addButton(QDialogButtonBox::Ok, &dlg, SLOT(accept()));
    btnOk->setEnabled(wgt->hasValidInput());
    connect(wgt, SIGNAL(inputValidityChanged(bool)), btnOk, SLOT(setEnabled(bool)));
    dlg.addButton(QDialogButtonBox::Cancel, &dlg, SLOT(reject()));
    dlg.resize(800, 0);
    while (dlg.exec() == BDialog::Accepted) {
        TReply r = mclient->performAnonymousOperation(TOperation::Register, wgt->createRequestData(), true, parent);
        if (r.success()) {
            Settings::Texsample::setLogin(wgt->login());
            Settings::Texsample::setPassword(wgt->password());
            updateClientSettings();
            if (mclient->isConnected())
                mclient->reconnect();
            else
                mclient->connectToServer();
            showStatusBarMessage(tr("You have successfully registered", "message"));
            break;
        } else {
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Registration error", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to register due to the following error:", "msgbox text"));
            msg.setInformativeText(r.message());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
        }
    }
    Settings::Texsample::setPasswordWidgetState(wgt->savePasswordWidgetState());
    return true;
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

void Application::updateClientSettings()
{
    mclient->setHostName(Settings::Texsample::host(true));
    mclient->setLogin(Settings::Texsample::login());
    mclient->setPassword(Settings::Texsample::password().encryptedPassword());
    mclient->reconnect();
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

bool Application::checkForNewVersion(bool persistent)
{
    if (!mclient->isValid(true)) {
        if (!showSettings(TexsampleSettings))
            return false;
        updateClientSettings();
    }
    if (!mclient->isValid(true))
        return false;
    Future f = QtConcurrent::run(&checkForNewVersionFunction, persistent);
    Watcher *w = new Watcher;
    w->setFuture(f);
    connect(w, SIGNAL(finished()), this, SLOT(checkingForNewVersionFinished()));
    mfutureWatchers << w;
    return true;
}

bool Application::checkForNewVersionPersistent()
{
    return checkForNewVersion(true);
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

void Application::showMessageFunction(const QString &text, const QString &informativeText, bool error,
                                      QWidget *parentWidget)
{
    QMessageBox msg(parentWidget ? parentWidget : bApp->mostSuitableWindow());
    if (error) {
        msg.setWindowTitle(tr("TeXSample connection error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
    } else {
        msg.setWindowTitle(tr("TeXSample connection message", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Information);
    }
    msg.setText(text);
    msg.setInformativeText(informativeText);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    msg.exec();
}

bool Application::testAppInit()
{
    return bTest(bApp, "Application", "There must be an Application instance");
}

bool Application::waitForConnectedFunction(BNetworkConnection *connection, int timeout, bool gui,
                                           QWidget *parentWidget, QString *msg)
{
    if (!connection)
        return false;
    if (gui) {
        QProgressDialog pd(parentWidget ? parentWidget : bApp->mostSuitableWindow());
        pd.setWindowTitle(tr("Connecting to server", "pdlg windowTitle"));
        pd.setLabelText(tr("Connecting to server, please, wait...", "pdlg labelText"));
        pd.setMinimum(0);
        pd.setMaximum(0);
        QTimer::singleShot(timeout, &pd, SLOT(close()));
        if (pd.exec() == QProgressDialog::Rejected)
            return false;
        return connection->isConnected();
    } else {
        BeQt::waitNonBlocking(connection, SIGNAL(connected()), timeout);
        if (connection->isConnected())
            return bRet(msg, QString(), true);
        return bRet(msg, connection->errorString(), false);
    }
}

bool Application::waitForFinishedFunction(BNetworkOperation *op, int timeout, bool gui, QWidget *parentWidget,
                                          QString *msg)
{
    if (!op)
        return false;
    if (gui) {
        BOperationProgressDialog dlg(op, parentWidget ? parentWidget : bApp->mostSuitableWindow());
        dlg.setWindowTitle(tr("Executing request...", "opdlg windowTitle"));
        dlg.setAutoCloseInterval((timeout > 0) ? timeout : 0);
        dlg.exec();
        return op->isFinished();
    } else {
        if (op->waitForFinished(timeout))
            return bRet(msg, QString(), true);
        return bRet(msg, tr("Operation timed out", "error"), false);
    }
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
    if (bSettings->value("Global/version").value<BVersion>() < BVersion("3.0.0-pa")) {
        bSettings->remove("General/CodeEditor");
        bSettings->remove("BeQt/Core/deactivated_plugins");
        bSettings->remove("CodeEditor/document_driver_state");
        bSettings->remove("CodeEditor/search_moudle_state");
        bSettings->remove("CodeEditor/edit_font_family");
        bSettings->remove("CodeEditor/font_family");
        bSettings->remove("CodeEditor/font_point_size");
        bSettings->remove("SamplesWidget");
        bSettings->remove("TeXSample");
        bSettings->remove("editor");
        bSettings->remove("main_window");
        bSettings->remove("TexsampleWidget/table_header_state");
        bSettings->remove("TexsampleWidget/add_sample_dialog_size");
        bSettings->remove("TexsampleWidget/edit_sample_dialog_size");
        bSettings->remove("Console/compiler_commands");
        bSettings->remove("Console/compiler_name");
        bSettings->remove("Console/compiler_options");
        bSettings->remove("Console/dvips_enabled");
        bSettings->remove("Console/makeindex_enabled");
        //sCache->clear();
    }
    if (bSettings->value("Global/version").value<BVersion>() < BVersion("3.4.0-beta"))
        bSettings->remove("Macros/ExternalTools");
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

void Application::showStatusBarMessage(const QString &message)
{
    MainWindow *mw = mostSuitableWindow();
    if (!mw)
        return;
    mw->showStatusBarMessage(message);
}

void Application::texsample()
{
    bool b = true;
    if (!Settings::Texsample::hasTexsample()) {
        QMessageBox msg(mostSuitableWindow());
        msg.setWindowTitle(tr("TeXSample configuration", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Question);
        msg.setText(tr("It seems that you have not configured TeXSample service yet.\n"
                       "Would you like to do it now?", "msgbox text"));
        msg.setInformativeText(tr("To remove this notification, you have to configure or disable TeXSample service",
                                  "msgbox informativeText"));
        QPushButton *btnRegister = msg.addButton(tr("Register", "btn text"), QMessageBox::AcceptRole);
        QPushButton *btnConfig = msg.addButton(tr("I have an account", "btn text"), QMessageBox::AcceptRole);
        QPushButton *btnDisable = msg.addButton(tr("Disable TeXSample", "btn text"), QMessageBox::RejectRole);
        msg.addButton(tr("Not right now", "btn text"), QMessageBox::RejectRole);
        msg.setDefaultButton(btnConfig);
        msg.exec();
        if (msg.clickedButton() == btnRegister) {
            if (showRegisterDialog())
                Settings::Texsample::setConnectOnStartup(true);
        } else if (msg.clickedButton() == btnConfig) {
            if (showSettings(TexsampleSettings))
                mclient->connectToServer();
            else
                b = false;
        } else if (msg.clickedButton() == btnDisable) {
            Settings::Texsample::setConnectOnStartup(false);
            b = false;
        }
    } else if (Settings::Texsample::connectOnStartup()) {
        if (!mclient->isValid())
            b = showSettings(TexsampleSettings);
        mclient->connectToServer();
    }
    if (Settings::General::checkForNewVersionOnStartup() && (b || mclient->isValid(true)))
        checkForNewVersion();
}

/*============================== Private slots =============================*/

void Application::checkingForNewVersionFinished()
{
    Watcher *w = dynamic_cast<Watcher *>(sender());
    if (!w)
        return;
    mfutureWatchers.removeAll(w);
    CheckForNewVersionResult result = w->result();
    delete w;
    if (!result.success) {
        //TODO
        qDebug() << "fail";
        return;
    }
    QMessageBox msg(mostSuitableWindow());
    msg.setWindowTitle(tr("New version", "msgbox windowTitle"));
    msg.setIcon(QMessageBox::Information);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    if (result.version.isValid() && result.version > BVersion(applicationVersion())) {
        QString s = tr("A new version of the application is available", "msgbox text")
                + " (v" + result.version.toString(BVersion::Full) + ").";
        if (result.url.isValid()) {
            s += " " + tr("Click the following link to go to the download page:", "msgbox text")
                    + " <a href=\"" + result.url.toString() + "\">" + tr("download", "msgbox text") + "</a>";
        }
        msg.setText(s);
        msg.setInformativeText(tr("You should always use the latest application version. "
                                  "Bugs are fixed and new features are implemented in new versions.",
                                  "msgbox informativeText"));
        msg.exec();
    } else if (result.persistent) {
        msg.setText(tr("You are using the latest version.", "msgbox text"));
        msg.exec();
    }
}

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
            mw->restoreState(MainWindow::getWindowState()); //NOTE: Hack
        }
    }
}
