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
#include "dialog.h"
#include "mainwindow.h"
#include "sampleinfowidget.h"
#include "samplemodel.h"
#include "settings.h"
#include "usermodel.h"

#include <TeXSample/TeXSampleCore>
#include <TeXSample/TeXSampleNetwork>
#include <TeXSample/TeXSampleNetworkWidgets>
#include <TeXSample/TeXSampleWidgets>

#include <BAbstractCodeEditorDocument>
#include <BCodeEditor>
#include <BDialog>
#include <BDirTools>
#include <BDynamicTranslator>
#include <BeQt>
#include <BFileDialog>
#include <BLocationProvider>
#include <BNetworkConnection>
#include <BNetworkOperation>
#include <BOperationProgressDialog>
#include <BPassword>
#include <BTextTools>
#include <BTranslation>
#include <BUuid>
#include <BVersion>

#include <QAbstractSocket>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
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
#include <QTextCodec>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#undef bApp
#define bApp (static_cast<Application *>(BApplication::instance()))

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
    QString texsampleLocation = BDirTools::findResource("texsample", BDirTools::UserOnly);
    mcache = new Cache(texsampleLocation);
    mclient = new TNetworkClient;
    mclient->setShowMessageFunction(&showMessageFunction);
    mclient->setWaitForConnectedFunction(&waitForConnectedFunction);
    mclient->setWaitForFinishedFunction(&waitForFinishedFunction);
    mclient->setWaitForConnectedDelay(BeQt::Second / 2);
    mclient->setWaitForFinishedDelay(BeQt::Second / 2);
    mclient->setWaitForConnectedTimeout(10 * BeQt::Second);
    mclient->setWaitForFinishedTimeout(30 * BeQt::Second);
    mclient->setPingInterval(5 * BeQt::Minute);
    connect(mclient, SIGNAL(authorizedChanged(bool)), this, SLOT(clientAuthorizedChanged(bool)));
    Settings::Texsample::loadPassword();
    updateCacheSettings();
    mgroupModel = new TGroupModel;
    minviteModel = new TInviteModel;
    msampleModel = new SampleModel;
    muserModel = new UserModel(texsampleLocation);
    updateClientSettings();
    if (mcache->isEnabled()) {
        mgroupModel->update(mcache->groupInfoList(), mcache->lastRequestDateTime(Cache::GroupListRequest));
        minviteModel->update(mcache->inviteInfoList(), mcache->lastRequestDateTime(Cache::InviteListRequest));
        msampleModel->update(mcache->sampleInfoList(), mcache->lastRequestDateTime(Cache::SampleListRequest));
        muserModel->update(mcache->userInfoList(), mcache->lastRequestDateTime(Cache::UserListRequest));
    }
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
    BDirTools::rmdir(QDir::tempPath() + "/tex-creator/previews");
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

void TexsampleCore::updateCacheSettings()
{
    mcache->setEnabled(Settings::Texsample::cachingEnabled());
    mclient->setCachingEnabled(mcache->isEnabled());
}

void TexsampleCore::updateClientSettings()
{
    if (mclient->hostName() != Settings::Texsample::host(true)) {
        mgroupModel->clear();
        minviteModel->clear();
        msampleModel->clear();
        muserModel->clear();
        if (!mclient->hostName().isEmpty())
            mcache->clear();
    }
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
    if (!sampleId)
        return false;
    if (!mclient->isAuthorized())
        return false;
    if (!parent)
        parent = bApp->mostSuitableWindow();
    QMessageBox msg(parent);
    msg.setWindowTitle(tr("Deleting sample", "msgbox windowTitle"));
    msg.setIcon(QMessageBox::Question);
    msg.setText(tr("You are going to delete a sample. Do you want to continue?", "msgbox text"));
    msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msg.setDefaultButton(QMessageBox::Ok);
    if (msg.exec() != QMessageBox::Ok)
        return true;
    TDeleteSampleRequestData requestData;
    requestData.setId(sampleId);
    TReply reply = mclient->performOperation(TOperation::DeleteSample, requestData, parent);
    if (!reply.success()) {
        QMessageBox msg(parent);
        msg.setWindowTitle(tr("Deleting sample error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to delete sample due to the following error:", "msgbox text"));
        msg.setInformativeText(reply.message());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return true;
    }
    msampleModel->removeSample(sampleId);
    mcache->removeData(TOperation::DeleteSample, sampleId);
    bApp->showStatusBarMessage(tr("Sample was successfully deleted", "message"));
    return true;
}

