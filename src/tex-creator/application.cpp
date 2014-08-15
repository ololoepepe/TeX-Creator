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
#include "global.h"
#include "mainwindow.h"
#include "settingstab/codeeditorsettingstab.h"
#include "settingstab/consolesettingstab.h"
#include "settingstab/generalsettingstab.h"
#include "settingstab/networksettingstab.h"
#include "settingstab/texsamplesettingstab.h"

#include <CodeEditorModulePluginInterface>

#include <TApplication>
#include <TUserInfo>
#include <TUserWidget>

#include <BAboutDialog>
#include <BAbstractSettingsTab>
#include <BCodeEditor>
#include <BDialog>
#include <BDirTools>
#include <BGuiTools>
#include <BLocationProvider>
#include <BLoginWidget>
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
#include <QPushButton>
#include <QRegExp>
#include <QSettings>
#include <QSize>
#include <QtConcurrentRun>
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
    prov->addLocation("klm");
    prov->addLocation("texsample");
    prov->createLocationPath("autotext", UserResource);
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
    //if (Global::checkForNewVersions())
    //    Application::checkForNewVersions();
    mspellChecker = new BSpellChecker(this);
    reloadDictionaries();
    mspellChecker->setUserDictionary(location(DataPath, UserResource) + "/dictionaries/ignored.txt");
    mspellChecker->ignoreImplicitlyRegExp(QRegExp("\\\\|\\\\\\w+"));
    mspellChecker->considerLeftSurrounding(1);
    mspellChecker->considerRightSurrounding(0);
    Global::loadPasswordState();
    mfsWatcher = new QFileSystemWatcher(this);
    foreach (const QString &s, QStringList() << "autotext" << "klm" << "dictionaries") {
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
}

Application::~Application()
{
    /*typedef QFutureWatcher<Client::CheckForNewVersionsResult> Watcher;
    while (!futureWatchers.isEmpty())
    {
        Watcher *w = dynamic_cast<Watcher *>(futureWatchers.takeLast());
        if (!w)
            continue;
        w->waitForFinished();
    }*/
    delete mspellChecker;
    Global::savePasswordState();
#if defined(BUILTIN_RESOURCES)
    Q_CLEANUP_RESOURCE(tex_creator);
    Q_CLEANUP_RESOURCE(tex_creator_doc);
    Q_CLEANUP_RESOURCE(tex_creator_symbols);
    Q_CLEANUP_RESOURCE(tex_creator_translations);
#endif
}

/*============================== Static public methods =====================*/

QWidget *Application::mostSuitableWindow()
{
    if ( !testAppInit() )
        return 0;
    QWidget *wgt = activeWindow();
    QList<MainWindow *> list = bApp->mmainWindows.values();
    foreach (MainWindow *mw, list)
        if (mw == wgt)
            return wgt;
    return !list.isEmpty() ? list.first() : 0;
}

QList<BCodeEditor *> Application::codeEditors()
{
    QList<BCodeEditor *> list;
    foreach (MainWindow *mw, bApp->mmainWindows)
        list << mw->codeEditor();
    return list;
}

bool Application::mergeWindows()
{
    if (bApp->mmainWindows.size() < 2)
        return true;
    QList<MainWindow *> list = bApp->mmainWindows.values();
    MainWindow *first = list.takeFirst();
    foreach (MainWindow *mw, list)
    {
        first->codeEditor()->mergeWith(mw->codeEditor());
        mw->close();
    }
    first->activateWindow();
    return true;
}

void Application::handleExternalRequest(const QStringList &args)
{
    if ( !testAppInit() )
        return;
    if (Global::multipleWindowsEnabled())
    {
        bApp->addMainWindow(args);
    }
    else
    {
        MainWindow *mw = !bApp->mmainWindows.isEmpty() ? bApp->mmainWindows.values().first() : 0;
        if (!mw)
            return;
        mw->codeEditor()->openDocuments(args);
    }
}

