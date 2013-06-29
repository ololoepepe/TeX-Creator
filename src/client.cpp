#include "client.h"
#include "application.h"
#include "texsamplesettingstab.h"
#include "samplesmodel.h"
#include "requestprogressdialog.h"
#include "cache.h"
#include "global.h"

#include <TUserInfo>
#include <TSampleInfo>
#include <TProject>
#include <TCompilerParameters>
#include <TCompilationResult>
#include <TCompiledProject>
#include <TInviteInfo>
#include <TOperationResult>
#include <TClientInfo>
#include <TeXSample>
#include <TAccessLevel>

#include <BNetworkConnection>
#include <BGenericSocket>
#include <BPasswordWidget>
#include <BeQtGlobal>
#include <BNetworkOperation>
#include <BNetworkOperationMetaData>
#include <BDirTools>
#include <BCodeEditor>
#include <BCodeEditorDocument>

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QAbstractSocket>
#include <QSettings>
#include <QVariant>
#include <QApplication>
#include <QMessageBox>
#include <QWidget>
#include <QVariantMap>
#include <QVariantList>
#include <QDateTime>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QTextCodec>
#include <QPushButton>
#include <QImage>
#include <QBuffer>
#include <QUuid>
#include <QRegExp>
#include <QProgressDialog>
#include <QTimer>

#include <QDebug>

/*============================================================================
================================ Client ======================================
============================================================================*/

/*============================== Static public methods =====================*/

Client *Client::instance()
{
    if (!minstance)
        minstance = new Client;
    return minstance;
}

TOperationResult Client::registerUser(const TUserInfo &info, const QString &invite, QWidget *parent)
{
    if (!info.isValid(TUserInfo::RegisterContext) || BeQt::uuidFromText(invite).isNull())
        return TOperationResult(invalidParametersString());
    BNetworkConnection c(BGenericSocket::TcpSocket);
    QString host = Global::host();
    c.connectToHost(host.compare("auto_select") ? host : QString("texsample-server.no-ip.org"),
                    Texsample::RegistrationPort);
    parent = chooseParent(parent);
    if (!c.isConnected() && !c.waitForConnected(BeQt::Second / 2))
    {
        QProgressDialog pd(parent);
        pd.setWindowTitle(tr("Connecting to server", "pdlg windowTitle"));
        pd.setLabelText(tr("Connecting to server, please, wait...", "pdlg labelText"));
        pd.setMinimum(0);
        pd.setMaximum(0);
        QTimer::singleShot(10 * BeQt::Second, &pd, SLOT(close()));
        if (pd.exec() == QProgressDialog::Rejected)
        {
            c.close();
            return TOperationResult(operationErrorString());
        }
    }
    if (!c.isConnected())
    {
        c.close();
        return TOperationResult(tr("Failed to connect to server", "errorString"));
    }
    QVariantMap out;
    out.insert("user_info", info);
    out.insert("invite", invite);
    BNetworkOperation *op = c.sendRequest(Texsample::RegisterRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, parent).exec();
    c.close();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
    return in.value("operation_result").value<TOperationResult>();
}

/*============================== Public constructors =======================*/