void TexsampleCore::disconnectFromServer()
{
    mclient->disconnectFromServer();
}

void TexsampleCore::editSample(quint64 sampleId, BCodeEditor *editor)
{
    if (!sampleId)
        return;
    if (!editor)
        return;
    if (meditSampleDialogs.contains(sampleId)) {
        QPointer<QWidget> wgt = meditSampleDialogs.value(sampleId);
        if (!wgt.isNull())
            return wgt->activateWindow();
        meditSampleDialogs.remove(sampleId);
    }
    if (msampleInfoDialogs.contains(sampleId)) {
        QPointer<QWidget> wgt = msampleInfoDialogs.value(sampleId);
        if (!wgt.isNull())
            return wgt->activateWindow();
        else
            msampleInfoDialogs.remove(sampleId);
    }
    if (!mclient->isAuthorized())
        return;
    SampleInfoWidget::Mode mode = (int(mclient->userInfo().accessLevel()) >= TAccessLevel::ModeratorLevel) ?
                SampleInfoWidget::EditAdminMode : SampleInfoWidget::EditSelfMode;
    if (SampleInfoWidget::EditSelfMode == mode
            && msampleModel->sampleInfo(sampleId).senderId() != mclient->userInfo().id()) {
        return;
    }
    BDialog *dlg = new BDialog;
    dlg->setProperty("sample_id", sampleId);
    SampleInfoWidget *swgt = new SampleInfoWidget(mode);
    swgt->setModel(msampleModel);
    swgt->setClient(mclient);
    swgt->setCache(mcache);
    swgt->setEditor(editor);
    if (!swgt->setSample(sampleId))
        return dlg->deleteLater();
    BTranslation t = BTranslation::translate("TexsampleCore", "Editing sample: %1", "wgt windowTitle");
    t.setArgument(swgt->title());
    new BDynamicTranslator(dlg, "windowTitle", t);
    dlg->setWidget(swgt);
    dlg->addButton(QDialogButtonBox::Cancel, SLOT(reject()));
    QPushButton *btnAccept = dlg->addButton(QDialogButtonBox::Ok, SLOT(accept()));
    btnAccept->setEnabled(swgt->hasValidInput());
    connect(swgt, SIGNAL(inputValidityChanged(bool)), btnAccept, SLOT(setEnabled(bool)));
    dlg->restoreGeometry(Settings::TexsampleCore::sendSampleDialogGeometry());
    connect(dlg, SIGNAL(finished(int)), this, SLOT(editSampleDialogFinished(int)));
    meditSampleDialogs.insert(sampleId, dlg);
    dlg->show();
}

