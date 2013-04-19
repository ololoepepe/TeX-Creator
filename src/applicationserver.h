#ifndef APPLICATIONSERVER_H
#define APPLICATIONSERVER_H

class QStringList;

#include <BApplicationServer>
#include <BeQt>

#include <QObject>

/*============================================================================
================================ ApplicationServer ===========================
============================================================================*/

class ApplicationServer : public BApplicationServer
{
public:
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    explicit ApplicationServer(quint16 port, int operationTimeout = 5 * BeQt::Second);
#else
    explicit ApplicationServer(const QString &serverName, int operationTimeout = 5 * BeQt::Second);
#endif
protected:
    void handleMessage(const QStringList &arguments);
private:
    Q_DISABLE_COPY(ApplicationServer)
};

#endif // APPLICATIONSERVER_H
