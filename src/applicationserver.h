#ifndef APPLICATIONSERVER_H
#define APPLICATIONSERVER_H

class MainWindow;

class QTcpServer;
class QSignalMapper;

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QPointer>

class ApplicationServer : public QObject
{
    Q_OBJECT
public:
    static const QString JustWindow;
    //
    explicit ApplicationServer(QObject *parent = 0);
    //
    bool tryListen(quint16 port);
    void sendOpenFiles(quint16 port, const QStringList &files);
    MainWindow *createWindow( const QStringList &files = QStringList() );
private:
    QTcpServer *mServer;
    QList< QPointer<MainWindow> > mWindows;
    //
    void cleanup();
private slots:
    void newConnection();
};

#endif // APPLICATIONSERVER_H
