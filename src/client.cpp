#include "client.h"
#include "application.h"
#include "texsamplesettingstab.h"
#include "samplesmodel.h"
#include "cache.h"
#include "global.h"

#include <TUserInfo>
#include <TSampleInfo>
#include <TTexProject>
#include <TCompilerParameters>
#include <TCompilationResult>
#include <TCompiledProject>
#include <TInviteInfo>
#include <TOperationResult>
#include <TClientInfo>
#include <TeXSample>
#include <TAccessLevel>
#include <TProjectFileList>
#include <TServiceList>
#include <TService>
#include <BVersion>

#include <BNetworkConnection>
#include <BGenericSocket>
#include <BPasswordWidget>
#include <BeQtGlobal>
#include <BNetworkOperation>
#include <BNetworkOperationMetaData>
#include <BDirTools>
#include <BCodeEditor>
#include <BAbstractCodeEditorDocument>
#include <BSignalDelayProxy>
#include <BOperationProgressDialog>
#include <BTextTools>

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

B_DECLARE_TRANSLATE_FUNCTION

static bool handleNoopRequest(BNetworkOperation *op)
{
    bLogger->logInfo(translate("Client", "Replying to connection test...", "log"));
    op->reply();
    if (!op->waitForFinished())
        bLogger->logCritical(translate("Client", "Operation error", "log"));
    return true;
}

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

bool Client::hasAccessToService(const TService &s)
{
    return instance()->mservices.contains(s);
}

TOperationResult Client::checkEmail(const QString &email, bool &free, QWidget *parent)
{
    BNetworkConnection c(BGenericSocket::TcpSocket);
    QString host = Global::host();
    c.connectToHost(host.compare("auto_select") ? host : QString("texsample-server.no-ip.org"), Texsample::MainPort);
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
            return TOperationResult(TMessage::ClientOperationCanceledError);
        }
    }
    if (!c.isConnected())
    {
        c.close();
        return TOperationResult(TMessage::ClientConnectionTimeoutError);
    }
    QVariantMap out;
    out.insert("email", email);
    BNetworkOperation *op = c.sendRequest(Texsample::CheckEmailRequest, out);
    showProgressDialog(op, parent);
    c.close();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    free = in.value("free").toBool();
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::checkLogin(const QString &login, bool &free, QWidget *parent)
{
    BNetworkConnection c(BGenericSocket::TcpSocket);
    QString host = Global::host();
    c.connectToHost(host.compare("auto_select") ? host : QString("texsample-server.no-ip.org"), Texsample::MainPort);
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
            return TOperationResult(TMessage::ClientOperationCanceledError);
        }
    }
    if (!c.isConnected())
    {
        c.close();
        return TOperationResult(TMessage::ClientConnectionTimeoutError);
    }
    QVariantMap out;
    out.insert("login", login);
    BNetworkOperation *op = c.sendRequest(Texsample::CheckLoginRequest, out);
    showProgressDialog(op, parent);
    c.close();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    free = in.value("free").toBool();
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::registerUser(const TUserInfo &info, QWidget *parent)
{
    if (!info.isValid(TUserInfo::RegisterContext))
        return TOperationResult(TMessage::InvalidUserInfoError);
    BNetworkConnection c(BGenericSocket::TcpSocket);
    QString host = Global::host();
    c.connectToHost(host.compare("auto_select") ? host : QString("texsample-server.no-ip.org"), Texsample::MainPort);
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
            return TOperationResult(TMessage::ClientOperationCanceledError);
        }
    }
    if (!c.isConnected())
    {
        c.close();
        return TOperationResult(TMessage::ClientConnectionTimeoutError);
    }
    QVariantMap out;
    out.insert("user_info", info);
    out.insert("locale", Application::locale());
    BNetworkOperation *op = c.sendRequest(Texsample::RegisterRequest, out);
    showProgressDialog(op, parent);
    c.close();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::getRecoveryCode(const QString &email, QWidget *parent)
{
    if (email.isEmpty())
        return TOperationResult(TMessage::InvalidEmailError);
    BNetworkConnection c(BGenericSocket::TcpSocket);
    QString host = Global::host();
    c.connectToHost(host.compare("auto_select") ? host : QString("texsample-server.no-ip.org"), Texsample::MainPort);
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
            return TOperationResult(TMessage::ClientOperationCanceledError);
        }
    }
    if (!c.isConnected())
    {
        c.close();
        return TOperationResult(TMessage::ClientConnectionTimeoutError);
    }
    QVariantMap out;
    out.insert("email", email);
    out.insert("locale", Application::locale());
    BNetworkOperation *op = c.sendRequest(Texsample::GetRecoveryCodeRequest, out);
    showProgressDialog(op, parent);
    c.close();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::recoverAccount(const QString &email, const QString &code, const QByteArray &password,
                                        QWidget *parent)
{
    if (email.isEmpty() || BeQt::uuidFromText(code).isNull() || password.isEmpty())
        return TOperationResult(TMessage::InvalidDataError);
    BNetworkConnection c(BGenericSocket::TcpSocket);
    QString host = Global::host();
    c.connectToHost(host.compare("auto_select") ? host : QString("texsample-server.no-ip.org"), Texsample::MainPort);
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
            return TOperationResult(TMessage::ClientOperationCanceledError);
        }
    }
    if (!c.isConnected())
    {
        c.close();
        return TOperationResult(TMessage::ClientConnectionTimeoutError);
    }
    QVariantMap out;
    out.insert("email", email);
    out.insert("recovery_code", code);
    out.insert("password", password);
    out.insert("locale", Application::locale());
    BNetworkOperation *op = c.sendRequest(Texsample::RecoverAccountRequest, out);
    showProgressDialog(op, parent);
    c.close();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    return in.value("operation_result").value<TOperationResult>();
}

