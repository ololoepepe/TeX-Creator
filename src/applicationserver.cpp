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

ApplicationServer::ApplicationServer(const QString &serverName, int operationTimeout) :
    BApplicationServer(serverName, operationTimeout)
{
    //
}

/*============================== Protected methods =========================*/

void ApplicationServer::handleMessage(const QStringList &arguments)
{
    bApp->handleExternalRequest(arguments);
}
