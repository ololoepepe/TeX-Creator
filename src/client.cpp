#include "client.h"
#include "sample.h"
#include "application.h"
#include "texsamplesettingstab.h"
#include "samplesmodel.h"
#include "requestprogressdialog.h"
#include "cache.h"

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
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QPushButton>
#include <QImage>
#include <QBuffer>
#include <QUuid>

#include <QDebug>

/*============================================================================
================================ Client ======================================
============================================================================*/

/*============================== Public constructors =======================*/

Client::Client(QObject *parent) :
    QObject(parent)
{
    mstate = DisconnectedState;
    maccessLevel = NoLevel;
    mreconnect = false;
    mlastUpdated.setTimeSpec(Qt::UTC);
    msamplesModel = new SamplesModel(this);
    mcache = new Cache;
    mconnection = new BNetworkConnection(BGenericSocket::TcpSocket, this);
    connect( mconnection, SIGNAL( connected() ), this, SLOT( connected() ) );
    connect( mconnection, SIGNAL( disconnected() ), this, SLOT( disconnected() ) );
    connect( mconnection, SIGNAL( error(QAbstractSocket::SocketError) ),
             this, SLOT( error(QAbstractSocket::SocketError) ) );
    mhost = TexsampleSettingsTab::getHost();
    mlogin = TexsampleSettingsTab::getLogin();
    mpassword = TexsampleSettingsTab::getPassword();
    if ( TexsampleSettingsTab::getCachingEnabled() )
        mcache->setHost(mhost);
}

Client::~Client()
{
    delete mcache;
}

/*============================== Static public methods =====================*/

SamplesModel *Client::samplesModelInstance()
{
    return sClient ? sClient->msamplesModel : 0;
}

QString Client::accessLevelToLocalizedString(AccessLevel lvl)
{
    switch (lvl)
    {
    case UserLevel:
        return tr("User", "access level");
    case ModeratorLevel:
        return tr("Moderator", "access level");
    case AdminLevel:
        return tr("Administrator", "access level");
    case NoLevel:
    default:
        return tr("No", "access level");
    }
}

/*============================== Public methods ============================*/

bool Client::updateSettings()
{
    QString host = TexsampleSettingsTab::getHost();
    QString login = TexsampleSettingsTab::getLogin();
    QByteArray password = TexsampleSettingsTab::getPassword();
    bool b = false;
    if (host != mhost || login != mlogin || password != mpassword)
    {
        bool bcc = canConnect();
        if (host != mhost)
            emit hostChanged(host);
        if (login != mlogin)
            emit loginChanged(login);
        mhost = host;
        mlogin = login;
        mpassword = password;
        bool bccn = canConnect();
        if (bcc != bccn)
            emit canConnectChanged(bccn);
        if (ConnectingState == mstate || ConnectedState == mstate || AuthorizedState == mstate)
            reconnect();
        b = true;
    }
    if ( TexsampleSettingsTab::getCachingEnabled() )
        mcache->setHost(host);
    else
        mcache->close();
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
    return ( DisconnectedState == mstate && !mhost.isEmpty() && !mlogin.isEmpty() );
}

bool Client::canDisconnect() const
{
    return (ConnectedState == mstate || AuthorizedState == mstate);
}

bool Client::isAuthorized() const
{
    return (AuthorizedState == mstate);
}

QString Client::login() const
{
    return mlogin;
}

int Client::accessLevel() const
{
    return maccessLevel;
}

QString Client::realName() const
{
    return mrealName;
}

QByteArray Client::avatar() const
{
    return mavatar;
}

