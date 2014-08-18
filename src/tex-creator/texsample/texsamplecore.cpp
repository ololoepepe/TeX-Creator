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

#include "texsamplecore.h"

#include "application.h"
#include "cache.h"
#include "client.h"
#include "mainwindow.h"
#include "samplemodel.h"
#include "settings.h"

#include <TClientInfo>
#include <TGetLatestAppVersionReplyData>
#include <TGetLatestAppVersionRequestData>
#include <TGroupModel>
#include <TGroupWidget>
#include <TInviteModel>
#include <TOperation>
#include <TRecoveryWidget>
#include <TReply>
#include <TUserInfoWidget>
#include <TUserModel>
#include <TUserWidget>

#include <BDialog>
#include <BDirTools>
#include <BDynamicTranslator>
#include <BeQt>
#include <BLocationProvider>
#include <BNetworkConnection>
#include <BNetworkOperation>
#include <BOperationProgressDialog>
#include <BPassword>
#include <BTranslation>
#include <BVersion>

#include <QAbstractSocket>
#include <QByteArray>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFuture>
#include <QFutureWatcher>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QProgressDialog>
#include <QPushButton>
#include <QString>
#include <QtConcurrentRun>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

/*============================================================================
================================ TexsampleCore::CheckForNewVersionResult =====
============================================================================*/

/*============================== Public constructors =======================*/

TexsampleCore::CheckForNewVersionResult::CheckForNewVersionResult(bool persistent)
{
    this->persistent = persistent;
    success = false;
}

/*============================================================================
================================ TexsampleCore ===============================
============================================================================*/

/*============================== Public constructors =======================*/

TexsampleCore::TexsampleCore(QObject *parent) :
    QObject(parent)
{
    BLocationProvider *provider = new BLocationProvider;
    provider->addLocation("texsample");
    provider->createLocationPath("texsample", Application::UserResource);
    Application::installLocationProvider(provider);
    mcache = new Cache(BDirTools::findResource("texsample", BDirTools::UserOnly));
    mclient = new Client(this);
    mclient->setShowMessageFunction(&showMessageFunction);
    mclient->setWaitForConnectedFunction(&waitForConnectedFunction);
    mclient->setWaitForFinishedFunction(&waitForFinishedFunction);
    mclient->setWaitForConnectedDelay(BeQt::Second / 2);
    mclient->setWaitForFinishedDelay(BeQt::Second / 2);
    mclient->setWaitForConnectedTimeout(10 * BeQt::Second);
    Settings::Texsample::loadPassword();
    updateClientSettings();
    mgroupModel = new TGroupModel(this);
    minviteModel = new TInviteModel(this);
    msampleModel = new SampleModel(this);
    muserModel = new TUserModel(this);
    //
    bool b = true;
    if (!Settings::Texsample::hasTexsample()) {
        QMessageBox msg(bApp->mostSuitableWindow());
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
            if (showTexsampleSettings())
                mclient->connectToServer();
            else
                b = false;
        } else if (msg.clickedButton() == btnDisable) {
            Settings::Texsample::setConnectOnStartup(false);
            b = false;
        }
    } else if (Settings::Texsample::connectOnStartup()) {
        if (!mclient->isValid())
            b = showTexsampleSettings();
        mclient->connectToServer();
    }
    if (Settings::General::checkForNewVersionOnStartup() && (b || mclient->isValid(true)))
        checkForNewVersion();
}

TexsampleCore::~TexsampleCore()
{
    while (!mfutureWatchers.isEmpty()) {
        Watcher *w = dynamic_cast<Watcher *>(mfutureWatchers.takeLast());
        if (!w)
            continue;
        w->waitForFinished();
    }
}

/*============================== Public methods ============================*/

Cache *TexsampleCore::cache() const
{
    return mcache;
}

Client *TexsampleCore::client() const
{
    return mclient;
}

TGroupModel *TexsampleCore::groupModel() const
{
    return mgroupModel;
}

TInviteModel *TexsampleCore::inviteModel() const
{
    return minviteModel;
}

SampleModel *TexsampleCore::sampleModel() const
{
    return msampleModel;
}

void TexsampleCore::updateClientSettings()
{
    mclient->setHostName(Settings::Texsample::host(true));
    mclient->setLogin(Settings::Texsample::login());
    mclient->setPassword(Settings::Texsample::password().encryptedPassword());
    mclient->reconnect();
}

TUserModel *TexsampleCore::userModel() const
{
    return muserModel;
}

/*============================== Public slots ==============================*/

