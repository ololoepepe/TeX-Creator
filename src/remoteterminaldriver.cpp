#include "remoteterminaldriver.h"
#include "client.h"
#include "application.h"

#include <BAbstractTerminalDriver>

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QTextCodec>

#include <QDebug>

/*============================================================================
================================ RemoteTerminalDriver ========================
============================================================================*/

/*============================== Public constructors =======================*/

RemoteTerminalDriver::RemoteTerminalDriver(QObject *parent) :
    BAbstractTerminalDriver(parent)
{
    mactive = false;
}

/*============================== Public methods ============================*/

bool RemoteTerminalDriver::processCommand(const QString &, const QStringList &, QString &)
{
    return false;
}

bool RemoteTerminalDriver::isActive() const
{
    return mactive;
}

QString RemoteTerminalDriver::read()
{
    QString s = mbuffer;
    mbuffer.clear();
    return s;
}

void RemoteTerminalDriver::close()
{
    //
}

bool RemoteTerminalDriver::terminalCommand(const QVariant &data, QString &error)
{
    mactive = true;
    emitBlockTerminal();
    Client::CompileParameters param;
    QVariantMap m = data.toMap();
    param.fileName = m.value("file_name").toString();
    param.codec = QTextCodec::codecForName(m.value("codec_name").toString().toLatin1());
    param.compiler = m.value("compiler").toString();
    param.makeindex = m.value("makeindex").toBool();
    param.dvips = m.value("dvips").toBool();
    param.options = m.value("options").toStringList();
    param.commands = m.value("commands").toStringList();
    int exitCode = -1;
    bool b = sClient->compile(param, &error, &exitCode, &mbuffer);
    emitReadyRead();
    emitUnblockTerminal();
    mactive = false;
    emitFinished(exitCode);
    return b;
}
