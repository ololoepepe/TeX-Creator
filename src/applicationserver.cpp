#include "src/applicationserver.h"
#include "src/mainwindow.h"

#include <bcore.h>
#include <btexteditor.h>

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QScopedPointer>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QDataStream>
#include <QIODevice>
#include <QHostAddress>
#include <QPointer>
#include <QMessageBox>
#include <QRect>

#include <QDebug>

const int OperationTimeout = 5000;

//

const QString ApplicationServer::JustWindow = "///just window///";

//

ApplicationServer::ApplicationServer(QObject *parent) :
    QObject(parent)
{
    mServer = new QTcpServer(this);
    connect( mServer, SIGNAL( newConnection() ), this, SLOT( newConnection() ) );
}

//

bool ApplicationServer::tryListen(quint16 port)
{
    if (port <= 1000)
        return false;
    return mServer->listen(QHostAddress::LocalHost, port);
}

void ApplicationServer::sendOpenFiles(quint16 port, const QStringList &files)
{
    if (port <= 1000)
        return;
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    for (int i = 0; i < files.size(); ++i)
        out << files.at(i);
    if ( ba.isEmpty() )
        return;
    QScopedPointer<QTcpSocket> s(new QTcpSocket);
    s->connectToHost(QHostAddress::LocalHost, port);
    if ( !s->waitForConnected(OperationTimeout) )
        return;
    if (s->write(ba) < 0)
        return;
    s->waitForBytesWritten(OperationTimeout);
}

MainWindow *ApplicationServer::createWindow(const QStringList &files)
{
    QPointer<MainWindow> mw(new MainWindow);
    mw->setAttribute(Qt::WA_DeleteOnClose, true);
    BCore::addPluginHandlingObject(mw);
    mw->show();
    if ( !files.isEmpty() )
        mw->textEditor()->openFiles(files);
    mWindows << mw;
    return mw;
}

//

void ApplicationServer::cleanup()
{
    for (int i = mWindows.size() - 1; i >= 0; --i)
        if ( !mWindows.at(i) )
            mWindows.removeAt(i);
}

//

void ApplicationServer::newConnection()
{
    if ( !mServer->hasPendingConnections() )
        return;
    QScopedPointer<QTcpSocket> s( mServer->nextPendingConnection() );
    if ( !s->waitForReadyRead(OperationTimeout) )
        return;
    QByteArray ba = s->readAll();
    QDataStream in(ba);
    QStringList files;
    while ( !in.atEnd() )
    {
        QString file;
        in >> file;
        files << file;
    }
    files.removeDuplicates();
    cleanup();
    if ( mWindows.isEmpty() )
        createWindow(files.contains(JustWindow) ? QStringList() : files);
    if ( files.isEmpty() )
        return;
    bool multiple = BCore::multipleInstancesEnabled();
    if ( files.contains(JustWindow) )
    {
        if (multiple)
            createWindow();
        return;
    }
    if (multiple)
        createWindow(files);
    else
        mWindows.first()->textEditor()->openFiles(files);
}