bool Client::updateSamplesList(bool full, QString *errs, QWidget *parent)
{
    if ( !isAuthorized() )
        return retErr( errs, tr("Not authorized", "errorString") );
    QVariantMap out;
    out.insert( "last_update_dt", !full ? lastUpdated() : QDateTime() );
    BNetworkOperation *op = mconnection->sendRequest("get_samples_list", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap m = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() )
        return retErr( errs, operationErrorString() );
    QList<Sample> list;
    foreach ( const QVariant &v, m.value("samples").toList() )
        list << Sample::fromVariantMap( v.toMap() );
    QList<quint64> dlist;
    foreach ( const QVariant &v, m.value("deleted_samples").toList() )
        dlist << v.toMap().value("id").toULongLong();
    if ( mcache->isValid() )
    {
        mcache->removeSamplesFromList(dlist);
        mcache->insertSamplesIntoList(list);
    }
    if ( mcache->isValid() && sModel->isEmpty() )
    {
        sModel->setSamples( mcache->samplesList() );
    }
    else
    {
        sModel->removeSamples(dlist);
        sModel->insertSamples(list);
    }
    if ( m.contains("update_dt") )
        setLastUpdated( m.value("update_dt").toDateTime() );
    return true;
}

bool Client::previewSample(quint64 id, QWidget *parent, bool full)
{
    if ( !id || !isAuthorized() )
        return false;
    QVariantMap out;
    out.insert("id", id);
    out.insert("full", full);
    out.insert( "last_update_dt", mcache->samplePreviewUpdateDateTime(id) );
    BNetworkOperation *op = mconnection->sendRequest("get_sample_preview", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() )
        return false;
    if ( mcache->isValid() )
    {
        mcache->setSamplePreviewUpdateDateTime( id, in.value("update_dt").toDateTime() );
        if ( !in.value("cache_ok").toBool() )
            mcache->setSamplePreview(id, in);
        return mcache->showSamplePreview(id);
    }
    else
    {
        BDirTools::rmdir(QDir::tempPath() + "/tex-creator");
        QString fn = in.value("file_name").toString();
        QString fp = QDir::tempPath() + "/tex-creator/" + QFileInfo(fn).fileName();
        QByteArray ba = in.value("data").toByteArray();
        if ( fn.isEmpty() || ba.isEmpty() || !BDirTools::writeFile(fp, ba) )
            return false;
        return bApp->openLocalFile(fp);
    }
}

