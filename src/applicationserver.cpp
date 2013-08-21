#include "applicationserver.h"
#include "application.h"

#include <BApplicationServer>

#include <QObject>
#include <QStringList>

#include <QDebug>

/*============================================================================
================================ ApplicationServer ===========================
============================================================================*/

/*============================== Public constructors =======================*/

ApplicationServer::ApplicationServer(quint16 port, const QString &serverName, int operationTimeout) :
    BApplicationServer(port, serverName, operationTimeout)
{
    //
}

/*============================== Protected methods =========================*/

void ApplicationServer::handleMessage(const QStringList &arguments)
{
    bApp->handleExternalRequest(arguments);
}
