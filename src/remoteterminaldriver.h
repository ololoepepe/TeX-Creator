#ifndef REMOTETERMINALDRIVER_H
#define REMOTETERMINALDRIVER_H

class QStringList;
class QVariant;

#include <BAbstractTerminalDriver>
#include <QString>

/*============================================================================
================================ RemoteTerminalDriver ========================
============================================================================*/

class RemoteTerminalDriver : public BAbstractTerminalDriver
{
    Q_OBJECT
public:
    explicit RemoteTerminalDriver(QObject *parent = 0);
public:
    bool processCommand(const QString &command, const QStringList &arguments, QString &error);
    bool isActive() const;
    QString read();
    void close();
    bool terminalCommand(const QVariant &data, QString &error);
private:
    bool mactive;
    QString mbuffer;
};

#endif // REMOTETERMINALDRIVER_H
