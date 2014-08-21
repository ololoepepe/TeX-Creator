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

#include "texsamplecore.h"

#include "application.h"
#include "cache.h"
#include "mainwindow.h"
#include "samplemodel.h"
#include "settings.h"

#include <TClientInfo>
#include <TGetLatestAppVersionReplyData>
#include <TGetLatestAppVersionRequestData>
#include <TGroupModel>
#include <TGroupWidget>
#include <TInviteModel>
#include <TInviteWidget>
#include <TNetworkClient>
#include <TOperation>
#include <TRecoveryWidget>
#include <TReply>
#include <TUserInfoWidget>
#include <TUserModel>
#include <TUserWidget>

#include <BCodeEditor>
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
#include <QThread>
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
    mdestructorCalled = false;
    BLocationProvider *provider = new BLocationProvider;
    provider->addLocation("texsample");
    provider->createLocationPath("texsample", Application::UserResource);
    Application::installLocationProvider(provider);
    mcache = new Cache(BDirTools::findResource("texsample", BDirTools::UserOnly));
    mclient = new TNetworkClient;
    mclient->setShowMessageFunction(&showMessageFunction);
    mclient->setWaitForConnectedFunction(&waitForConnectedFunction);
    mclient->setWaitForFinishedFunction(&waitForFinishedFunction);
    mclient->setWaitForConnectedDelay(BeQt::Second / 2);
    mclient->setWaitForFinishedDelay(BeQt::Second / 2);
    mclient->setWaitForConnectedTimeout(10 * BeQt::Second);
    Settings::Texsample::loadPassword();
    updateClientSettings();
    mgroupModel = new TGroupModel;
    minviteModel = new TInviteModel;
    msampleModel = new SampleModel;
    muserModel = new TUserModel;
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
    mdestructorCalled = true;
    delete mclient;
    delete mgroupModel;
    delete minviteModel;
    delete msampleModel;
    delete muserModel;
    emit stopWaiting();
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

TNetworkClient *TexsampleCore::client() const
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

void TexsampleCore::connectToServer()
{
    if (!mclient->isValid())
        showTexsampleSettings();
    mclient->connectToServer();
}

bool TexsampleCore::deleteSample(quint64 sampleId, QWidget *parent)
{
    return false;
    //TODO
    /*QString title = tr("Deleting sample", "idlg title");
    QString lblText = tr("You are going to delete a sample. Please, enter the reason:", "idlg lblText");
    bool ok = false;
    QString reason = QInputDialog::getText(Window->codeEditor(), title, lblText, QLineEdit::Normal, QString(), &ok);
    if (!ok)
        return;
    TOperationResult r = sClient->deleteSample(mlastId, reason, Window->codeEditor());
    if (!r)
    {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Deleting sample error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to delete sample due to the following error:", "msgbox text"));
        msg.setInformativeText(r.messageString());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    emit message(tr("Sample was successfully deleted", "message"));*/
}

void TexsampleCore::disconnectFromServer()
{
    mclient->disconnectFromServer();
}

void TexsampleCore::editSample(quint64 sampleId, BCodeEditor *editor)
{
    //TODO
}

bool TexsampleCore::insertSample(quint64 sampleId, BCodeEditor *editor)
{
    return false;
    //TODO
    /*BAbstractCodeEditorDocument *doc = Window->codeEditor()->currentDocument();
    if (!doc)
        return;
    QFileInfo fi(doc->fileName());
    QString subdir = fi.path() + "/texsample-" + QString::number(mlastId);
    if (!fi.isAbsolute() || !fi.isFile() || !BDirTools::mkpath(subdir))
        return;
    BFileDialog dlg(fi.path(), this);
    dlg.setCodecSelectionEnabled(false);
    dlg.selectFile(subdir);
    if (!dlg.restoreGeometry(bSettings->value("TexsampleWidget/sample_subdir_dialog_geometry").toByteArray()))
        dlg.resize(700, 400);
    dlg.restoreState(bSettings->value("TexsampleWidget/sample_subdir_dialog_state").toByteArray());
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::Directory);
    bool b = dlg.exec() == BFileDialog::Accepted;
    bSettings->setValue("TexsampleWidget/sample_subdir_dialog_geometry", dlg.saveGeometry());
    bSettings->setValue("TexsampleWidget/sample_subdir_dialog_state", dlg.saveState());
    QStringList files = dlg.selectedFiles();
    b = b && !files.isEmpty();
    if (!b)
        return bRet(BDirTools::rmdir(subdir));
    subdir = files.first().remove(fi.path() + "/");
    TOperationResult r = sClient->insertSample(mlastId, doc, subdir);
    if (!r)
    {
        QMessageBox msg(window());
        msg.setWindowTitle(tr("Failed to insert sample", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to get or insert sample. The following error occured:", "msgbox text"));
        msg.setInformativeText(r.messageString());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    emit message(tr("Sample was successfully inserted", "message"));*/
}