bool Application::showLoginDialog(QWidget *parent)
{
    static const BTranslation AutoSelect = BTranslation::translate("Application", "Auto select");
    BDialog dlg(parent ? parent : mostSuitableWindow());
      dlg.setWindowTitle(tr("Logging in", "windowTitle"));
      BLoginWidget *lwgt = new BLoginWidget;
      dlg.setWidget(lwgt);
      QPushButton *btnOk = dlg.addButton(QDialogButtonBox::Ok, SLOT(accept()));
        btnOk->setDefault(true);
        btnOk->setEnabled(lwgt->hasValidInput());
        connect(lwgt, SIGNAL(inputValidityChanged(bool)), btnOk, SLOT(setEnabled(bool)));
      dlg.addButton(QDialogButtonBox::Cancel, SLOT(reject()));
        lwgt->setAddressType(BLoginWidget::EditableComboAddress, true);
        QStringList hosts;
        hosts << AutoSelect << Global::hostHistory();
        lwgt->setAvailableAddresses(hosts);
        lwgt->setPersistentAddress(AutoSelect);
        lwgt->setAddress((Global::host() == "auto_select") ? AutoSelect : Global::host());
        lwgt->restorePasswordWidgetState(Global::passwordWidgetState());
        lwgt->setLogin(Global::login());
        lwgt->setPassword(Global::password());
      dlg.setFixedSize(dlg.sizeHint());
    if (dlg.exec() != BDialog::Accepted)
    {
        Global::setPasswordWidgetSate(lwgt->savePasswordWidgetState());
        return false;
    }
    Global::setPasswordWidgetSate(lwgt->savePasswordWidgetState());
    hosts = lwgt->availableAddresses().mid(1);
    QString nhost = lwgt->address();
    if (AutoSelect.translate() != nhost)
        hosts.prepend(nhost);
    else
        nhost = "auto_select";
    hosts.removeDuplicates();
    Global::setHostHistory(hosts);
    Global::setHost(nhost);
    Global::setLogin(lwgt->login());
    Global::setPassword(lwgt->securePassword());
    //sClient->updateSettings();
    return true;
}

bool Application::showRegisterDialog(QWidget *parent)
{
    /*QDialog dlg(parent ? parent : mostSuitableWindow());
    dlg.setWindowTitle(tr("Registration", "dlg windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      TUserWidget *uwgt = new TUserWidget(&Client::checkEmail, &Client::checkLogin);
        uwgt->restorePasswordWidgetState(Global::passwordWidgetState());
      vlt->addWidget(uwgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Ok);
        dlgbbox->button(QDialogButtonBox::Ok)->setEnabled(false);
        connect(uwgt, SIGNAL(validityChanged(bool)), dlgbbox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
        connect(dlgbbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dlg, SLOT(accept()));
        dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(dlgbbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dlg, SLOT(reject()));
      vlt->addWidget(dlgbbox);
      dlg.setMinimumWidth(700);
      dlg.setFixedHeight(dlg.sizeHint().height());
    while (dlg.exec() == QDialog::Accepted)
    {
        TUserInfo info = uwgt->info();
        TOperationResult r = Client::registerUser(info, dlg.parentWidget());
        if (r)
        {
            Global::setLogin(info.login());
            Global::setPassword(uwgt->password());
            Global::setPasswordWidgetSate(uwgt->savePasswordWidgetState());
            sClient->updateSettings();
            sClient->connectToServer();
            return true;
        }
        else
        {
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Registration error", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to register due to the following error:", "msgbox text"));
            msg.setInformativeText(r.messageString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
        }
    }
    Global::setPasswordWidgetSate(uwgt->savePasswordWidgetState());*/
    return false;
}

