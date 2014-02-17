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
    explicit ApplicationServer(quint16 port, const QString &serverName, int operationTimeout = 5 * BeQt::Second);
protected:
    void handleMessage(const QStringList &arguments);
private:
    Q_DISABLE_COPY(ApplicationServer)
};

#endif // APPLICATIONSERVER_H