bool TexsampleCore::checkForNewVersion(bool persistent)
{
    if (!mclient->isValid(true)) {
        if (!bApp->showSettings(Application::TexsampleSettings))
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

bool TexsampleCore::checkForNewVersionPersistent()
{
    return checkForNewVersion(true);
}

void TexsampleCore::showGroupManagementWidget()
{
    if (!mgroupManagementWidget.isNull())
        return mgroupManagementWidget->activateWindow();
    QWidget *wgt = new QWidget;
    wgt->setAttribute(Qt::WA_DeleteOnClose, true);
    BTranslation t = BTranslation::translate("Application", "Group management", "wgt windowTitle");
    wgt->setWindowTitle(t);
    new BDynamicTranslator(wgt, "windowTitle", t);
    QVBoxLayout *vlt = new QVBoxLayout(wgt);
    TGroupWidget *gwgt = new TGroupWidget(mgroupModel);
    gwgt->setClient(mclient);
    gwgt->setCache(mcache);
    vlt->addWidget(gwgt);
    QDialogButtonBox *dlgbbox = new QDialogButtonBox;
    connect(dlgbbox->addButton(QDialogButtonBox::Close), SIGNAL(clicked()), wgt, SLOT(close()));
    vlt->addWidget(dlgbbox);
    wgt->resize(800, 400);
    wgt->show();
}

bool TexsampleCore::showRecoverDialog(QWidget *parent)
{
    if (!mclient->isValid(true) && !showTexsampleSettings(parent))
        return false;
    if (!mclient->isValid(true))
        return false;
    BDialog dlg(parent ? parent : bApp->mostSuitableWindow());
    dlg.setWindowTitle(tr("Account recovery", "dlg windowTitle"));
    dlg.setWidget(new TRecoveryWidget(mclient));
    dlg.addButton(QDialogButtonBox::Close, &dlg, SLOT(reject()));
    dlg.resize(700, 0);
    dlg.exec();
    return true;
}

bool TexsampleCore::showRegisterDialog(QWidget *parent)
{
    if (!mclient->isValid(true) && !showTexsampleSettings(parent))
        return false;
    if (!mclient->isValid(true))
        return false;
    BDialog dlg(parent ? parent : bApp->mostSuitableWindow());
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
        TReply r = mclient->performAnonymousOperation(TOperation::Register, wgt->createRequestData(), parent);
        if (r.success()) {
            Settings::Texsample::setLogin(wgt->login());
            Settings::Texsample::setPassword(wgt->password());
            updateClientSettings();
            if (mclient->isConnected())
                mclient->reconnect();
            else
                mclient->connectToServer();
            bApp->showStatusBarMessage(tr("You have successfully registered", "message"));
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

bool TexsampleCore::showTexsampleSettings(QWidget *parent)
{
    return bApp->showSettings(Application::TexsampleSettings, parent);
}

void TexsampleCore::showUserManagementWidget()
{
    if (!muserManagementWidget.isNull())
        return muserManagementWidget->activateWindow();
    QWidget *wgt = new QWidget;
    wgt->setAttribute(Qt::WA_DeleteOnClose, true);
    BTranslation t = BTranslation::translate("Application", "User management", "wgt windowTitle");
    wgt->setWindowTitle(t);
    new BDynamicTranslator(wgt, "windowTitle", t);
    QVBoxLayout *vlt = new QVBoxLayout(wgt);
    TUserWidget *uwgt = new TUserWidget(muserModel);
    uwgt->setClient(mclient);
    uwgt->setCache(mcache);
    vlt->addWidget(uwgt);
    QDialogButtonBox *dlgbbox = new QDialogButtonBox;
    connect(dlgbbox->addButton(QDialogButtonBox::Close), SIGNAL(clicked()), wgt, SLOT(close()));
    vlt->addWidget(dlgbbox);
    wgt->resize(800, 400);
    wgt->show();
}

/*============================== Static private methods ====================*/

TexsampleCore::CheckForNewVersionResult TexsampleCore::checkForNewVersionFunction(bool persistent)
{
    CheckForNewVersionResult result(persistent);
    TGetLatestAppVersionRequestData request;
    request.setClientInfo(TClientInfo::create());
    Client *client = tSmp->client();
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

void TexsampleCore::showMessageFunction(const QString &text, const QString &informativeText, bool error,
                                        QWidget *parentWidget)
{
    QMessageBox msg(parentWidget ? parentWidget : bApp->mostSuitableWindow());
    if (error) {
        msg.setWindowTitle(tr("TeXSample error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
    } else {
        msg.setWindowTitle(tr("TeXSample message", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Information);
    }
    msg.setText(text);
    msg.setInformativeText(informativeText);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    msg.exec();
}

bool TexsampleCore::waitForConnectedFunction(BNetworkConnection *connection, int timeout, QWidget *parentWidget,
                                             QString *msg)
{
    if (!connection || connection->error() != QAbstractSocket::UnknownSocketError)
        return bRet(msg, tr("An error occured while connecting", "error"), false);
    QProgressDialog pd(parentWidget ? parentWidget : bApp->mostSuitableWindow());
    pd.setWindowTitle(tr("Connecting to server", "pdlg windowTitle"));
    pd.setLabelText(tr("Connecting to server, please, wait...", "pdlg labelText"));
    pd.setMinimum(0);
    pd.setMaximum(0);
    QTimer::singleShot(timeout, &pd, SLOT(close()));
    if (pd.exec() == QProgressDialog::Rejected)
        return bRet(msg, tr("Connection cancelled by user", "error"), false);
    if (connection->isConnected())
        return bRet(msg, QString(), true);
    else
        return bRet(msg, tr("An error occured, or connection timed out", "error"), false);
}

bool TexsampleCore::waitForFinishedFunction(BNetworkOperation *op, int timeout, QWidget *parentWidget, QString *msg)
{
    if (!op || op->isError())
        return bRet(msg, tr("An error occured during operation", "error"), false);
    BOperationProgressDialog dlg(op, parentWidget ? parentWidget : bApp->mostSuitableWindow());
    dlg.setWindowTitle(tr("Executing request...", "opdlg windowTitle"));
    dlg.setAutoCloseInterval((timeout > 0) ? timeout : 0);
    if (dlg.exec() == BOperationProgressDialog::Rejected)
        return bRet(msg, tr("Operation cancelled by user", "error"), false);
    if (op->isFinished())
        return bRet(msg, QString(), true);
    else
        return bRet(msg, tr("An error occured during operation", "error"), false);
}

/*============================== Private slots =============================*/

void TexsampleCore::checkingForNewVersionFinished()
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
    QMessageBox msg(bApp->mostSuitableWindow());
    msg.setWindowTitle(tr("New version", "msgbox windowTitle"));
    msg.setIcon(QMessageBox::Information);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    if (result.version.isValid() && result.version > BVersion(Application::applicationVersion())) {
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