bool Client::insertSample(quint64 id, BCodeEditor *edr)
{
    if ( !id || !edr || !isAuthorized() )
        return false;
    BCodeEditorDocument *doc = edr->currentDocument();
    if (!doc)
        return false;
    QFileInfo fi( doc->fileName() );
    if ( !fi.exists() || !fi.isFile() )
        return false;
    QString path = fi.path();
    QString spath = sampleSubdirPath(path, id);
    if ( !spath.isEmpty() )
    {
        QMessageBox msg(edr);
        msg.setWindowTitle( tr("Reloading sample", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Question);
        msg.setText( tr("It seems like this sample is already in the target directory", "msgbox text") );
        msg.setInformativeText( tr("Do you want to download it again, or use existing version?",
                                   "magbox informativeText") );
        msg.addButton(tr("Download", "btn text"), QMessageBox::AcceptRole);
        QPushButton *btnEx = msg.addButton(tr("Use existing", "btn text"), QMessageBox::AcceptRole);
        msg.setDefaultButton(btnEx);
        msg.addButton(QMessageBox::Cancel);
        if (msg.exec() == QMessageBox::Cancel)
            return false;
        if (msg.clickedButton() == btnEx)
            return insertSample( doc, id, sampleSourceFileName(spath) );
    }
    QVariantMap out;
    out.insert("id", id);
    out.insert( "last_update_dt", mcache->sampleSourceUpdateDateTime(id) );
    BNetworkOperation *op = mconnection->sendRequest("get_sample_source", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(edr) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() )
        return false;
    if ( mcache->isValid() )
    {
        mcache->setSampleSourceUpdateDateTime( id, in.value("update_dt").toDateTime() );
        if ( in.value("cache_ok").toBool() )
            in = mcache->sampleSource(id);
        else
            mcache->setSampleSource(id, in);
    }
    return writeSample( path, id, in, doc->codec() ) && insertSample( doc, id, in.value("file_name").toString() );
}

bool Client::addSample(const SampleData &data, QString *errs, QString *log, QWidget *parent)
{
    if ( !isAuthorized() )
        return retErr( errs, tr("Not authorized", "errorString") );
    if ( data.fileName.isEmpty() || data.title.isEmpty() )
        return retErr( errs, tr("No file name or title", "errorString") );
    bool ok = false;
    QVariantMap out = packSample(data, &ok, errs);
    if (!ok)
        return false;
    BNetworkOperation *op = mconnection->sendRequest("add_sample", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() )
        return retErr( errs, operationErrorString() );
    bool b = in.value("ok").toBool();
    if (log)
        *log = in.value("log").toString();
    if (b)
        updateSamplesList();
    return b;
}

bool Client::updateSample(const Sample &newInfo, bool moderLevel, QWidget *parent)
{
    if ( !newInfo.id() || !isAuthorized() )
        return false;
    QVariantMap out;
    out.insert( "id", newInfo.id() );
    out.insert( "title", newInfo.title() );
    out.insert( "tags", newInfo.tags() );
    out.insert( "comment", newInfo.comment() );
    if (moderLevel)
    {
        out.insert( "type", newInfo.type() );
        out.insert( "rating", newInfo.rating() );
        out.insert( "admin_remark", newInfo.adminRemark() );
    }
    BNetworkOperation *op = mconnection->sendRequest("update_sample", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() )
        return false;
    bool b = in.value("ok").toBool();
    if (b)
        updateSamplesList();
    return b;
}

bool Client::deleteSample(quint64 id, const QString &reason, QWidget *parent)
{
    if ( !id || !isAuthorized() )
        return false;
    QVariantMap out;
    out.insert("id", id);
    out.insert("reason", reason);
    BNetworkOperation *op = mconnection->sendRequest("delete_sample", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() )
        return false;
    bool b = in.value("ok").toBool();
    if (b)
        updateSamplesList();
    return b;
}

bool Client::updateAccount(const QByteArray &password, const QString &realName, const QByteArray &avatar,
                           QWidget *parent)
{
    if ( password.isEmpty() || !isAuthorized() )
        return false;
    QVariantMap out;
    out.insert("password", password);
    out.insert("real_name", realName);
    out.insert("avatar", avatar);
    BNetworkOperation *op = mconnection->sendRequest("update_account", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    return !op->isError() && in.value("ok").toBool();
}

bool Client::generateInvite(QUuid &invite, const QDateTime &expires, QWidget *parent)
{
    if (!isAuthorized())
        return false;
    QVariantMap out;
    out.insert("expires_dt", expires);
    BNetworkOperation *op = mconnection->sendRequest("generate_invite", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    invite = in.value("uuid").toUuid();
    return !op->isError() && !invite.isNull();
}

bool Client::getInvitesList(QList<Invite> &list, QWidget *parent)
{
    if (!isAuthorized())
        return false;
    BNetworkOperation *op = mconnection->sendRequest("get_invites_list");
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    QVariantList vl = in.value("list").toList();
    bool b = !op->isError() && in.value("ok").toBool();
    if (b)
    {
        foreach (const QVariant &v, vl)
        {
            QVariantMap vm = v.toMap();
            Invite inv;
            inv.uuid = vm.value("uuid").toUuid();
            inv.expires = vm.value("expires_dt").toDateTime().toLocalTime();
            list << inv;
        }
    }
    return b;
}

bool Client::addUser(const QString &login, const QByteArray &password, const QString &realName, int accessLevel,
                     QWidget *parent)
{
    if ( login.isEmpty() || password.isEmpty() ||
         !bRange(NoLevel, AdminLevel).contains(accessLevel) || !isAuthorized() )
        return false;
    QVariantMap out;
    out.insert("login", login);
    out.insert("password", password);
    out.insert("real_name", realName);
    out.insert("access_level", accessLevel);
    BNetworkOperation *op = mconnection->sendRequest("add_user", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    return !op->isError() && in.value("ok").toBool();
}

Client::UserInfo Client::getUserInfo(const QString &login, QWidget *parent)
{
    UserInfo info;
    if ( login.isEmpty() || !isAuthorized() )
        return info;
    QVariantMap out;
    out.insert("login", login);
    out.insert( "last_update_dt", mcache->userInfoUpdateDateTime(login) );
    BNetworkOperation *op = mconnection->sendRequest("get_user_info", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() || in.isEmpty() )
        return info;
    if ( mcache->isValid() )
    {
        mcache->setUserInfoUpdateDateTime( login, in.value("update_dt").toDateTime() );
        if ( in.value("cache_ok").toBool() )
            return mcache->userInfo(login);
        UserInfo info = userInfoFromVariantMap(in, login);
        mcache->setUserInfo(info);
        return info;
    }
    else
    {
        return userInfoFromVariantMap(in, login);
    }
}

bool Client::compile(const CompileParameters &param, QString *errs, int *exitCode, QString *log, QWidget *parent)
{
    if ( !isAuthorized() )
        return bRet(errs, tr("Not authorized", "errorString"), exitCode, -1, false);
    if (param.fileName.isEmpty())
        return bRet(errs, tr("No file name or title", "errorString"), exitCode, -1, false);
    bool ok = false;
    QVariantMap out = packProject(param, &ok, errs);
    if (!ok)
        return false;
    BNetworkOperation *op = mconnection->sendRequest("compile", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if (op->isError())
        return bRet(errs, operationErrorString(), exitCode, -1, false);
    int code = in.value("exit_code", -1).toInt();
    if (code < 0 || !in.value("ok").toBool())
        return bRet(errs, tr("Compilation failed", "errorString"), exitCode, code, false);
    QString l = in.value("log").toString();
    QFileInfo fi(param.fileName);
    QString path = fi.path();
    QString bfn = fi.baseName();
    ok = false;
    bool pdf = param.compiler.contains("pdf");
    QStringList suffixes = QStringList() << "aux" << "idx" << "log" << "out" << "toc" << (pdf ? "pdf" : "dvi");
    if (!pdf && param.dvips)
        suffixes << "ps";
    foreach (const QString &suff, suffixes)
    {
        ok = BDirTools::writeFile(path + "/" + bfn + "." + suff, in.value(suff).toByteArray());
        if (!ok)
            break;
    }
    return bRet(errs, ok ? QString() : tr("Failed to save file", "errorString"), log, l, exitCode, code, ok);
}

/*============================== Public slots ==============================*/

void Client::connectToServer()
{
    if ( !canConnect() )
        return;
    if ( TexsampleSettingsTab::getPassword().isEmpty() && !Application::showPasswordDialog() )
        return;
    if ( TexsampleSettingsTab::getPassword().isEmpty() )
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
    mconnection->connectToHost(mhost, 9041);
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
    if ( !canDisconnect() )
        return;
    setState(DisconnectingState);
    mconnection->disconnectFromHost();
}

/*============================== Static private methods ====================*/

QStringList Client::auxFileNames(const QString &text, const QString &path, QTextCodec *codec, bool *ok)
{
    QStringList list;
    bool b = !path.isEmpty();
    if (text.isEmpty())
        return list;
    if (b && !QDir(path).exists())
        return bRet(ok, false, list);
    QStringList patterns;
    patterns << "((?<=\\includegraphics)(.+)(?=\\}))"; //includegraphics[...]{...}
    if (b)
    {
        //TODO
        patterns << "((?<=\\\\input )(\\S+))"; //input "..."
    }
    else
    {
        patterns << "((?<=\\\\href\\{)(.+)(?=#))"; //href{...}{...}
        patterns << "((?<=\\\\href\\{run\\:)(.+)(?=\\}))"; //href{run:...}{...}
    }
    QRegularExpressionMatchIterator i = QRegularExpression("(" + patterns.join('|') + ")").globalMatch(text);
    while ( i.hasNext() )
        list << i.next().capturedTexts();
    list.removeAll("");
    QStringList schemes = QStringList() << "http" << "https" << "ftp";
    if ( !list.isEmpty() )
    {
        foreach (int i, bRange(list.size() - 1, 0))
        {
            list[i].remove(QRegExp("^(\\[.*\\])?\\{"));
            if (list.at(i).right(1) == "\\")
                list[i].remove(list.at(i).length() - 1, 1);
            list[i] = BeQt::unwrapped(list.at(i));
            foreach (const QString &s, schemes) //Skip external (network) links
                if (list.at(i).left((s + "://").length()) == s + "://")
                    list.removeAt(i);
        }
    }
    if (b)
    {
        list.removeAll("texsample.tex"); //Removing global dependency
        foreach (const QString &fn, list)
        {
            if (QFileInfo(fn).suffix().compare("tex", Qt::CaseInsensitive))
                continue;
            bool bok = false;
            QString t = BDirTools::readTextFile(path + "/" + fn, codec, &bok);
            if (!bok)
                return bRet(ok, false, list);
            if (t.isEmpty())
                continue;
            bok = false;
            list << auxFileNames(t, path, codec, &bok);
            if (!bok)
                return bRet(ok, false, list);
        }
    }
    list.removeDuplicates();
    return bRet(ok, b, list);
}

QString Client::withoutRestrictedCommands(const QString &text)
{
    if ( text.isEmpty() )
        return text;
    QStringList sl = text.split('\n');
    static QRegularExpression rx(".*\\\\(documentclass|makeindex|begin\\{document\\}|end\\{document\\}).*");
    foreach ( int i, bRange(sl.size() - 1, 0, -1) )
    {
        QString &line = sl[i];
        if ( line.isEmpty() )
            continue;
        line.remove(rx);
        if ( line.isEmpty() )
            sl.removeAt(i);
    }
    return sl.join('\n');
}

QStringList Client::restrictedCommands(const QString &text)
{
    QStringList list;
    if ( text.isEmpty() )
        return list;
    QStringList cmds = QStringList() << "\\documentclass" << "\\usepackage" << "\\makeindex"
                                     << "\\begin{document}" << "\\input" << "\\end{document}";
    foreach ( const QString &line, text.split('\n') )
    {
        foreach (const QString &cmd, cmds)
        {
            int ind = line.indexOf(cmd);
            if (!ind || ( ind > 0 && !line.left(ind).contains('%') ) )
            {
                list << cmd;
                cmds.removeAll(cmd);
                if ( cmds.isEmpty() )
                    return list;
            }
        }
    }
    return list;
}

QStringList Client::absoluteFileNames(const QStringList &fileNames)
{
    QStringList list;
    if ( fileNames.isEmpty() )
        return list;
    foreach (const QString &fn, fileNames)
        if ( !fn.isEmpty() && QFileInfo(fn).isAbsolute() )
            list << fn;
    return list;
}

bool Client::retErr(QString *errs, const QString &string)
{
    if (errs)
        *errs = string;
    return false;
}

QWidget *Client::chooseParent(QWidget *supposed)
{
    return supposed ? supposed : Application::mostSuitableWindow();
}

QString Client::operationErrorString()
{
    return tr("Operation failed due to connection error", "errorString");
}

QString Client::sampleSubdirName(quint64 id, const QString &fileName)
{
    if (!id || fileName.isEmpty())
        return "";
    return "texsample-" + QString::number(id) + "-" + QFileInfo(fileName).baseName();
}

QString Client::sampleSubdirPath(const QString &path, quint64 id)
{
    if (path.isEmpty() || !id)
        return "";
    QDir d(path);
    if ( !d.exists() )
        return "";
    QString part = "texsample-" + QString::number(id) + "-";
    foreach ( const QString &p, d.entryList(QDir::Dirs | QDir::NoDotAndDotDot) )
        if (p.left( part.length() ) == part)
            return d.absoluteFilePath(p);
    return "";
}

QString Client::sampleSourceFileName(const QString &subdirPath)
{
    if ( subdirPath.isEmpty() )
        return "";
    QDir d(subdirPath);
    if ( !d.exists() )
        return "";
    QStringList files = d.entryList(QStringList() << "*.tex", QDir::Files);
    return (files.size() == 1) ? d.absoluteFilePath( files.first() ) : QString();
}

bool Client::writeSample(const QString &path, quint64 id, const QVariantMap &sample, QTextCodec *codec)
{
    if ( sample.isEmpty() || !id || !QDir(path).exists() )
        return false;
    QString fn = sample.value("file_name").toString();
    QString text = sample.value("text").toString();
    if ( fn.isEmpty() || text.isEmpty() )
        return false;
    QString subdir = sampleSubdirName(id, fn);
    QString spath = path + "/" + subdir;
    if ( !BDirTools::mkpath(spath) || !BDirTools::removeFilesInDir(spath) )
        return false;
    foreach (const QString aux, auxFileNames(text))
    {
        int auxlen = aux.length();
        int ind = text.indexOf(aux);
        while (ind >= 0)
        {
            if ((!ind || text.at(ind - 1) == '{' || text.at(ind - 1) == ' ')
                && (ind + auxlen == text.length() - 1 || text.at(ind + auxlen) == '}' || text.at(ind + auxlen) == ' '))
            {
                QString repl = subdir + "/" + aux;
                text.replace(ind, aux.length(), repl);
                ind = text.indexOf(aux, ind + repl.length());
            }
            else
            {
                ind = text.indexOf(aux, ind + auxlen);
            }
        }
    }
    if ( !BDirTools::writeTextFile( spath + "/" + QFileInfo(fn).fileName(), text, codec) )
        return false;
    foreach ( const QVariant &v, sample.value("aux_files").toList() )
    {
        QVariantMap m = v.toMap();
        QString afn = m.value("file_name").toString();
        if ( afn.isEmpty() )
            return false;
        if ( !BDirTools::writeFile( spath + "/" + QFileInfo(afn).fileName(), m.value("data").toByteArray() ) )
            return false;
    }
    return true;
}

QVariantMap Client::packProject(const CompileParameters &param, bool *ok, QString *errs)
{
    QVariantMap out;
    bool b = packTextFile(out, "", param.fileName, param.fileName, param.codec);
    QString text = out.value("text").toString();
    if (!b || text.isEmpty())
        return bRet(ok, false, errs, tr("Unable to get sample text", "errorString"), out);
    qint64 sz = text.toUtf8().size();
    if (sz > MaxSampleSize)
        return bRet(ok, false, errs, tr("The source is too big", "errorString"), out);
    QString path = QFileInfo(param.fileName).path();
    bool bok = false;
    QStringList auxfns = auxFileNames(text, path, param.codec, &bok);
    if (!bok)
        return bRet(ok, false, errs, tr("Failed to find dependencies", "errorString"), out);
    QStringList absfns = absoluteFileNames(auxfns);
    if (!absfns.isEmpty())
        return bRet(ok, false, errs, tr("Absolute file references:", "errorString") + "\n" + absfns.join('\n'), out);
    bok = false;
    QVariantList aux = packAuxFiles(auxfns, path, &bok, errs, &sz);
    if (!bok)
        return bRet(ok, false, out);
    out.insert("compiler", param.compiler);
    out.insert("makeindex", param.makeindex);
    out.insert("dvips", param.dvips);
    out.insert("options", param.options);
    out.insert("commands", param.commands);
    if (!aux.isEmpty())
        out.insert("aux_files", aux);
    return bRet(ok, true, out);
}

QVariantMap Client::packSample(const SampleData &data, bool *ok, QString *errs)
{
    QVariantMap out;
    bool b = packTextFile(out, data.text, data.fileName, data.initialFileName, data.codec);
    QString text = withoutRestrictedCommands(out.value("text").toString());
    if (!b || text.isEmpty())
        return bRet(ok, false, errs, tr("Unable to get sample text", "errorString"), out);
    qint64 sz = text.toUtf8().size();
    if (sz > MaxSampleSize)
        return bRet(ok, false, errs, tr("The source is too big", "errorString"), out);
    QString rcmds = restrictedCommands(text).join('\n');
    if (!rcmds.isEmpty())
        return bRet(ok, false, errs, tr("Sample contains restricted commands:", "errorString") + "\n" + rcmds, out);
    bool bok = false;
    QStringList auxfns = auxFileNames(text);
    QStringList absfns = absoluteFileNames(auxfns);
    if (!absfns.isEmpty())
        return bRet(ok, false, errs, tr("Absolute file references:", "errorString") + "\n" + absfns.join('\n'), out);
    QVariantList aux = packAuxFiles(auxfns, QFileInfo(data.initialFileName).path(), &bok, errs, &sz);
    if (!bok)
        return bRet(ok, false, out);
    out.insert("title", data.title);
    out.insert("text", text); //Reinserting the text without restricted commands
    out.insert("tags", data.tags);
    out.insert("comment", data.comment);
    if (!aux.isEmpty())
        out.insert("aux_files", aux);
    return bRet(ok, true, out);
}

QVariantList Client::packAuxFiles(const QStringList &fileNames, const QString &path,
                                  bool *ok, QString *errs, qint64 *sz)
{
    QVariantList list;
    if (fileNames.isEmpty())
        return bRet(ok, true, list);
    if (path.isEmpty() || !QDir(path).exists())
        return bRet(ok, false, errs, tr("Unable to locate referenced files", "errorString"), list);
    foreach (const QString &fn, fileNames)
    {
        QString afn = path + "/" + fn;
        QFileInfo fi(afn);
        if (sz)
        {
            *sz += fi.size();
            if (*sz > MaxSampleSize)
                return bRet(ok, false, errs, tr("The sample is too big", "errorString"), list);
        }
        bool bok = false;
        QVariantMap m = packFile(path, fn, PackAsBinary, 0, &bok);
        if (!bok)
            return bRet(ok, false, errs, tr("Failed to read file:", "errorString") + "\n" + fn, list);
        list << m;
    }
    return bRet(ok, true, list);
}

bool Client::insertSample(BCodeEditorDocument *doc, quint64 id, const QString &fileName)
{
    if ( !doc || !id || fileName.isEmpty() )
        return false;
    doc->insertText("\\input " + BeQt::wrapped(sampleSubdirName(id, fileName) + "/" + QFileInfo(fileName).fileName()));
    return true;
}

Client::UserInfo Client::userInfoFromVariantMap(const QVariantMap &m, const QString &login)
{
    UserInfo info;
    bool ok = false;
    info.accessLevel = static_cast<AccessLevel>( m.value("access_level", NoLevel).toInt(&ok) );
    if (!ok)
        return info;
    info.login = login;
    info.realName = m.value("real_name").toString();
    info.avatar = m.value("avatar").toByteArray();
    return info;
}

bool Client::packFile(QVariantMap &target, const QString &path, const QString &relativeFileName,
                      FilePackingMode mode, QTextCodec *codec)
{
    QFileInfo fi(relativeFileName);
    QDir d(path);
    if (path.isEmpty() || !d.exists() || fi.isAbsolute())
        return false;
    bool bok = false;
    if (PackAsText == mode || (PackAuto == mode && !fi.suffix().compare("tex", Qt::CaseInsensitive)))
    {
        QString text = BDirTools::readTextFile(d.absoluteFilePath(relativeFileName), codec, &bok);
        if (bok)
            target.insert("text", text);
    }
    else
    {
        QByteArray ba = BDirTools::readFile(d.absoluteFilePath(relativeFileName), -1, &bok);
        if (bok)
            target.insert("data", ba);
    }
    if (!bok)
        return false;
    QString spath = fi.path();
    if (!spath.isEmpty() && spath != ".")
        target.insert("subpath", spath);
    target.insert("file_name", fi.fileName());
    return true;
}

bool Client::packFile(QVariantMap &target, const QString &fileName, FilePackingMode mode, QTextCodec *codec)
{
    QFileInfo fi(fileName);
    return !fileName.isEmpty() && fi.isAbsolute() && packFile(target, fi.path(), fi.fileName(), mode, codec);
}

QVariantMap Client::packFile(const QString &path, const QString &relativeFileName,
                             FilePackingMode mode, QTextCodec *codec, bool *ok)
{
    QVariantMap m;
    bool bok = packFile(m, path, relativeFileName, mode, codec);
    return bRet(ok, bok, m);
}

QVariantMap Client::packFile(const QString &fileName, FilePackingMode mode, QTextCodec *codec, bool *ok)
{
    QVariantMap m;
    bool bok = packFile(m, fileName, mode, codec);
    return bRet(ok, bok, m);
}

bool Client::packTextFile(QVariantMap &target, const QString &text, const QString &fileName,
                          const QString &initialFileName, QTextCodec *codec)
{
    if (fileName.isEmpty())
        return false;
    if (text.isEmpty())
    {
        if (!packFile(target, initialFileName, PackAsText, codec))
            return false;
    }
    else
    {
        target.insert("text", text);
    }
    target.insert("file_name", fileName);
    return true;
}

QVariantMap Client::packTextFile(const QString &text, const QString &fileName,
                                 const QString &initialFileName, QTextCodec *codec, bool *ok)
{
    QVariantMap m;
    bool bok = packTextFile(m, text, fileName, initialFileName, codec);
    return bRet(ok, bok, m);
}

/*============================== Private methods ===========================*/

void Client::setState(State s, int accessLvl, const QString &realName, const QByteArray &avatar)
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
    if (accessLvl >= 0 && accessLvl != maccessLevel)
    {
        maccessLevel = accessLvl;
        emit accessLevelChanged(accessLvl);
    }
    bool bccn = canConnect();
    bool bcdn = canDisconnect();
    if (bcc != bccn)
        emit canConnectChanged(bccn);
    if (bcd != bcdn)
        emit canDisconnectChanged(bcdn);
    if (accessLvl >= 0 && realName != mrealName)
    {
        mrealName = realName;
        emit realNameChanged(realName);
    }
    mavatar = avatar;
}

QDateTime Client::lastUpdated() const
{
    return ( mlastUpdated.isValid() || !mcache->isValid() ) ? mlastUpdated : mcache->samplesListUpdateDateTime();
}

void Client::setLastUpdated(const QDateTime &dt)
{
    mlastUpdated = dt;
    if ( mcache->isValid() )
        mcache->setSamplesListUpdateDateTime(dt);
}

/*============================== Private slots =============================*/

void Client::connected()
{
    setState(ConnectedState);
    QVariantMap out;
    out.insert("login", mlogin);
    out.insert("password", mpassword);
    out.insert( "last_update_dt", mcache->userInfoUpdateDateTime(mlogin) );
    BNetworkOperation *op = mconnection->sendRequest("authorize", out);
    if ( !op->waitForFinished(ProgressDialogDelay) )
        RequestProgressDialog( op, chooseParent() ).exec();
    QVariantMap in = op->variantData().toMap();
    if ( in.value("authorized", false).toBool() )
    {
        UserInfo info;
        if ( mcache->isValid() )
        {
            mcache->setUserInfoUpdateDateTime( mlogin, in.value("update_dt").toDateTime() );
            bool b = in.value("cache_ok", false).toBool();
            info = b ? mcache->userInfo(mlogin) : userInfoFromVariantMap(in, mlogin);
            if (!b)
                mcache->setUserInfo(info);
        }
        else
        {
            info = userInfoFromVariantMap(in, mlogin);
        }
        setState(AuthorizedState, info.accessLevel, info.realName, info.avatar);
        updateSamplesList();
    }
    else
    {
        disconnectFromServer();
    }
    op->deleteLater();
}

void Client::disconnected()
{
    setState(DisconnectedState, NoLevel);
    if (mreconnect)
    {
        mreconnect = false;
        connectToServer();
    }
}

void Client::error(QAbstractSocket::SocketError)
{
    setState(DisconnectedState, NoLevel);
    QString errorString = mconnection->errorString();
    if ( mconnection->isConnected() )
        mconnection->close();
    QMessageBox msg( Application::mostSuitableWindow() );
    msg.setWindowTitle( tr("TeXSample connection error", "msgbox windowTitle") );
    msg.setIcon(QMessageBox::Critical);
    msg.setText(tr("The following connection error occured:", "msgbox text") + "\n" + errorString);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    msg.exec();
}

/*============================== Static private constants ==================*/

const int Client::ProgressDialogDelay = BeQt::Second / 2;
const int Client::MaxSampleSize = 199 * BeQt::Megabyte;