bool TexsampleCore::saveSample(quint64 sampleId, QWidget *parent)
{
    return false;
    //TODO
    /*const TSampleInfo *info = sModel->sample(mlastId);
    if (!info)
        return;
    QString fn = QFileDialog::getSaveFileName(this, "Select directory", QDir::homePath() + "/" + info->fileName());
    if (fn.isEmpty())
        return;
    if (!sClient->saveSample(mlastId, fn))
    {
        QMessageBox msg( window() );
        msg.setWindowTitle( tr("Failed to save sample", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to get or save sample", "msgbox text") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    emit message(tr("Sample was successfully saved", "message"));*/
}

void TexsampleCore::sendSample(BCodeEditor *editor)
{
    //TODO
}

bool TexsampleCore::showAccountSettings(QWidget *parent)
{
    return false;
    //TODO
}

bool TexsampleCore::showConfirmRegistrationDialog(QWidget *parent)
{
    return false;
    //TODO
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
    mgroupManagementWidget = wgt;
    wgt->show();
}

void TexsampleCore::showInviteManagementWidget()
{
    if (!minviteManagementWidget.isNull())
        return minviteManagementWidget->activateWindow();
    QWidget *wgt = new QWidget;
    wgt->setAttribute(Qt::WA_DeleteOnClose, true);
    BTranslation t = BTranslation::translate("Application", "Invite management", "wgt windowTitle");
    wgt->setWindowTitle(t);
    new BDynamicTranslator(wgt, "windowTitle", t);
    QVBoxLayout *vlt = new QVBoxLayout(wgt);
    TInviteWidget *iwgt = new TInviteWidget(minviteModel);
    iwgt->setClient(mclient);
    iwgt->setCache(mcache);
    vlt->addWidget(iwgt);
    QDialogButtonBox *dlgbbox = new QDialogButtonBox;
    connect(dlgbbox->addButton(QDialogButtonBox::Close), SIGNAL(clicked()), wgt, SLOT(close()));
    vlt->addWidget(dlgbbox);
    wgt->resize(800, 400);
    minviteManagementWidget = wgt;
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

void TexsampleCore::showSampleInfo(quint64 sampleId)
{
    //TODO
    /*if (minfoDialogMap.contains(mlastId))
    {
        if (minfoDialogMap.value(mlastId).isNull())
        {
            minfoDialogMap.remove(mlastId);
            minfoDialogIdMap.remove(QPointer<QObject>());
        }
        else
        {
            return minfoDialogMap.value(mlastId)->activateWindow();
        }
    }
    const TSampleInfo *s = sModel->sample(mlastId);
    if (!s)
        return;
    QDialog *dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("Sample:", "windowTitle") + " " + s->title());
    QVBoxLayout *vlt = new QVBoxLayout(dlg);
      SampleInfoWidget *swgt = new SampleInfoWidget(SampleInfoWidget::ShowMode);
        swgt->setInfo(*s);
      vlt->addWidget(swgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Close);
        connect(dlgbbox->button(QDialogButtonBox::Close), SIGNAL(clicked()), dlg, SLOT(close()));
      vlt->addWidget(dlgbbox);
      dlg->resize(bSettings->value("TexsampleWidget/sample_info_dialog_size", QSize(750, 550)).toSize());
    minfoDialogMap.insert(mlastId, dlg);
    minfoDialogIdMap.insert(dlg, mlastId);
    connect(dlg, SIGNAL(finished(int)), this, SLOT(infoDialogFinished()));
    dlg->show();*/
}

void TexsampleCore::showSamplePreview(quint64 sampleId)
{
    //TODO
    /*if ( !sClient->previewSample(mlastId) )
    {
        QMessageBox msg( window() );
        msg.setWindowTitle( tr("Failed to show preview", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to get or show sample preview", "msgbox text") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }*/
}

bool TexsampleCore::showTexsampleSettings(QWidget *parent)
{
    return bApp->showSettings(Application::TexsampleSettings, parent);
}

void TexsampleCore::showUserInfo(quint64 userId)
{
    //TODO
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
    uwgt->setAlwaysRequestAvatar(true);
    vlt->addWidget(uwgt);
    QDialogButtonBox *dlgbbox = new QDialogButtonBox;
    connect(dlgbbox->addButton(QDialogButtonBox::Close), SIGNAL(clicked()), wgt, SLOT(close()));
    vlt->addWidget(dlgbbox);
    wgt->resize(800, 400);
    muserManagementWidget = wgt;
    wgt->show();
}

void TexsampleCore::updateSampleList()
{
    //TODO
}

/*============================== Static private methods ====================*/

TexsampleCore::CheckForNewVersionResult TexsampleCore::checkForNewVersionFunction(bool persistent)
{
    CheckForNewVersionResult result(persistent);
    TGetLatestAppVersionRequestData request;
    request.setClientInfo(TClientInfo::create());
    TNetworkClient *client = tSmp->client();
    if (!client->isValid(true))
        return result;
    TReply reply = client->performAnonymousOperation(TOperation::GetLatestAppVersion, request);
    result.success = reply.success();
    if (result.success) {
        TGetLatestAppVersionReplyData data = reply.data().value<TGetLatestAppVersionReplyData>();
        result.url = data.downloadUrl();
        result.version = data.version();
    }
    result.message = reply.message();
    return result;
}

void TexsampleCore::showMessageFunction(const QString &text, const QString &informativeText, bool error,
                                        QWidget *parentWidget)
{
    if (QThread::currentThread() != bApp->thread())
        return TNetworkClient::defaultShowMessageFunction(text, informativeText, error, parentWidget);
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
    if (!connection)
        return bRet(msg, tr("Null connection pointer", "error"), false);
    if (connection->error() != QAbstractSocket::UnknownSocketError)
        return bRet(msg, connection->errorString(), false);
    if (connection->isConnected())
        return bRet(msg, QString(), true);
    if (QThread::currentThread() != bApp->thread()) {
        BeQt::waitNonBlocking(connection, SIGNAL(connected()), tSmp, SIGNAL(stopWaiting()), timeout);
        if (connection->isConnected())
            return bRet(msg, QString(), true);
        return bRet(msg, connection->errorString(), false);
    }
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
    else if (connection->error() != QAbstractSocket::UnknownSocketError)
        return bRet(msg, connection->errorString(), false);
    else
        return bRet(msg, tr("An error occured, or connection timed out", "error"), false);
}

bool TexsampleCore::waitForFinishedFunction(BNetworkOperation *op, int timeout, QWidget *parentWidget, QString *msg)
{
    if (!op)
        return bRet(msg, tr("Null operation pointer", "error"), false);
    if (op->isError())
        return bRet(msg, tr("An error occured during operation", "error"), false);
    if (op->isFinished())
        return bRet(msg, QString(), true);
    if (QThread::currentThread() != bApp->thread()) {
        QList<BeQt::Until> until;
        until << BeQt::until(op, SIGNAL(finished()));
        until << BeQt::until(op, SIGNAL(error()));
        until << BeQt::until(tSmp, SIGNAL(stopWaiting()));
        BeQt::waitNonBlocking(until, timeout);
        if (op->isFinished())
            return bRet(msg, QString(), true);
        return bRet(msg, tr("Operation timed out", "error"), false);
    }
    BOperationProgressDialog dlg(op, parentWidget ? parentWidget : bApp->mostSuitableWindow());
    dlg.setWindowTitle(tr("Executing request...", "opdlg windowTitle"));
    dlg.setAutoCloseInterval((timeout > 0) ? timeout : 0);
    if (dlg.exec() == BOperationProgressDialog::Rejected)
        return bRet(msg, tr("Operation cancelled by user", "error"), false);
    if (op->isFinished())
        return bRet(msg, QString(), true);
    else if (op->isError())
        return bRet(msg, tr("An error occured during operation", "error"), false);
    else
        return bRet(msg, tr("Operation timed out", "error"), false);
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
    if (mdestructorCalled)
        return;
    if (!result.success) {
        QMessageBox msg(bApp->mostSuitableWindow());
        msg.setWindowTitle(tr("Checking for new version failed", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to check for new version. The following error occured:", "msgbox text"));
        msg.setInformativeText(result.message);
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
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

/*============================================================================
================================ AddSampleDialog =============================
============================================================================*/

/*============================== Public constructors =======================*/

/*AddSampleDialog::AddSampleDialog(BCodeEditor *editor, QWidget *parent) :
    BDialog(parent)
{
    msmpwgt = new SampleInfoWidget(SampleInfoWidget::AddMode, editor);
    //msmpwgt->setCheckSourceValidity(true);
    setWindowTitle(tr("Sending sample..."));
    //msmpwgt->setInfo(bSettings->value("AddSampleDialog/sample_widget_info").value<TSampleInfo>());
    msmpwgt->restoreState(bSettings->value("TexsampleWidget/sample_widget_state").toByteArray());
    //msmpwgt->restoreSourceState(bSettings->value("AddSampleDialog/sample_widget_source_state").toByteArray());
    setWidget(msmpwgt);
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    addButton(tr("Clear", "btn text"), QDialogButtonBox::ActionRole, msmpwgt, SLOT(clear()));
    button(QDialogButtonBox::Ok)->setEnabled(msmpwgt->hasValidInput());
    connect(msmpwgt, SIGNAL(validityChanged(bool)), button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
    connect(button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    restoreGeometry(bSettings->value("AddSampleDialog/geometry").toByteArray());
}*/

/*============================== Public methods ============================*/

/*SampleInfoWidget *AddSampleDialog::sampleInfoWidget() const
{
    return msmpwgt;
}*/

/*============================== Protected methods =========================*/

/*void AddSampleDialog::closeEvent(QCloseEvent *e)
{
    //bSettings->setValue("AddSampleDialog/sample_widget_info", msmpwgt->info());
    bSettings->setValue("TexsampleWidget/sample_widget_state", msmpwgt->saveState());
    //bSettings->setValue("AddSampleDialog/sample_widget_source_state", msmpwgt->saveSourceState());
    bSettings->setValue("AddSampleDialog/geometry", saveGeometry());
    e->accept();
}*/

/*============================================================================
================================ EditSampleDialog ============================
============================================================================*/

/*============================== Public constructors =======================*/

/*EditSampleDialog::EditSampleDialog(BCodeEditor *editor, quint64 id, QWidget *parent) :
    BDialog(parent)
{
    if (!id)
        return;
    bool moder = sClient->accessLevel() >= TAccessLevel::ModeratorLevel;
    const TSampleInfo *info = sModel->sample(id);
    if (!info)
        return;
    msmpwgt = new SampleInfoWidget(moder ? SampleInfoWidget::EditMode : SampleInfoWidget::UpdateMode, editor);
    setWindowTitle((moder ? tr("Editing sample:") : tr("Updating sample:")) + " " + info->title());
    msmpwgt->restoreState(bSettings->value("TexsampleWidget/sample_widget_state").toByteArray());
    msmpwgt->setInfo(*info);
    setWidget(msmpwgt);
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    addButton(tr("Clear", "btn text"), QDialogButtonBox::ActionRole, msmpwgt, SLOT(clear()));
    button(QDialogButtonBox::Ok)->setEnabled(msmpwgt->isValid());
    connect(msmpwgt, SIGNAL(validityChanged(bool)), button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
    connect(button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    restoreGeometry(bSettings->value("EditSampleDialog/geometry").toByteArray());
}*/

/*============================== Public methods ============================*/

/*SampleInfoWidget *EditSampleDialog::sampleInfoWidget() const
{
    return msmpwgt;
}*/

/*============================== Protected methods =========================*/

/*void EditSampleDialog::closeEvent(QCloseEvent *e)
{
    bSettings->setValue("TexsampleWidget/sample_widget_state", msmpwgt->saveState());
    bSettings->setValue("EditSampleDialog/geometry", saveGeometry());
    e->accept();
}*/

/*============================================================================
================================ SelectUserDialog ============================
============================================================================*/

/*============================== Public constructors =======================*/

/*SelectUserDialog::SelectUserDialog(QWidget *parent) :
    BDialog(parent)
{
    QButtonGroup *btngr = new QButtonGroup(this);
    connect(btngr, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));
    QWidget *wgt = new QWidget;
      QFormLayout *flt = new QFormLayout(wgt);
        QHBoxLayout *hlt = new QHBoxLayout;
          QRadioButton *rbtn = new QRadioButton(tr("ID", "rbtn text"));
          hlt->addWidget(rbtn);
          btngr->addButton(rbtn, 0);
          rbtn = new QRadioButton(tr("Login", "rbtn text"));
            rbtn->setChecked(true);
          hlt->addWidget(rbtn);
          btngr->addButton(rbtn, 1);
        flt->addRow(tr("Identifier:", "lbl text"), hlt);
        mledt = new QLineEdit;
          connect(mledt, SIGNAL(textChanged(QString)), this, SLOT(checkValidity()));
          mfield = new BInputField;
          mfield->addWidget(mledt);
        flt->addRow(tr("Value:", "lbl text"), mfield);
      wgt->setLayout(flt);
    setWidget(wgt);
    //
    addButton(QDialogButtonBox::Ok, SLOT(accept()));
    addButton(QDialogButtonBox::Cancel, SLOT(reject()));
    buttonClicked(1);
}*/

/*============================== Public methods ============================*/

/*quint64 SelectUserDialog::userId() const
{
    return mledt->text().toULongLong();
}

QString SelectUserDialog::userLogin() const
{
    return mledt->text();
}*/

/*============================== Private slots =============================*/

/*void SelectUserDialog::buttonClicked(int id)
{
    if (id)
    {
        delete mledt->validator();
    }
    else
    {
        QIntValidator *v = new QIntValidator(mledt);
        v->setBottom(1);
        mledt->setValidator(v);
    }
    mledt->setFocus();
    mledt->selectAll();
    checkValidity();
}

void SelectUserDialog::checkValidity()
{
    bool b = !mledt->text().isEmpty() && mledt->hasAcceptableInput();
    mfield->setValid(b);
    button(QDialogButtonBox::Ok)->setEnabled(b);
}*/

/*void TexsampleWidget::addDialogFinished()
{
    if (maddDialog.isNull())
        return;
    if (maddDialog->result() == AddSampleDialog::Accepted)
    {
        SampleInfoWidget *smpwgt = maddDialog->sampleInfoWidget();
        QString fn = smpwgt->actualFileName();
        QString text = smpwgt->document() ? smpwgt->document()->text() : BDirTools::readTextFile(fn);
        TCompilationResult r = sClient->addSample(smpwgt->info(), fn, smpwgt->codec(), text, this);
        if (!r)
            return showAddingSampleFailedMessage(r.messageString());
        emit message(tr("Sample was successfully sent", "message"));
    }
    maddDialog->close();
    maddDialog->deleteLater();
}*/

/*void TexsampleWidget::editDialogFinished()
{
    EditSampleDialog *dlg = qobject_cast<EditSampleDialog *>(sender());
    if (!dlg)
        return;
    if (dlg->result() == EditSampleDialog::Accepted)
    {
        bool moder = sClient->accessLevel() >= TAccessLevel::ModeratorLevel;
        TSampleInfo info = dlg->sampleInfoWidget()->info();
        SampleInfoWidget *smpwgt = dlg->sampleInfoWidget();
        QString fn = smpwgt->actualFileName();
        QString text = smpwgt->document() ? smpwgt->document()->text() : BDirTools::readTextFile(fn);
        TCompilationResult r = moder ? sClient->editSample(info, fn, smpwgt->codec(), text, this) :
                                       sClient->updateSample(info, fn, smpwgt->codec(), text, this);
        if (!r)
            return showEditingSampleFailedMessage(r.messageString());
        if (moder)
            emit message(tr("Sample was successfully edited", "message"));
        else
            emit message(tr("Sample was successfully updated", "message"));
    }
    meditDialogMap.remove(meditDialogIdMap.take(dlg));
    dlg->close();
    dlg->deleteLater();
}*/

//bSettings->setValue("TexsampleWidget/sample_info_dialog_size", dlg->size());

/*TOperationResult Client::updateAccount(TUserInfo info, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    info.setId(mid);
    if (!info.isValid(TUserInfo::UpdateContext))
        return TOperationResult(TMessage::InvalidUserInfoError);
    QVariantMap out;
    out.insert("user_info", info);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::UpdateAccountRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::getUserInfo(quint64 id, TUserInfo &info, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    if (!id)
        return TOperationResult(TMessage::InvalidUserIdError);
    QVariantMap out;
    out.insert("user_id", id);
    out.insert("update_dt", sCache->userInfoUpdateDateTime(id));
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetUserInfoRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    QDateTime dt = in.value("update_dt").toDateTime();
    if (in.value("cache_ok").toBool())
    {
        info = sCache->userInfo(id);
        sCache->cacheUserInfo(id, dt);
    }
    else
    {
        info = in.value("user_info").value<TUserInfo>();
        sCache->cacheUserInfo(info, dt);
    }
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::getUserInfo(const QString &login, TUserInfo &info, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    if (login.isEmpty())
        return TOperationResult(TMessage::InvalidLoginError);
    QVariantMap out;
    out.insert("user_login", login);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetUserInfoRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    info = in.value("user_info").value<TUserInfo>();
    return in.value("operation_result").value<TOperationResult>();
}

TCompilationResult Client::addSample(const TSampleInfo &info, const QString &fileName, QTextCodec *codec,
                                     const QString &text, QWidget *parent)
{
    if (!isAuthorized())
        return TCompilationResult(TMessage::NotAuthorizedError);
    if (fileName.isEmpty() || text.isEmpty() || !info.isValid(TSampleInfo::AddContext))
        return TCompilationResult(TMessage::InvalidDataError);
    TTexProject p(fileName, text, codec);
    if (!p.isValid())
        return TCompilationResult(TMessage::ClientFileSystemError);
    p.removeRestrictedCommands();
    QVariantMap out;
    out.insert("project", p);
    out.insert("sample_info", info);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::AddSampleRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TCompilationResult(TMessage::ClientOperationError);
    TCompilationResult r = in.value("compilation_result").value<TCompilationResult>();
    if (r)
        updateSamplesList();
    return r;
}

TCompilationResult Client::editSample(const TSampleInfo &newInfo, const QString &fileName, QTextCodec *codec,
                                      const QString &text, QWidget *parent)
{
    if (!isAuthorized())
        return TCompilationResult(TMessage::NotAuthorizedError);
    if (!newInfo.isValid(TSampleInfo::EditContext))
        return TCompilationResult(TMessage::ClientInvalidSampleInfoError);
    QVariantMap out;
    out.insert("sample_info", newInfo);
    if (!fileName.isEmpty() && codec && !text.isEmpty())
    {
        TTexProject p(fileName, text, codec);
        if (!p.isValid())
            return TCompilationResult(TMessage::ClientFileSystemError);
        p.removeRestrictedCommands();
        out.insert("project", p);
    }
    BNetworkOperation *op = mconnection->sendRequest(Texsample::EditSampleRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TCompilationResult(TMessage::ClientOperationError);
    TCompilationResult r = in.value("compilation_result").value<TCompilationResult>();
    if (r)
        updateSamplesList();
    return r;
}

TCompilationResult Client::updateSample(const TSampleInfo &newInfo, const QString &fileName, QTextCodec *codec,
                                        const QString &text, QWidget *parent)
{
    if (!isAuthorized())
        return TCompilationResult(TMessage::NotAuthorizedError);
    if (!newInfo.isValid(TSampleInfo::UpdateContext))
        return TCompilationResult(TMessage::ClientInvalidSampleInfoError);
    QVariantMap out;
    out.insert("sample_info", newInfo);
    if (!fileName.isEmpty() && codec && !text.isEmpty())
    {
        TTexProject p(fileName, text, codec);
        if (!p.isValid())
            return TCompilationResult(TMessage::ClientFileSystemError);
        p.removeRestrictedCommands();
        out.insert("project", p);
    }
    BNetworkOperation *op = mconnection->sendRequest(Texsample::UpdateSampleRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TCompilationResult(TMessage::ClientOperationError);
    TCompilationResult r = in.value("compilation_result").value<TCompilationResult>();
    if (r)
        updateSamplesList();
    return r;
}

TOperationResult Client::deleteSample(quint64 id, const QString &reason, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    if (!id)
        return TOperationResult(TMessage::InvalidSampleIdError);
    QVariantMap out;
    out.insert("sample_id", id);
    out.insert("reason", reason);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::DeleteSampleRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    TOperationResult r = in.value("operation_result").value<TOperationResult>();
    if (r)
        updateSamplesList();
    return r;
}

TOperationResult Client::updateSamplesList(bool full, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    QVariantMap out;
    out.insert("update_dt", !full ? sampleInfosUpdateDateTime() : QDateTime());
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetSamplesListRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    TOperationResult r = in.value("operation_result").value<TOperationResult>();
    if (r)
        updateSampleInfos(in.value("new_sample_infos").value<TSampleInfoList>(),
                          in.value("deleted_sample_infos").value<TIdList>(), in.value("update_dt").toDateTime());
    return r;
}

TOperationResult Client::insertSample(quint64 id, BAbstractCodeEditorDocument *doc, const QString &subdir)
{
    QFileInfo sfi(subdir);
    if (!sfi.fileName().indexOf(QRegExp("^texsample\\-\\d+$")))
    {
        //TODO: Improve
        QString sfn = sCache->sampleInfo(sfi.fileName().split('-').last().toULongLong()).fileName();
        QString path = QFileInfo(doc->fileName()).path() + "/" + subdir + "/" + sfn;
        if (!sfn.isEmpty() && QFileInfo(path).isFile())
        {
            QMessageBox msg(doc->editor());
            msg.setWindowTitle(tr("Updating sample", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Question);
            msg.setText(tr("It seems like there is some sample in the selected directory", "msgbox text"));
            msg.setInformativeText(tr("Do you want to update it, or use the existing one?", "magbox informativeText"));
            msg.addButton(tr("Update", "btn text"), QMessageBox::AcceptRole);
            QPushButton *btnEx = msg.addButton(tr("Use existing", "btn text"), QMessageBox::AcceptRole);
            msg.setDefaultButton(btnEx);
            msg.addButton(QMessageBox::Cancel);
            if (msg.exec() == QMessageBox::Cancel)
                return TOperationResult(true);
            if (msg.clickedButton() == btnEx)
            {
                doc->insertText("\\input " + BTextTools::wrapped(subdir + "/" + sfn));
                return TOperationResult(true);
            }
            if (!BDirTools::removeFilesInDir(QFileInfo(path).path()))
                return TOperationResult(TMessage::ClientFileSystemError);
        }
    }
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    if (!id || !doc || subdir.isEmpty() || subdir.contains(QRegExp("\\s")))
        return TOperationResult(TMessage::InvalidDataError);
    QFileInfo fi(doc->fileName());
    if (!fi.exists() || !fi.isFile())
        return TOperationResult(TMessage::InvalidDataError);
    QString path = fi.path() + "/" + subdir;
    if ((QFileInfo(path).isDir() && !BDirTools::rmdir(path)) || !BDirTools::mkpath(path))
        return TOperationResult(TMessage::ClientFileSystemError);
    QVariantMap out;
    out.insert("sample_id", id);
    out.insert("update_dt", sCache->sampleSourceUpdateDateTime(id));
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetSampleSourceRequest, out);
    showProgressDialog(op);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    TOperationResult r = in.value("operation_result").value<TOperationResult>();
    if (!r)
        return r;
    TTexProject p = (in.value("cache_ok").toBool() && sCache->isValid()) ? sCache->sampleSource(id) :
                                                                           in.value("project").value<TTexProject>();
    sCache->cacheSampleSource(id, in.value("update_dt").toDateTime(), in.value("project").value<TTexProject>());
    r.setSuccess(p.prependExternalFileNames(subdir) && p.save(path, doc->codec()));
    if (!r)
        r.setMessage(TMessage::ClientFileSystemError);
    else
        doc->insertText("\\input " + BTextTools::wrapped(QFileInfo(path).fileName() + "/" + p.rootFileName()));
    return r;
}

TOperationResult Client::saveSample(quint64 id, const QString &fileName, QTextCodec *codec)
{
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    if (!id || fileName.isEmpty())
        return TOperationResult(TMessage::InvalidDataError);
    QString path = QFileInfo(fileName).path();
    if (!QFileInfo(path).isDir())
        return TOperationResult(TMessage::ClientInvalidPathError);
    QVariantMap out;
    out.insert("sample_id", id);
    out.insert("update_dt", sCache->sampleSourceUpdateDateTime(id));
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetSampleSourceRequest, out);
    showProgressDialog(op);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    TOperationResult r = in.value("operation_result").value<TOperationResult>();
    if (!r)
        return r;
    TTexProject p = (in.value("cache_ok").toBool() && sCache->isValid()) ? sCache->sampleSource(id) :
                                                                           in.value("project").value<TTexProject>();
    sCache->cacheSampleSource(id, in.value("update_dt").toDateTime(), in.value("project").value<TTexProject>());
    p.rootFile()->setFileName(fileName);
    r.setSuccess(p.save(path, codec));
    if (!r)
        r.setMessage(TMessage::ClientFileSystemError);
    return r;
}

TOperationResult Client::previewSample(quint64 id, QWidget *parent, bool) //"bool full" will be used in later versions
{
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    if (!id)
        return TOperationResult(TMessage::InvalidSampleIdError);
    QVariantMap out;
    out.insert("sample_id", id);
    out.insert("update_dt", sCache->samplePreviewUpdateDateTime(id));
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetSamplePreviewRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    TOperationResult r = in.value("operation_result").value<TOperationResult>();
    if (!r)
        return r;
    TProjectFile pf = in.value("project_file").value<TProjectFile>();
    sCache->cacheSamplePreview(id, in.value("update_dt").toDateTime(), pf);
    if (sCache->isValid())
    {
        r.setSuccess(bApp->openLocalFile(sCache->samplePreviewFileName(id)));
    }
    else
    {
        QString path = QDir::tempPath() + "/tex-creator/previews";
        r.setSuccess(pf.save(path) && bApp->openLocalFile(path + "/" + pf.fileName()));
        BDirTools::rmdir(path);
    }
    if (!r)
        r.setMessage(TMessage::ClientFileSystemError);
    return r;
}*/

/*============================== Private methods ===========================*/

/*void Client::updateSampleInfos(const TSampleInfoList &newInfos, const TIdList &deletedInfos, const QDateTime &updateDT)
{
    msamplesListUpdateDT = updateDT.toUTC();
    sModel->removeSamples(deletedInfos);
    sModel->insertSamples(newInfos);
    sCache->removeSamples(deletedInfos);
    sCache->cacheSampleInfos(newInfos, updateDT);
}

QDateTime Client::sampleInfosUpdateDateTime(Qt::TimeSpec spec) const
{
    if (!msamplesListUpdateDT.isValid())
        msamplesListUpdateDT = sCache->sampleInfosUpdateDateTime();
    return msamplesListUpdateDT.toTimeSpec(spec);
}*/
