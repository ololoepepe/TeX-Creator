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

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
ApplicationServer::ApplicationServer(quint16 port, int operationTimeout) :
    BApplicationServer(port, operationTimeout)
{
    //
}
#else
ApplicationServer::ApplicationServer(const QString &serverName, int operationTimeout) :
    BApplicationServer(serverName, operationTimeout)
{
    //
}
#endif

/*============================== Protected methods =========================*/

void ApplicationServer::handleMessage(const QStringList &arguments)
{
    bApp->handleExternalRequest(arguments);
}