Client::Client(QObject *parent) :
    QObject(parent)
{
    mstate = DisconnectedState;
    mreconnect = false;
    mconnection = new BNetworkConnection(BGenericSocket::TcpSocket, this);
    connect(mconnection, SIGNAL(connected()), this, SLOT(connected()));
    connect(mconnection, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(mconnection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(error(QAbstractSocket::SocketError)));
    connect(mconnection, SIGNAL(requestReceived(BNetworkOperation *)),
            this, SLOT(remoteRequest(BNetworkOperation *)));
    mhost = Global::host();
    mlogin = Global::login();
    mpassword = Global::password();
    if (Global::cachingEnabled())
        sCache->open();
    mid = 0;
}

Client::~Client()
{
    delete sModel;
    delete sCache;
}

/*============================== Public methods ============================*/

bool Client::updateSettings()
{
    QString login = Global::login();
    QByteArray password = Global::password();
    QString host = Global::host();
    bool b = false;
    if (host != mhost || login != mlogin || password != mpassword)
    {
        bool bcc = canConnect();
        if (host != mhost)
        {
            sModel->clear();
            sCache->clear();
            msamplesListUpdateDT = QDateTime().toUTC();
            mhost = host;
            emit hostChanged(host);
        }
        if (login != mlogin)
        {
            mlogin = login;
            emit loginChanged(login);
        }
        mpassword = password;
        bool bccn = canConnect();
        if (bcc != bccn)
            emit canConnectChanged(bccn);
        if (ConnectingState == mstate || ConnectedState == mstate || AuthorizedState == mstate)
            reconnect();
        b = true;
    }
    if (Global::cachingEnabled())
        sCache->open();
    else
        sCache->close();
    return b;
}

void Client::setConnected(bool b)
{
   if (b)
       connectToServer();
   else
       disconnectFromServer();
}

Client::State Client::state() const
{
    return mstate;
}

bool Client::canConnect() const
{
    return (DisconnectedState == mstate && !mhost.isEmpty() && !mlogin.isEmpty() && !mpassword.isEmpty());
}

bool Client::canDisconnect() const
{
    return (ConnectingState == mstate || ConnectedState == mstate || AuthorizedState == mstate);
}

bool Client::isAuthorized() const
{
    return (AuthorizedState == mstate);
}

QString Client::login() const
{
    return mlogin;
}

TAccessLevel Client::accessLevel() const
{
    return maccessLevel;
}

quint64 Client::userId() const
{
    return mid;
}

TOperationResult Client::addUser(const TUserInfo &info, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    if (!info.isValid(TUserInfo::AddContext))
        return TOperationResult(invalidParametersString());
    QVariantMap out;
    out.insert("user_info", info);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::AddUserRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::editUser(const TUserInfo &info, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    if (!info.isValid(TUserInfo::EditContext))
        return TOperationResult(invalidParametersString());
    QVariantMap out;
    out.insert("user_info", info);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::EditUserRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::updateAccount(TUserInfo info, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    info.setId(mid);
    if (!info.isValid(TUserInfo::UpdateContext))
        return TOperationResult(invalidParametersString());
    QVariantMap out;
    out.insert("user_info", info);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::UpdateAccountRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::getUserInfo(quint64 id, TUserInfo &info, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    if (!id)
        return TOperationResult(invalidParametersString());
    QVariantMap out;
    out.insert("user_id", id);
    out.insert("update_dt", sCache->userInfoUpdateDateTime(id));
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetUserInfoRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
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

TCompilationResult Client::addSample(const QString &fileName, QTextCodec *codec, const TSampleInfo &info,
                                     QWidget *parent)
{
    return addSample(fileName, codec, BDirTools::readTextFile(fileName, codec), info, parent);
}

TCompilationResult Client::addSample(const QString &fileName, QTextCodec *codec, const QString &text,
                                     const TSampleInfo &info, QWidget *parent)
{
    if (!isAuthorized())
        return TCompilationResult(notAuthorizedString());
    if (fileName.isEmpty() || text.isEmpty() || !info.isValid(TSampleInfo::AddContext))
        return TCompilationResult(invalidParametersString());
    TProject p(fileName, text, codec);
    if (!p.isValid())
        return TCompilationResult(tr("Failed to pack sample", "errorString"));
    p.removeRestrictedCommands();
    QVariantMap out;
    out.insert("project", p);
    out.insert("sample_info", info);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::AddSampleRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TCompilationResult(operationErrorString());
    TCompilationResult r = in.value("compilation_result").value<TCompilationResult>();
    if (r)
        updateSamplesList();
    return r;
}

TCompilationResult Client::editSample(const TSampleInfo &newInfo, QWidget *parent)
{
    return editSample(newInfo, QString(), 0, parent);
}

TCompilationResult Client::editSample(const TSampleInfo &newInfo, const QString &fileName, QTextCodec *codec,
                                      QWidget *parent)
{
    return editSample(newInfo, fileName, codec, BDirTools::readTextFile(fileName, codec), parent);
}

TCompilationResult Client::editSample(const TSampleInfo &newInfo, const QString &fileName, QTextCodec *codec,
                                      const QString &text, QWidget *parent)
{
    if (!isAuthorized())
        return TCompilationResult(notAuthorizedString());
    if (!newInfo.isValid(TSampleInfo::EditContext))
        return TCompilationResult(invalidParametersString());
    QVariantMap out;
    out.insert("sample_info", newInfo);
    if (!fileName.isEmpty() && codec && !text.isEmpty())
    {
        TProject p(fileName, text, codec);
        if (!p.isValid())
            return TCompilationResult(tr("Failed to pack sample", "errorString"));
        p.removeRestrictedCommands();
        out.insert("project", p);
    }
    BNetworkOperation *op = mconnection->sendRequest(Texsample::EditSampleRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TCompilationResult(operationErrorString());
    TCompilationResult r = in.value("compilation_result").value<TCompilationResult>();
    if (r)
        updateSamplesList();
    return r;
}

TCompilationResult Client::updateSample(const TSampleInfo &newInfo, QWidget *parent)
{
    return updateSample(newInfo, QString(), 0, parent);
}

TCompilationResult Client::updateSample(const TSampleInfo &newInfo, const QString &fileName, QTextCodec *codec,
                                        QWidget *parent)
{
    return updateSample(newInfo, fileName, codec, BDirTools::readTextFile(fileName, codec), parent);
}

TCompilationResult Client::updateSample(const TSampleInfo &newInfo, const QString &fileName, QTextCodec *codec,
                                        const QString &text, QWidget *parent)
{
    if (!isAuthorized())
        return TCompilationResult(notAuthorizedString());
    if (!newInfo.isValid(TSampleInfo::UpdateContext))
        return TCompilationResult(invalidParametersString());
    QVariantMap out;
    out.insert("sample_info", newInfo);
    if (!fileName.isEmpty() && codec && !text.isEmpty())
    {
        TProject p(fileName, text, codec);
        if (!p.isValid())
            return TCompilationResult(tr("Failed to pack sample", "errorString"));
        p.removeRestrictedCommands();
        out.insert("project", p);
    }
    BNetworkOperation *op = mconnection->sendRequest(Texsample::UpdateSampleRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TCompilationResult(operationErrorString());
    TCompilationResult r = in.value("compilation_result").value<TCompilationResult>();
    if (r)
        updateSamplesList();
    return r;
}

TOperationResult Client::deleteSample(quint64 id, const QString &reason, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    if (!id)
        return TOperationResult(invalidParametersString());
    QVariantMap out;
    out.insert("sample_id", id);
    out.insert("reason", reason);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::DeleteSampleRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
    TOperationResult r = in.value("operation_result").value<TOperationResult>();
    if (r)
        updateSamplesList();
    return r;
}

TOperationResult Client::updateSamplesList(bool full, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    QVariantMap out;
    out.insert("update_dt", !full ? sampleInfosUpdateDateTime() : QDateTime());
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetSamplesListRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
    updateSampleInfos(in.value("new_sample_infos").value<TSampleInfo::SamplesList>(),
                      in.value("deleted_sample_infos").value<Texsample::IdList>(), in.value("update_dt").toDateTime());
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::insertSample(quint64 id, BCodeEditorDocument *doc, const QString &subdir)
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    if (!id || !doc || subdir.isEmpty() || subdir.contains(QRegExp("\\s")))
        return TOperationResult(invalidParametersString());
    QFileInfo fi(doc->fileName());
    if (!fi.exists() || !fi.isFile())
        return TOperationResult(tr("The document is not saved", "errorString"));
    QString path = fi.path() + "/" + subdir;
    if ((QFileInfo(path).isDir() && !BDirTools::rmdir(path)) || !BDirTools::mkpath(path))
        return TOperationResult(tr("Failed to prepare directory", "errorString"));
    QVariantMap out;
    out.insert("sample_id", id);
    out.insert("update_dt", sCache->sampleSourceUpdateDateTime(id));
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetSampleSourceRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(doc->editor())).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
    TOperationResult r = in.value("operation_result").value<TOperationResult>();
    if (!r)
        return r;
    TProject p = (in.value("cache_ok").toBool() && sCache->isValid()) ? sCache->sampleSource(id) :
                                                                        in.value("project").value<TProject>();
    sCache->cacheSampleSource(id, in.value("update_dt").toDateTime(), in.value("project").value<TProject>());
    r.setSuccess(p.prependExternalFileNames(subdir) && p.save(path, doc->codec()));
    if (!r)
        r.setErrorString(tr("Failed to save project", "errorString"));
    else
        doc->insertText("\\input " + BeQt::wrapped(QFileInfo(path).fileName() + "/" + p.rootFileName()));
    return r;
}

TOperationResult Client::previewSample(quint64 id, QWidget *parent, bool) //"bool full" will be used in later versions
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    if (!id)
        return TOperationResult(invalidParametersString());
    QVariantMap out;
    out.insert("sample_id", id);
    out.insert("update_dt", sCache->samplePreviewUpdateDateTime(id));
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetSamplePreviewRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
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
        r.setErrorString(tr("Failed to save or open preview", "errorString"));
    return r;
}

TOperationResult Client::generateInvites(TInviteInfo::InvitesList &invites, const QDateTime &expiresDT, quint8 count,
                                         QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    if (!count || count > Texsample::MaximumInvitesCount)
        return TOperationResult(invalidParametersString());
    QVariantMap out;
    out.insert("expiration_dt", expiresDT);
    out.insert("count", count ? count : 1);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GenerateInvitesRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
    invites = in.value("invite_infos").value<TInviteInfo::InvitesList>();
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::getInvitesList(TInviteInfo::InvitesList &list, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(notAuthorizedString());
    BNetworkOperation *op = mconnection->sendRequest(Texsample::GetInvitesListRequest);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(operationErrorString());
    list = in.value("invite_infos").value<TInviteInfo::InvitesList>();
    return in.value("operation_result").value<TOperationResult>();
}

TCompilationResult Client::compile(const QString &fileName, QTextCodec *codec, const TCompilerParameters &param,
                                   TCompilationResult &makeindexResult, TCompilationResult &dvipsResult,
                                   QWidget *parent)
{
    if (!isAuthorized())
        return TCompilationResult(notAuthorizedString());
    if (fileName.isEmpty())
        return TCompilationResult(invalidParametersString());
    TProject p(fileName, codec);
    if (!p.isValid())
        return TCompilationResult(tr("Failed to load project", "errorString"));
    QVariantMap out;
    out.insert("project", p);
    out.insert("parameters", param);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::CompileProjectRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent(parent)).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TCompilationResult(operationErrorString());
    TCompilationResult r = in.value("compilation_result").value<TCompilationResult>();
    if (!r)
        return r;
    r.setSuccess(in.value("compiled_project").value<TCompiledProject>().save(QFileInfo(fileName).path()));
    if (!r)
        r.setErrorString(tr("Failed to save compiled project", "errorString"));
    makeindexResult = in.value("makeindex_result").value<TCompilationResult>();
    dvipsResult = in.value("dvips_result").value<TCompilationResult>();
    return r;
}

/*============================== Public slots ==============================*/

void Client::connectToServer()
{
    if (!canConnect() || (Global::password().isEmpty() && !Application::showPasswordDialog()))
        return;
    if (Global::password().isEmpty())
    {
        QMessageBox msg( Application::mostSuitableWindow() );
        msg.setWindowTitle( tr("No password", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Information);
        msg.setText( tr("You have not specified your password", "msgbox text") );
        msg.setInformativeText( tr("You can't connect without password", "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    setState(ConnectingState);
    mconnection->connectToHost(mhost.compare("auto_select") ? mhost : QString("texsample-server.no-ip.org"),
                               Texsample::MainPort);
}

void Client::reconnect()
{
    if (DisconnectedState == mstate || DisconnectingState == mstate)
        return;
    mreconnect = true;
    disconnectFromServer();
}

void Client::disconnectFromServer()
{
    if (!canDisconnect())
        return;
    if (ConnectingState == mstate)
    {
        mconnection->abort();
        setState(DisconnectedState);
    }
    else
    {
        setState(DisconnectingState);
        mconnection->disconnectFromHost();
    }
}

/*============================== Static private methods ====================*/

QWidget *Client::chooseParent(QWidget *supposed)
{
    return supposed ? supposed : Application::mostSuitableWindow();
}

QString Client::notAuthorizedString()
{
    return tr("Not authorized", "errorString");
}

QString Client::invalidParametersString()
{
    return tr("Invalid parameters", "errorString");
}

QString Client::operationErrorString()
{
    return tr("Operation failed due to connection error", "errorString");
}

void Client::showConnectionErrorMessage(const QString &errorString)
{
    QMessageBox msg(Application::mostSuitableWindow());
    msg.setWindowTitle(tr("TeXSample connection error", "msgbox windowTitle"));
    msg.setIcon(QMessageBox::Critical);
    msg.setText(tr("The following connection error occured:", "msgbox text") + "\n" + errorString);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    msg.exec();
}

/*============================== Private methods ===========================*/

void Client::setState(State s, TAccessLevel alvl)
{
    if (s == mstate)
        return;
    bool b = (AuthorizedState == s || AuthorizedState == mstate);
    bool bcc = canConnect();
    bool bcd = canDisconnect();
    mstate = s;
    emit stateChanged(s);
    if (b)
        emit authorizedChanged(AuthorizedState == mstate);
    TAccessLevel palvl = maccessLevel;
    maccessLevel = alvl;
    if (palvl != alvl)
        emit accessLevelChanged(alvl);
    bool bccn = canConnect();
    bool bcdn = canDisconnect();
    if (bcc != bccn)
        emit canConnectChanged(bccn);
    if (bcd != bcdn)
        emit canDisconnectChanged(bcdn);
}

void Client::updateSampleInfos(const TSampleInfo::SamplesList &newInfos, const Texsample::IdList &deletedInfos,
                               const QDateTime &updateDT)
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
}

/*============================== Private slots =============================*/

void Client::connected()
{
    setState(ConnectedState);
    QVariantMap out;
    out.insert("login", mlogin);
    out.insert("password", mpassword);
    out.insert("client_info", TClientInfo::createDefaultInfo());
    BNetworkOperation *op = mconnection->sendRequest(Texsample::AuthorizeRequest, out);
    if (!op->waitForFinished(ProgressDialogDelay))
        RequestProgressDialog(op, chooseParent()).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return;
    TOperationResult r = in.value("operation_result").value<TOperationResult>();
    if (r)
    {
        mid = in.value("user_id").toULongLong();
        setState(AuthorizedState, in.value("access_level").value<TAccessLevel>());
        updateSamplesList();
    }
    else
    {
        disconnectFromServer();
        showConnectionErrorMessage(r.errorString());
    }
}

void Client::disconnected()
{
    setState(DisconnectedState);
    mid = 0;
    if (mreconnect)
    {
        mreconnect = false;
        connectToServer();
    }
}

void Client::error(QAbstractSocket::SocketError)
{
    setState(DisconnectedState);
    mid = 0;
    QString errorString = mconnection->errorString();
    if (mconnection->isConnected())
        mconnection->close();
    showConnectionErrorMessage(errorString);
}

void Client::remoteRequest(BNetworkOperation *op)
{
    if (op->metaData().operation() != "noop")
        return op->deleteLater();
    bLogger->logInfo(tr("Replying to connection test...", "log"));
    mconnection->sendReply(op, QByteArray());
    if (!op->isFinished() && !op->isError() && !op->waitForFinished())
    {
        op->deleteLater();
        return bLogger->logCritical(tr("Operation error", "log"));
    }
    op->deleteLater();
}

/*============================== Static private constants ==================*/

const int Client::ProgressDialogDelay = BeQt::Second / 2;
const int Client::MaxSampleSize = 199 * BeQt::Megabyte;

/*============================== Static private members ====================*/

Client *Client::minstance = 0;