bool TexsampleCore::insertSample(quint64 sampleId, BCodeEditor *editor)
{
    if (!sampleId)
        return false;
    if (!editor)
        return false;
    QWidget *parent = editor->parentWidget();
    if (!parent)
        parent = bApp->mostSuitableWindow();
    BAbstractCodeEditorDocument *doc = editor->currentDocument();
    if (!doc) {
        doc = editor->addDocument();
        if (!editor->saveCurrentDocumentAs() || !editor->waitForAllDocumentsProcessed())
            return false;
        if (!QFileInfo(doc->fileName()).isFile())
            return false;
    }
    QString path = QFileInfo(doc->fileName()).path() + "/texsample-" + QString::number(sampleId);
    if (QFileInfo(path).isFile())
        return false;
    if (QFileInfo(path).isDir()) {
        QMessageBox msg(parent);
        msg.setWindowTitle(tr("Existing sample", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Question);
        msg.setText(tr("It seems like there is some sample in the selected directory", "msgbox text"));
        msg.setInformativeText(tr("Do you want to update it, or use the existing one?", "magbox informativeText"));
        msg.addButton(tr("Update", "btn text"), QMessageBox::AcceptRole);
        QPushButton *btnExisting = msg.addButton(tr("Use existing", "btn text"), QMessageBox::AcceptRole);
        msg.setDefaultButton(btnExisting);
        msg.addButton(QMessageBox::Cancel);
        if (msg.exec() == QMessageBox::Cancel) {
            !BDirTools::rmdir(path);
            return false;
        }
        if (msg.clickedButton() == btnExisting)
            return true;
        if (!BDirTools::rmdir(path))
            return false;
    }
    if (!BDirTools::mkpath(path))
        return false;
    BFileDialog dlg(path, parent);
    dlg.setCodecSelectionEnabled(false);
    dlg.selectFile(path);
    dlg.setLineFeedSelectionEnabled(false);
    QByteArray geometry = Settings::TexsampleCore::selectSampleSubdirDialogGeometry();
    if (!geometry.isEmpty())
        dlg.restoreGeometry(geometry);
    else
        dlg.resize(700, 400);
    QByteArray state = Settings::TexsampleCore::selectSampleSubdirDialogState();
    if (!state.isEmpty())
        dlg.restoreState(state);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::Directory);
    bool b = (dlg.exec() == BFileDialog::Accepted);
    Settings::TexsampleCore::setSelectSampleSubdirDialogGeometry(dlg.saveGeometry());
    Settings::TexsampleCore::setSelectSampleSubdirDialogState(dlg.saveState());
    if (!b) {
        BDirTools::rmdir(path);
        return false;
    }
    QStringList files = dlg.selectedFiles();
    if (files.size() != 1) {
        BDirTools::rmdir(path);
        return false;
    }
    TTexProject source;
    if (!getSampleSource(sampleId, source, parent)) {
        BDirTools::rmdir(path);
        return false;
    }
    if (!source.save(path, doc->codec())) {
        BDirTools::rmdir(path);
        return false;
    }
    QString fn = QFileInfo(path).fileName() + "/" + source.rootFile().fileName();
    doc->insertText("\\input " + BTextTools::wrapped(fn, "\""));
    bApp->showStatusBarMessage(tr("Sample was successfully inserted", "message"));
    return true;
}

