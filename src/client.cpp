#include "client.h"
#include "sample.h"
#include "application.h"
#include "texsamplesettingstab.h"
#include "samplesmodel.h"
#include "requestprogressdialog.h"

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

#include <QDebug>
#include <QTimer>
#include <QEventLoop>

/*============================================================================
================================ Client ======================================
============================================================================*/

/*============================== Public constructors =======================*/

Client::Client(QObject *parent) :
    QObject(parent)
{
    mstate = DisconnectedState;
    mreconnect = false;
    mlastUpdated.setTimeSpec(Qt::UTC);
    msamplesModel = new SamplesModel(this);
    mconnection = new BNetworkConnection(BGenericSocket::TcpSocket, this);
    connect( mconnection, SIGNAL( connected() ), this, SLOT( connected() ) );
    connect( mconnection, SIGNAL( disconnected() ), this, SLOT( disconnected() ) );
    connect( mconnection, SIGNAL( error(QAbstractSocket::SocketError) ),
             this, SLOT( error(QAbstractSocket::SocketError) ) );
    mhost = TexsampleSettingsTab::getHost();
    mlogin = TexsampleSettingsTab::getLogin();
    mpassword = TexsampleSettingsTab::getPassword();
}

Client::~Client()
{
    //
}

/*============================== Static public methods =====================*/

SamplesModel *Client::samplesModelInstance()
{
    return sClient ? sClient->msamplesModel : 0;
}

/*============================== Public methods ============================*/

void Client::updateSettings()
{
    QString host = TexsampleSettingsTab::getHost();
    QString login = TexsampleSettingsTab::getLogin();
    QByteArray password = TexsampleSettingsTab::getPassword();
    if (host == mhost && login == mlogin && password == mpassword)
        return;
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
    {
        mreconnect = true;
        disconnectFromServer();
    }
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

bool Client::updateSamplesList(bool full, QString *errs, QWidget *parent)
{
    if ( !isAuthorized() )
        return retErr( errs, tr("Not authorized", "errorString") );
    QVariantMap out;
    out.insert( "last_update_dt", !full ? mlastUpdated : QDateTime() );
    BNetworkOperation *op = mconnection->sendRequest("get_samples_list", out);
    if ( !op->waitForFinished(100) )
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
    sModel->removeSamples(dlist);
    sModel->insertSamples(list);
    if ( m.contains("update_dt") )
        mlastUpdated = m.value("update_dt").toDateTime();
    return true;
}

bool Client::previewSample(quint64 id, QWidget *parent, bool full)
{
    if ( !id || !isAuthorized() )
        return false;
    QVariantMap out;
    out.insert("id", id);
    out.insert("full", full);
    out.insert("last_update_dt", mlastUpdated);
    BNetworkOperation *op = mconnection->sendRequest("get_sample_preview", out);
    if ( !op->waitForFinished(100) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap m = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() )
        return false;
    BDirTools::rmdir(QDir::tempPath() + "/tex-creator");
    QString fn = m.value("file_name").toString();
    QString fp = QDir::tempPath() + "/tex-creator/" + QFileInfo(fn).fileName();
    QByteArray ba = m.value("data").toByteArray();
    if ( fn.isEmpty() || ba.isEmpty() || !BDirTools::writeFile(fp, ba) )
        return false;
    bApp->openLocalFile(fp);
    return true;
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
    QString dpath = fi.path();
    QVariantMap out;
    out.insert("id", id);
    out.insert("last_update_dt", mlastUpdated);
    BNetworkOperation *op = mconnection->sendRequest("get_sample_source", out);
    if ( !op->waitForFinished(100) )
        RequestProgressDialog( op, chooseParent(edr) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() )
        return false;
    QString fn = in.value("file_name").toString();
    QString text = in.value("text").toString();
    if ( fn.isEmpty() || text.isEmpty() )
        return false;
    QString fnr = QFileInfo(fn).fileName();
    if ( !BDirTools::writeTextFile( dpath + "/" + fnr, text, doc->codec() ) )
        return false;
    doc->insertText( "\\input " + (fnr.contains(' ') ? fnr.prepend('\"').append('\"') : fnr) );
    return true;
}

bool Client::addSample(const SampleData &data, QString *errs, QString *log, QWidget *parent)
{
    if ( !isAuthorized() )
        return retErr( errs, tr("Not authorized", "errorString") );
    QVariantMap out;
    if ( data.fileName.isEmpty() || data.title.isEmpty() )
        return retErr( errs, tr("No file name or title", "errorString") );
    QString text = data.text;
    if ( text.isEmpty() )
    {
        bool ok = false;
        text = BDirTools::readTextFile(data.initialFileName, data.codec, &ok);
        if ( !ok || text.isEmpty() )
            return retErr( errs, tr("Unable to get sample text", "errorString") );
    }
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
        foreach (const QString &fn, fns)
        {
            bool ok = false;
            QByteArray ba = BDirTools::readFile(path + "/" + fn, -1, &ok);
            if (!ok)
                return retErr(errs, tr("Failed to read file:", "errorString") + "\n" + fn);
            QVariantMap m;
            m.insert("file_name", fn);
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
    if ( !op->waitForFinished(100) )
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

bool Client::deleteSample(quint64 id, QWidget *parent)
{
    if ( !id || !isAuthorized() )
        return false;
    QVariantMap out;
    out.insert("id", id);
    BNetworkOperation *op = mconnection->sendRequest("delete_sample", out);
    if ( !op->waitForFinished(100) )
        RequestProgressDialog( op, chooseParent(parent) ).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    if ( op->isError() )
        return false;
    if ( in.value("ok").toBool() )
        updateSamplesList();
    return true;
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

QStringList Client::restrictedCommands(const QString &text)
{
    QStringList list;
    if ( text.isEmpty() )
        return list;
    static const QStringList cmds = QStringList() << "\\documentclass" << "\\usepackage" << "\\makeindex"
                                                  << "\\begin{document}" << "\\input" << "\\end{document}";
    foreach (const QString &cmd, cmds)
        if ( text.contains(cmd) )
            list << cmd;
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

/*============================== Private methods ===========================*/

void Client::setState(State s)
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
    bool bccn = canConnect();
    bool bcdn = canDisconnect();
    if (bcc != bccn)
        emit canConnectChanged(bccn);
    if (bcd != bcdn)
        emit canDisconnectChanged(bcdn);
}

/*============================== Private slots =============================*/

void Client::connected()
{
    setState(ConnectedState);
    QVariantMap m;
    m.insert("login", mlogin);
    m.insert("password", mpassword);
    BNetworkOperation *op = mconnection->sendRequest("authorize", m);
    if ( !op->waitForFinished(100) )
        RequestProgressDialog( op, chooseParent() ).exec();
    if ( op->variantData().toMap().value("authorized", false).toBool() ) //TODO: Use access level
    {
        setState(AuthorizedState);
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
    setState(DisconnectedState);
    if (mreconnect)
    {
        mreconnect = false;
        connectToServer();
    }
}

void Client::error(QAbstractSocket::SocketError)
{
    setState(DisconnectedState);
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
