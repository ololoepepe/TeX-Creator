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

#include "client.h"

#include <TNetworkClient>

#include <QObject>

/*============================================================================
================================ Client ======================================
============================================================================*/

/*============================== Public constructors =======================*/

Client::Client(QObject *parent) :
    TNetworkClient(parent)
{
    //
}

Client::~Client()
{
    //
}

/*============================== Public methods ============================*/

/*bool Client::updateSettings()
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
}*/

/*============================== Static private methods ====================*/

/*void Client::showProgressDialog(BNetworkOperation *op, QWidget *parent)
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

/*============================== Static private constants ==================*/

//const int Client::ProgressDialogDelay = BeQt::Second / 2;
//const int Client::MaxSampleSize = 199 * BeQt::Megabyte;