bool TexsampleCore::saveSample(quint64 sampleId, QWidget *parent)
{
    if (!sampleId)
        return false;
    if (!parent)
        parent = bApp->mostSuitableWindow();
    QString caption = tr("Select directory", "fdlg caption");
    QString path = Settings::TexsampleCore::saveSampleDir();
    if (path.isEmpty())
        path = QDir::homePath();
    path = QFileDialog::getExistingDirectory(parent, caption, path);
    if (path.isEmpty())
        return false;
    TTexProject source;
    if (!getSampleSource(sampleId, source, parent))
        return false;
    if (!source.save(path, Settings::CodeEditor::defaultCodec())) {
        QMessageBox msg(parent);
        msg.setWindowTitle( tr("Failed to save sample", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("File system error occured", "msgbox text"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return false;
    }
    bApp->showStatusBarMessage(tr("Sample was successfully saved", "message"));
    return true;
}

void TexsampleCore::sendSample(BCodeEditor *editor)
{
    if (!msendSampleDialog.isNull())
        return msendSampleDialog->activateWindow();
    if (!mclient->isAuthorized())
        return;
    BDialog *dlg = new BDialog;
    BTranslation t = BTranslation::translate("TexsampleCore", "Sending sample", "wgt windowTitle");
    new BDynamicTranslator(dlg, "windowTitle", t);
    SampleInfoWidget *swgt = new SampleInfoWidget(SampleInfoWidget::AddMode);
    swgt->setClient(mclient);
    swgt->setCache(mcache);
    swgt->setEditor(editor);
    swgt->restoreState(Settings::TexsampleCore::sendSampleWidgetState());
    dlg->setWidget(swgt);
    dlg->addButton(QDialogButtonBox::Cancel, SLOT(reject()));
    QPushButton *btnAccept = dlg->addButton(QDialogButtonBox::Ok, SLOT(accept()));
    btnAccept->setEnabled(swgt->hasValidInput());
    connect(swgt, SIGNAL(inputValidityChanged(bool)), btnAccept, SLOT(setEnabled(bool)));
    dlg->restoreGeometry(Settings::TexsampleCore::sendSampleDialogGeometry());
    connect(dlg, SIGNAL(finished(int)), this, SLOT(sendSampleDialogFinished(int)));
    msendSampleDialog = dlg;
    dlg->show();
}

bool TexsampleCore::showAccountManagementDialog(QWidget *parent)
{
    if (!mclient->isAuthorized())
        return false;
    TUserInfoWidget *uwgt = new TUserInfoWidget(TUserInfoWidget::EditSelfMode);
    uwgt->setClient(mclient);
    uwgt->setCache(mcache);
    uwgt->setModel(muserModel);
    quint64 userId = mclient->userInfo().id();
    if (!uwgt->setUser(userId))
        return false;
    if (!parent)
        parent = bApp->mostSuitableWindow();
    BDialog dlg(parent);
    dlg.setWindowTitle(tr("Account management", "dlg windowTitle"));
    dlg.setWidget(uwgt);
    dlg.addButton(QDialogButtonBox::Cancel, SLOT(reject()));
    QPushButton *btnAccept = dlg.addButton(QDialogButtonBox::Ok, SLOT(accept()));
    btnAccept->setEnabled(uwgt->hasValidInput());
    connect(uwgt, SIGNAL(inputValidityChanged(bool)), btnAccept, SLOT(setEnabled(bool)));
    dlg.restoreGeometry(Settings::TexsampleCore::accountManagementDialogGeometry());
    bool b = (dlg.exec() == BDialog::Accepted);
    Settings::TexsampleCore::setAccountManagementDialogGeometry(dlg.saveGeometry());
    if (!b)
        return false;
    TReply reply = mclient->performOperation(TOperation::EditSelf, uwgt->createRequestData(), parent);
    if (!reply.success()) {
        QMessageBox msg(parent);
        msg.setWindowTitle(tr("Editing account error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to edit account due to the following error:", "msgbox text"));
        msg.setInformativeText(reply.message());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return false;
    }
    muserModel->updateUser(userId, reply.data().value<TEditSelfReplyData>().userInfo());
    mcache->setData(TOperation::EditSelf, reply.requestDateTime(), reply.data(), userId);
    return true;
}

bool TexsampleCore::showConfirmEmailChangeDialog(QWidget *parent)
{
    if (!parent)
        parent = bApp->mostSuitableWindow();
    return TUserInfoWidget::showConfirmEmailChangeDialog(mclient, parent);
}

bool TexsampleCore::showConfirmRegistrationDialog(QWidget *parent)
{
    if (!parent)
        parent = bApp->mostSuitableWindow();
    BDialog dlg(parent);
    dlg.setWindowTitle(tr("Registration confirmation", "dlg windowTitle"));
    TConfirmationWidget *cwgt = new TConfirmationWidget;
    dlg.setWidget(cwgt);
    dlg.addButton(QDialogButtonBox::Cancel, SLOT(reject()));
    QPushButton *btnAccept = dlg.addButton(QDialogButtonBox::Ok, SLOT(accept()));
    btnAccept->setEnabled(cwgt->hasValidInput());
    connect(cwgt, SIGNAL(inputValidityChanged(bool)), btnAccept, SLOT(setEnabled(bool)));
    dlg.resize(450, 0);
    while (dlg.exec() == BDialog::Accepted) {
        TConfirmRegistrationRequestData requestData;
        requestData.setConfirmationCode(cwgt->code());
        TReply r = mclient->performAnonymousOperation(TOperation::ConfirmRegistration, requestData, parent);
        if (r.success()) {
            bApp->showStatusBarMessage(tr("You have successfully confirmed your account registration", "message"));
            return true;
        } else {
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Registration confirmation error", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to confirm rgistration due to the following error:", "msgbox text"));
            msg.setInformativeText(r.message());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
        }
    }
    return false;
}

void TexsampleCore::showGroupManagementWidget()
{
    if (!mgroupManagementDialog.isNull())
        return mgroupManagementDialog->activateWindow();
    BTranslation t = BTranslation::translate("TexsampleCore", "Group management", "wgt windowTitle");
    Dialog *dlg = new Dialog(&Settings::TexsampleCore::setGroupManagementDialogGeometry, t);
    TGroupWidget *gwgt = new TGroupWidget(mgroupModel);
    gwgt->setCache(mcache);
    gwgt->setClient(mclient);
    dlg->setWidget(gwgt);
    QByteArray geometry = Settings::TexsampleCore::groupManagementDialogGeometry();
    if (!geometry.isEmpty())
        dlg->restoreGeometry(geometry);
    else
        dlg->resize(800, 400);
    mgroupManagementDialog = dlg;
    dlg->show();
}

void TexsampleCore::showInviteManagementWidget()
{
    if (!minviteManagementDialog.isNull())
        return minviteManagementDialog->activateWindow();
    BTranslation t = BTranslation::translate("TexsampleCore", "Invite management", "wgt windowTitle");
    Dialog *dlg = new Dialog(&Settings::TexsampleCore::setInviteManagementDialogGeometry, t);
    TInviteWidget *iwgt = new TInviteWidget(minviteModel);
    iwgt->setCache(mcache);
    iwgt->setClient(mclient);
    dlg->setWidget(iwgt);
    QByteArray geometry = Settings::TexsampleCore::inviteManagementDialogGeometry();
    if (!geometry.isEmpty())
        dlg->restoreGeometry(geometry);
    else
        dlg->resize(800, 400);
    minviteManagementDialog = dlg;
    dlg->show();
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
    if (!parent)
        parent = bApp->mostSuitableWindow();
    BDialog dlg(parent);
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
            bApp->showStatusBarMessage(tr("You have successfully registered", "message"));
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Registration", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Information);
            msg.setText(tr("You have successfully registered. Confirmation code was sent to your e-mail. "
                           "Please, click OK and enter that code to finish registration", "msgbox text"));
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
            if (showConfirmRegistrationDialog(dlg.parentWidget())) {
                Settings::Texsample::setLogin(wgt->login());
                Settings::Texsample::setPassword(wgt->password());
                updateClientSettings();
                if (mclient->isConnected())
                    mclient->reconnect();
                else
                    mclient->connectToServer();
            }
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
    if (!sampleId)
        return;
    if (msampleInfoDialogs.contains(sampleId)) {
        QPointer<QWidget> wgt = msampleInfoDialogs.value(sampleId);
        if (!wgt.isNull())
            return wgt->activateWindow();
        else
            msampleInfoDialogs.remove(sampleId);
    }
    if (meditSampleDialogs.contains(sampleId)) {
        QPointer<QWidget> wgt = meditSampleDialogs.value(sampleId);
        if (!wgt.isNull())
            return wgt->activateWindow();
        meditSampleDialogs.remove(sampleId);
    }
    SampleInfoWidget *swgt = new SampleInfoWidget(SampleInfoWidget::ShowMode);
    swgt->setCache(mcache);
    swgt->setClient(mclient);
    swgt->setModel(msampleModel);
    if (!swgt->setSample(sampleId))
        return;
    BTranslation t = BTranslation::translate("TexsampleCore", "Sample: %1", "wgt windowTitle");
    t.setArgument(swgt->title());
    Dialog *dlg = new Dialog(&Settings::TexsampleCore::setSampleInfoDialogGeometry, t);
    dlg->setWidget(swgt);
    QByteArray geometry = Settings::TexsampleCore::sampleInfoDialogGeometry();
    if (!geometry.isEmpty())
        dlg->restoreGeometry(geometry);
    else
        dlg->resize(800, 400);
    msampleInfoDialogs.insert(sampleId, dlg);
    dlg->show();
}

void TexsampleCore::showSamplePreview(quint64 sampleId)
{
    if (!sampleId)
        return;
    if (!mclient->isAuthorized())
        return;
    TGetSamplePreviewRequestData requestData;
    requestData.setId(sampleId);
    QWidget *parent = bApp->mostSuitableWindow();
    TReply reply = mclient->performOperation(TOperation::GetSamplePreview, requestData,
                                             mcache->lastRequestDateTime(Cache::SamplePreviewRequest, sampleId),
                                             parent);
    if (!reply.success()) {
        QMessageBox msg(parent);
        msg.setWindowTitle(tr("Getting sample preview error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to get sample preview due to the following error:", "msgbox text"));
        msg.setInformativeText(reply.message());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    TGetSamplePreviewReplyData replyData = reply.data().value<TGetSamplePreviewReplyData>();
    if (reply.cacheUpToDate())
        replyData = mcache->data(TOperation::GetSamplePreview, sampleId).value<TGetSamplePreviewReplyData>();
    else
        mcache->setData(TOperation::GetSamplePreview, reply.requestDateTime(), replyData, sampleId);
    QString path = QDir::tempPath() + "/tex-creator/previews/" + BUuid::createUuid().toString(true);
    if (!saveSamplePreview(path, replyData.mainFile(), replyData.extraFiles()))
        return;
    bApp->openLocalFile(path + "/" + replyData.mainFile().fileName());
}

bool TexsampleCore::showTexsampleSettings(QWidget *parent)
{
    return bApp->showSettings(Application::TexsampleSettings, parent);
}

void TexsampleCore::showUserInfo(quint64 userId)
{
    if (!userId)
        return;
    if (muserInfoDialogs.contains(userId)) {
        QPointer<QWidget> wgt = muserInfoDialogs.value(userId);
        if (!wgt.isNull())
            return wgt->activateWindow();
        else
            muserInfoDialogs.remove(userId);
    }
    TUserInfoWidget *uwgt = new TUserInfoWidget(TUserInfoWidget::ShowMode);
    uwgt->setCache(mcache);
    uwgt->setClient(mclient);
    uwgt->setModel(muserModel);
    if (!uwgt->setUser(userId))
        return;
    BTranslation t = BTranslation::translate("TexsampleCore", "User: %1", "wgt windowTitle");
    t.setArgument(uwgt->login());
    Dialog *dlg = new Dialog(&Settings::TexsampleCore::setUserInfoDialogGeometry, t);
    dlg->setWidget(uwgt);
    QByteArray geometry = Settings::TexsampleCore::userInfoDialogGeometry();
    if (!geometry.isEmpty())
        dlg->restoreGeometry(geometry);
    else
        dlg->resize(800, 400);
    muserInfoDialogs.insert(userId, dlg);
    dlg->show();
}

void TexsampleCore::showUserManagementWidget()
{
    if (!muserManagementDialog.isNull())
        return muserManagementDialog->activateWindow();
    BTranslation t = BTranslation::translate("TexsampleCore", "User management", "wgt windowTitle");
    Dialog *dlg = new Dialog(&Settings::TexsampleCore::setUserManagementDialogGeometry, t);
    TUserWidget *uwgt = new TUserWidget(muserModel);
    uwgt->setCache(mcache);
    uwgt->setClient(mclient);
    dlg->setWidget(uwgt);
    QByteArray geometry = Settings::TexsampleCore::userManagementDialogGeometry();
    if (!geometry.isEmpty())
        dlg->restoreGeometry(geometry);
    else
        dlg->resize(800, 400);
    muserManagementDialog = dlg;
    dlg->show();
}

void TexsampleCore::updateSampleList()
{
    if (!mclient->isAuthorized())
        return;
    TGetSampleInfoListRequestData requestData;
    TReply reply = mclient->performOperation(TOperation::GetSampleInfoList, requestData,
                                             msampleModel->lastUpdateDateTime(), bApp->mostSuitableWindow());
    if (!reply.success()) {
        QMessageBox msg(bApp->mostSuitableWindow());
        msg.setWindowTitle(tr("Updating sample list error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to update sample list due to the following error:", "msgbox text"));
        msg.setInformativeText(reply.message());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    TGetSampleInfoListReplyData replyData = reply.data().value<TGetSampleInfoListReplyData>();
    msampleModel->update(replyData.newSamples(), replyData.deletedSamples(), reply.requestDateTime());
    mcache->setData(TOperation::GetSampleInfoList, reply.requestDateTime(), replyData);
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

bool TexsampleCore::saveSamplePreview(const QString &path, const TBinaryFile &mainFile,
                                      const TBinaryFileList &extraFiles)
{
    if (path.isEmpty() || !mainFile.isValid())
        return false;
    if (!mainFile.save(path)) {
        BDirTools::rmdir(path);
        return false;
    }
    foreach (const TBinaryFile &file, extraFiles) {
        if (!file.save(path)) {
            BDirTools::rmdir(path);
            return false;
        }
    }
    return true;
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
    dlg.setAutoCloseInterval(0);
    if (timeout > 0)
        QTimer::singleShot(timeout, op, SLOT(cancel()));
    dlg.exec();
    if (op->isCancelled())
        return bRet(msg, tr("Operation cancelled by user", "error"), false);
    else if (op->isError())
        return bRet(msg, tr("An error occured during operation", "error"), false);
    else if (op->isFinished())
        return bRet(msg, QString(), true);
    else
        return bRet(msg, tr("Operation timed out", "error"), false);
}

/*============================== Private methods ===========================*/

bool TexsampleCore::getSampleSource(quint64 sampleId, TTexProject &source, QWidget *parent)
{
    if (!sampleId)
        return false;
    if (!mclient->isAuthorized())
        return false;
    if (!parent)
        parent = bApp->mostSuitableWindow();
    TGetSampleSourceRequestData requestData;
    requestData.setId(sampleId);
    TReply reply = mclient->performOperation(TOperation::GetSampleSource, requestData,
                                             mcache->lastRequestDateTime(Cache::SampleSourceRequest, sampleId),
                                             parent);
    if (!reply.success()) {
        QMessageBox msg(bApp->mostSuitableWindow());
        msg.setWindowTitle(tr("Getting sample source error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to get sample source due to the following error:", "msgbox text"));
        msg.setInformativeText(reply.message());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return false;
    }
    if (!reply.cacheUpToDate()) {
        mcache->setData(TOperation::GetSampleSource, reply.requestDateTime(), reply.data(), sampleId);
        source = reply.data().value<TGetSampleSourceReplyData>().project();
    } else {
        source = mcache->data(TOperation::GetSampleSource, sampleId).value<TGetSampleSourceReplyData>().project();
    }
    return true;
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

void TexsampleCore::clientAuthorizedChanged(bool authorized)
{
    if (!authorized)
        return;
    updateSampleList();
}

void TexsampleCore::editSampleDialogFinished(int result)
{
    BDialog *dlg = qobject_cast<BDialog *>(sender());
    if (!dlg)
        return;
    SampleInfoWidget *swgt = qobject_cast<SampleInfoWidget *>(dlg->widget());
    if (!swgt)
        return;
    bool b = (BDialog::Accepted == result);
    Settings::TexsampleCore::setSendSampleDialogGeometry(dlg->saveGeometry());
    if (!b)
        return dlg->deleteLater();
    if (!mclient->isAuthorized())
        return dlg->deleteLater();
    quint64 sampleId = dlg->property("sample_id").toULongLong();
    if (!sampleId)
        return dlg->deleteLater();
    QWidget *parent = bApp->mostSuitableWindow();
    bool admin = (int(mclient->userInfo().accessLevel()) >= TAccessLevel::ModeratorLevel);
    QString op = admin ? TOperation::EditSampleAdmin : TOperation::EditSample;
    TReply reply = mclient->performOperation(op, swgt->createRequestData(), 5 * BeQt::Minute, parent);
    if (!reply.success()) {
        QMessageBox msg(parent);
        msg.setWindowTitle(tr("Editing sample error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to edit sample due to the following error:", "msgbox text"));
        msg.setInformativeText(reply.message());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        dlg->deleteLater();
        return;
    }
    if (admin)
        msampleModel->updateSample(sampleId, reply.data().value<TEditSampleAdminReplyData>().sampleInfo());
    else
        msampleModel->updateSample(sampleId, reply.data().value<TEditSampleReplyData>().sampleInfo());
    mcache->setData(op, reply.requestDateTime(), reply.data());
    bApp->showStatusBarMessage(tr("Sample was successfully edited", "message"));
    dlg->deleteLater();
}

void TexsampleCore::sendSampleDialogFinished(int result)
{
    BDialog *dlg = qobject_cast<BDialog *>(sender());
    if (!dlg)
        return;
    SampleInfoWidget *swgt = qobject_cast<SampleInfoWidget *>(dlg->widget());
    if (!swgt)
        return;
    bool b = (BDialog::Accepted == result);
    Settings::TexsampleCore::setSendSampleDialogGeometry(dlg->saveGeometry());
    Settings::TexsampleCore::setSendSampleWidgetState(swgt->saveState());
    if (!b)
        return dlg->deleteLater();
    QWidget *parent = bApp->mostSuitableWindow();
    TReply reply = mclient->performOperation(TOperation::AddSample, swgt->createRequestData(), 5 * BeQt::Minute,
                                             parent);
    if (!reply.success()) {
        QMessageBox msg(parent);
        msg.setWindowTitle(tr("Sending sample error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to send sample due to the following error:", "msgbox text"));
        msg.setInformativeText(reply.message());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        dlg->deleteLater();
        return;
    }
    TAddSampleReplyData replyData = reply.data().value<TAddSampleReplyData>();
    msampleModel->addSample(replyData.sampleInfo());
    mcache->setData(TOperation::AddSample, reply.requestDateTime(), replyData);
    bApp->showStatusBarMessage(tr("Sample was successfully sent", "message"));
    dlg->deleteLater();
}