Client::CheckForNewVersionsResult Client::checkForNewVersions(bool persistent)
{
    CheckForNewVersionsResult r;
    r.persistent = persistent;
    BNetworkConnection c(BGenericSocket::TcpSocket);
    QString host = Global::host();
    c.connectToHost(host.compare("auto_select") ? host : QString("texsample-server.no-ip.org"), Texsample::MainPort);
    if (!c.isConnected() && !c.waitForConnected(10 * BeQt::Second))
    {
        c.close();
        r.result = TOperationResult(TMessage::ClientConnectionTimeoutError);
        return r;
    }
    QVariantMap out;
    out.insert("client_info", TClientInfo::createInfo());
    BNetworkOperation *op = c.sendRequest(Texsample::GetLatestAppVersionRequest, out);
    if (!op->isFinished() && !op->waitForFinished(10 * BeQt::Second))
    {
        op->cancel();
        r.result = TOperationResult(TMessage::ClientOperationCanceledError);
        return r;
    }
    c.close();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
    {
        r.result = TOperationResult(TMessage::ClientOperationError);
        return r;
    }
    r.version = in.value("version").value<BVersion>();
    r.url = in.value("url").toString();
    r.result = in.value("operation_result").value<TOperationResult>();
    return r;
}

TOperationResult Client::generateInvites(TInviteInfoList &invites, const QDateTime &expiresDT, quint8 count,
                                         const TServiceList &services, QWidget *parent)
{
    if (!instance()->isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    if (!count || count > Texsample::MaximumInvitesCount)
        return TOperationResult(TMessage::ClientInvalidInvitesCountError);
    QVariantMap out;
    out.insert("expiration_dt", expiresDT);
    out.insert("count", count ? count : 1);
    out.insert("services", services);
    BNetworkOperation *op = instance()->mconnection->sendRequest(Texsample::GenerateInvitesRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    invites = in.value("invite_infos").value<TInviteInfoList>();
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::getInvitesList(TInviteInfoList &list, QWidget *parent)
{
    if (!instance()->isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    BNetworkOperation *op = instance()->mconnection->sendRequest(Texsample::GetInvitesListRequest);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    list = in.value("invite_infos").value<TInviteInfoList>();
    return in.value("operation_result").value<TOperationResult>();
}

/*============================== Public constructors =======================*/

Client::Client(QObject *parent) :
    QObject(parent)
{
    mstate = DisconnectedState;
    mreconnect = false;
    mconnection = new BNetworkConnection(BGenericSocket::TcpSocket, this);
    mconnection->installRequestHandler(BNetworkConnection::operation(BNetworkConnection::NoopOperation),
                                       &handleNoopRequest);
    connect(mconnection, SIGNAL(connected()), this, SLOT(connected()));
    connect(mconnection, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(mconnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    mhost = Global::host();
    mlogin = Global::login();
    mpassword = Global::encryptedPassword();
    if (Global::cachingEnabled())
        sCache->open();
    mid = 0;
    mlanguageChangeProxy = new BSignalDelayProxy(this);
    mlanguageChangeProxy->setConnection(bApp, SIGNAL(languageChanged()), this, SLOT(languageChanged()));
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
    QByteArray password = Global::encryptedPassword();
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
    return (DisconnectedState == mstate && !mhost.isEmpty() && !mlogin.isEmpty());
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

TServiceList Client::services() const
{
    return mservices;
}

quint64 Client::userId() const
{
    return mid;
}

TOperationResult Client::addUser(const TUserInfo &info, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    if (!info.isValid(TUserInfo::AddContext))
        return TOperationResult(TMessage::InvalidUserInfoError);
    QVariantMap out;
    out.insert("user_info", info);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::AddUserRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::editUser(const TUserInfo &info, QWidget *parent)
{
    if (!isAuthorized())
        return TOperationResult(TMessage::NotAuthorizedError);
    if (!info.isValid(TUserInfo::EditContext))
        return TOperationResult(TMessage::InvalidUserInfoError);
    QVariantMap out;
    out.insert("user_info", info);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::EditUserRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TOperationResult(TMessage::ClientOperationError);
    return in.value("operation_result").value<TOperationResult>();
}

TOperationResult Client::updateAccount(TUserInfo info, QWidget *parent)
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
}

TCompilationResult Client::compile(const QString &fileName, QTextCodec *codec, const TCompilerParameters &param,
                                   TCompilationResult &makeindexResult, TCompilationResult &dvipsResult,
                                   QWidget *parent)
{
    if (!isAuthorized())
        return TCompilationResult(TMessage::NotAuthorizedError);
    if (fileName.isEmpty())
        return TCompilationResult(TMessage::ClientInvalidFileNameError);
    TTexProject p(fileName, codec);
    if (!p.isValid())
        return TCompilationResult(TMessage::ClientFileSystemError);
    QVariantMap out;
    out.insert("project", p);
    out.insert("parameters", param);
    BNetworkOperation *op = mconnection->sendRequest(Texsample::CompileProjectRequest, out);
    showProgressDialog(op, parent);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return TCompilationResult(TMessage::ClientOperationError);
    TCompilationResult r = in.value("compilation_result").value<TCompilationResult>();
    if (!r)
        return r;
    r.setSuccess(in.value("compiled_project").value<TCompiledProject>().save(QFileInfo(fileName).path()));
    if (!r)
        r.setMessage(TMessage::ClientFileSystemError);
    makeindexResult = in.value("makeindex_result").value<TCompilationResult>();
    dvipsResult = in.value("dvips_result").value<TCompilationResult>();
    return r;
}

/*============================== Public slots ==============================*/

void Client::connectToServer()
{
    if (!canConnect() || (Global::encryptedPassword().isEmpty() && !Application::showLoginDialog()))
        return;
    if (Global::encryptedPassword().isEmpty())
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

void Client::showProgressDialog(BNetworkOperation *op, QWidget *parent)
{
    if (!op)
        return;
    if (op->waitForFinished(ProgressDialogDelay))
        return;
    BOperationProgressDialog dlg(op, chooseParent(parent));
    dlg.setWindowTitle(tr("Executing request...", "opdlg windowTitle"));
    dlg.setAutoCloseInterval(0);
    dlg.exec();
}

QWidget *Client::chooseParent(QWidget *supposed)
{
    return supposed ? supposed : Application::mostSuitableWindow();
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

void Client::setState(State s, const TAccessLevel &alvl, const TServiceList &list)
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
    mservices = list;
    if (palvl != alvl)
        emit accessLevelChanged(alvl);
    bool bccn = canConnect();
    bool bcdn = canDisconnect();
    if (bcc != bccn)
        emit canConnectChanged(bccn);
    if (bcd != bcdn)
        emit canDisconnectChanged(bcdn);
}

void Client::updateSampleInfos(const TSampleInfoList &newInfos, const TIdList &deletedInfos, const QDateTime &updateDT)
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
    out.insert("client_info", TClientInfo::createInfo());
    BNetworkOperation *op = mconnection->sendRequest(Texsample::AuthorizeRequest, out);
    showProgressDialog(op);
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return;
    TOperationResult r = in.value("operation_result").value<TOperationResult>();
    if (r)
    {
        mid = in.value("user_id").toULongLong();
        setState(AuthorizedState, in.value("access_level").value<TAccessLevel>(),
                 in.value("services").value<TServiceList>());
        updateSamplesList();
    }
    else
    {
        disconnectFromServer();
        showConnectionErrorMessage(r.messageString());
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

void Client::languageChanged()
{
    if (!isAuthorized())
        return;
    QVariantMap out;
    out.insert("locale", BApplication::locale());
    BNetworkOperation *op = mconnection->sendRequest(Texsample::ChangeLocaleRequest, out);
    showProgressDialog(op);
    op->deleteLater();
}

/*============================== Static private constants ==================*/

const int Client::ProgressDialogDelay = BeQt::Second / 2;
const int Client::MaxSampleSize = 199 * BeQt::Megabyte;

/*============================== Static private members ====================*/

Client *Client::minstance = 0;
