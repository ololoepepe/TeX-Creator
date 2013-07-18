#include "remoteterminaldriver.h"
#include "client.h"
#include "application.h"

#include <TCompilerParameters>
#include <TCompilationResult>

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
    TCompilerParameters param;
    QVariantMap m = data.toMap();
    QString fn = m.value("file_name").toString();
    QTextCodec *codec = QTextCodec::codecForName(m.value("codec_name").toString().toLatin1());
    param.setCompiler(TCompilerParameters::PdfLaTex); //TODO
    param.setMakeindexEnabled(m.value("makeindex").toBool());
    param.setDvipsEnabled(m.value("dvips").toBool());
    param.setOptions(m.value("options").toStringList());
    param.setCommands(m.value("commands").toStringList());
    TCompilationResult mr;
    TCompilationResult dr;
    TCompilationResult r = sClient->compile(fn, codec, param, mr, dr);
    mbuffer = r.log();
    if (!mr.log().isEmpty())
        mbuffer += "\n\n" + mr.log();
    if (!dr.log().isEmpty())
        mbuffer += "\n\n" + dr.log();
    error = r.messageString();
    emitReadyRead();
    emitUnblockTerminal();
    mactive = false;
    emitFinished(r.exitCode());
    return r.success();
}