bool Application::showSettings(Settings type, QWidget *parent)
{
    if (!parent)
        parent = mostSuitableWindow();
    switch (type)
    {
    case AccountSettings:
    {
        /*BDialog dlg(parent);
          dlg.setWindowTitle(tr("Updating account", "dlg windowTitle"));
          TUserWidget *uwgt = new TUserWidget(TUserWidget::UpdateMode);
            TUserInfo info(TUserInfo::UpdateContext);
            sClient->getUserInfo(sClient->userId(), info, parent);
            uwgt->setInfo(info);
            uwgt->restorePasswordWidgetState(Global::passwordWidgetState());
            uwgt->restoreState(bSettings->value("UpdateUserDialog/user_widget_state").toByteArray());
            uwgt->setPassword(Global::password());
          dlg.setWidget(uwgt);
          dlg.addButton(QDialogButtonBox::Ok, SLOT(accept()));
          dlg.button(QDialogButtonBox::Ok)->setEnabled(uwgt->isValid());
          connect(uwgt, SIGNAL(validityChanged(bool)), dlg.button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
          dlg.addButton(QDialogButtonBox::Cancel, SLOT(reject()));
          dlg.setMinimumSize(600, dlg.sizeHint().height());
          if (dlg.exec() != BDialog::Accepted)
              return false;
        info = uwgt->info();
        TOperationResult r = sClient->updateAccount(info, parent);
        if (r)
        {
            Global::setPassword(uwgt->password());
            Global::setPasswordWidgetSate(uwgt->savePasswordWidgetState());
            if (!sClient->updateSettings())
                sClient->reconnect();
            return true;
        }
        else
        {
            QMessageBox msg(parent);
            msg.setWindowTitle(tr("Changing account failed", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("The following error occured:", "msgbox text"));
            msg.setInformativeText(r.messageString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
            //Global::setPasswordWidgetSate(uwgt->savePasswordWidgetState());
            return false;
        }*/
    }
    case ConsoleSettings:
    {
        BSettingsDialog sd(new ConsoleSettingsTab, parent ? parent : mostSuitableWindow());
        return sd.exec() == BSettingsDialog::Accepted;
    }
    default:
        return false;
    }
}

void Application::emitUseRemoteCompilerChanged()
{
    if (!bApp)
        return;
    QMetaObject::invokeMethod(bApp, "useRemoteCompilerChanged");
}

void Application::updateDocumentType()
{
    if (!bApp)
        return;
    foreach (MainWindow *mw, bApp->mmainWindows)
        mw->codeEditor()->setDocumentType(Global::editorDocumentType());
}

void Application::updateMaxDocumentSize()
{
    if (!bApp)
        return;
    foreach (MainWindow *mw, bApp->mmainWindows)
        mw->codeEditor()->setMaximumFileSize(Global::maxDocumentSize());
}

void Application::checkForNewVersions(bool persistent)
{
    /*typedef QFuture<Client::CheckForNewVersionsResult> Future;
    typedef QFutureWatcher<Client::CheckForNewVersionsResult> Watcher;
    if (!testAppInit())
        return;
    Future f = QtConcurrent::run(&Client::checkForNewVersions, persistent);
    Watcher *w = new Watcher;
    w->setFuture(f);
    connect(w, SIGNAL(finished()), bApp, SLOT(checkingForNewVersionsFinished()));
    bApp->futureWatchers << w;*/
}

BSpellChecker *Application::spellChecker()
{
    return bApp ? bApp->mspellChecker : 0;
}

void Application::resetProxy()
{
    switch (Global::proxyMode())
    {
    case Global::NoProxy:
        QNetworkProxy::setApplicationProxy(QNetworkProxy());
        break;
    case Global::SystemProxy:
    {
        QList<QNetworkProxy> list = QNetworkProxyFactory::systemProxyForQuery(
                    QNetworkProxyQuery(QUrl("http://www.google.com")));
        if (!list.isEmpty())
            QNetworkProxy::setApplicationProxy(list.first());
        else
            QNetworkProxy::setApplicationProxy(QNetworkProxy());
        break;
    }
    case Global::UserProxy:
        QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, Global::proxyHost(),
                                                         (quint16) Global::proxyPort(), Global::proxyLogin(),
                                                         Global::proxyPassword()));
        break;
    default:
        break;
    }
}

