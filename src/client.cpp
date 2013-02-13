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
    QVariantMap out;
    if ( data.fileName.isEmpty() || data.title.isEmpty() )
        return retErr( errs, tr("No file name or title", "errorString") );
    QString text = data.text;
    qint64 sz = !text.isEmpty() ? text.size() : QFileInfo(data.initialFileName).size();
    if (sz > MaxSampleSize)
        return retErr( errs, tr("The source is too big", "errorString") );
    if ( text.isEmpty() )
    {
        bool ok = false;
        text = BDirTools::readTextFile(data.initialFileName, data.codec, &ok);
        if ( !ok || text.isEmpty() )
            return retErr( errs, tr("Unable to get sample text", "errorString") );
    }
    text = withoutRestrictedCommands(text);
    QStringList rcmds = restrictedCommands(text);
    if ( !rcmds.isEmpty() )
        return retErr( errs, tr("Sample contains restricted commands:", "errorString") + "\n" + rcmds.join('\n') );
    QStringList fns = auxFileNames(text);
    if ( !fns.isEmpty() )
    {
        QStringList afns = absoluteFileNames(fns);
        if ( !afns.isEmpty() )
            return retErr( errs, tr("Absolute file references:", "errorString") + "\n" + afns.join('\n') );
        QFileInfo fi(data.initialFileName);
        QString path = fi.path();
        if ( !fi.isAbsolute() || !QDir(path).exists() )
            return retErr( errs, tr("File does not exist, unable to locate referenced files", "errorString") );
        QVariantList aux;
        foreach (const QString &fnl, fns)
        {
            QString fn = path + "/" + fnl;
            sz += QFileInfo(fn).size();
            if (sz > MaxSampleSize)
                return retErr( errs, tr("The sample is too big", "errorString") );
            bool ok = false;
            QByteArray ba = BDirTools::readFile(fn, -1, &ok);
            if (!ok)
                return retErr(errs, tr("Failed to read file:", "errorString") + "\n" + fnl);
            QVariantMap m;
            m.insert("file_name", fnl);
            m.insert("data", ba);
            aux << m;
        }
        out.insert("aux_files", aux);
    }
    out.insert("title", data.title);
    out.insert("text", text);
    out.insert("file_name", data.fileName);
    out.insert("tags", data.tags);
    out.insert("comment", data.comment);
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

QStringList Client::auxFileNames(const QString &text)
{
    QStringList list;
    if ( text.isEmpty() )
        return list;
    static const QString href = "((?<=\\\\href\\{)(.*)(?=#))";
    static const QString hrefRun = "((?<=\\\\href\\{run\\:)(.*)(?=\\}))";
    static const QString includegraphics = "((?<=\\includegraphics)(.*)(?=\\}))";
    static QRegularExpression rx("(" + href + "|" + hrefRun + "|" + includegraphics + ")");
    static QRegExp rx2("^(\\[.*\\])?\\{");
    QRegularExpressionMatchIterator i = rx.globalMatch(text);
    while ( i.hasNext() )
        list << i.next().capturedTexts();
    list.removeAll("");
    if ( !list.isEmpty() )
    {
        foreach ( int i, bRange(0, list.size() - 1) )
        {
            list[i].remove(rx2);
            if (list.at(i).right(1) == "\\")
                list[i].remove(list.at(i).length() - 1, 1);
        }
    }
    list.removeDuplicates();
    return list;
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
    QString spath = path + "/texsample-" + QString::number(id) + "-" + QFileInfo(fn).baseName();
    if ( !BDirTools::mkpath(spath) || !BDirTools::removeFilesInDir(spath) )
        return false;
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

bool Client::insertSample(BCodeEditorDocument *doc, quint64 id, const QString &fileName)
{
    if ( !doc || !id || fileName.isEmpty() )
        return false;
    doc->insertText("\\input \"texsample-" + QString::number(id) + "-" +
                    QFileInfo(fileName).baseName() + "/" + QFileInfo(fileName).fileName() + "\"");
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