void Application::windowAboutToClose(MainWindow *mw)
{
    if (!mw)
        return;
    foreach (BPluginWrapper *pw, pluginWrappers("editor-module"))
    {
        if (!pw)
            continue;
        CodeEditorModulePluginInterface *i = qobject_cast<CodeEditorModulePluginInterface *>(pw->instance());
        if (!i)
            continue;
        i->uninstallModule(mw->codeEditor(), mw);
    }
}

/*============================== Public slots ==============================*/

void Application::checkForNewVersionsSlot()
{
    checkForNewVersions(true);
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
        if (!pw)
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
    /*if (!Global::hasTexsample())
    {
        QMessageBox msg(mostSuitableWindow());
        msg.setWindowTitle( tr("TeXSample configuration", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Question);
        msg.setText( tr("It seems that you have not configured TeXSample service yet.\n"
                        "Would you like to do it now?", "msgbox text") );
        msg.setInformativeText( tr("To remove this notification, you have to configure or disable the service",
                                   "msgbox informativeText") );
        QPushButton *btn1 = msg.addButton(tr("Register", "btn text"), QMessageBox::AcceptRole);
        QPushButton *btn2 = msg.addButton(tr("I have an account", "btn text"), QMessageBox::AcceptRole);
        QPushButton *btn3 = msg.addButton(tr("Disable TeXSample", "btn text"), QMessageBox::RejectRole);
        msg.addButton(tr("Not right now", "btn text"), QMessageBox::RejectRole);
        msg.setDefaultButton(btn2);
        msg.exec();
        if (msg.clickedButton() == btn1)
        {
            if (!showRegisterDialog())
                return;
            Global::setAutoconnection(true);
        }
        else if (msg.clickedButton() == btn2)
        {
            if (BSettingsDialog(new TexsampleSettingsTab).exec() != BSettingsDialog::Accepted)
                return;
            //sClient->connectToServer();
        }
        else if (msg.clickedButton() == btn3)
        {
            return Global::setAutoconnection(false);
        }
    }
    else if (Global::autoconnection())
    {
        //sClient->connectToServer();
    }*/
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

void Application::checkingForNewVersionsFinished()
{
    /*typedef QFutureWatcher<Client::CheckForNewVersionsResult> Watcher;
    Watcher *w = dynamic_cast<Watcher *>(sender());
    if (!w)
        return;
    bApp->futureWatchers.removeAll(w);
    Client::CheckForNewVersionsResult r = w->result();
    delete w;
    QMessageBox msg(mostSuitableWindow());
    msg.setWindowTitle(tr("New version", "msgbox windowTitle"));
    msg.setIcon(QMessageBox::Information);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    if (r.version.isValid() && r.version > BVersion(applicationVersion()))
    {
        msg.setText(tr("A new version of the application is available", "msgbox text")
                    + " (v" + r.version.toString(BVersion::Full) + "). " +
                    tr("Click the following link to go to the download page:", "msgbox text")
                    + " <a href=\"" + r.url + "\">" + tr("download", "msgbox text") + "</a>");
        msg.setInformativeText(tr("You should always use the latest application version. "
                                  "Bugs are fixed and new features are implemented in new versions.",
                                  "msgbox informativeText"));
        msg.exec();
    }
    else if (r.persistent)
    {
        msg.setText(tr("You are using the latest version.", "msgbox text"));
        msg.exec();
    }*/
}

void Application::directoryChanged(const QString &path)
{
    if (locations("autotext").contains(path))
        emit reloadAutotexts();
    else
        reloadDictionaries();
    mfsWatcher->addPath(path);
}

void Application::fileHistoryChanged(const QStringList &history)
{
    Global::setFileHistory(history);
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
