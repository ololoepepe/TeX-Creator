#include "src/applicationserver.h"
#include "src/mainwindow.h"

#include <bcore.h>
#include <btexteditor.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QFont>

#include <QDebug>

#define connect QObject::connect

//

const quint16 AppServerPort = 9021;

//

int main(int argc, char *argv[])
{
    QApplication *app = new QApplication(argc, argv);
    QApplication::setApplicationName("TeX Creator");
    QApplication::setApplicationVersion("1.0.0a1");
    QApplication::setOrganizationName("TeXSample Team");
    QApplication::setOrganizationDomain("https://github.com/TeXSample-Team/TeX-Creator");
    QApplication::setWindowIcon( QIcon(":/tex-creator.png") );
    QFont fnt = QApplication::font();
    fnt.setPointSize(10);
    QApplication::setFont(fnt);
    QStringList args = app->arguments();
    args.removeFirst();
    args.removeDuplicates();
    ApplicationServer *s = new ApplicationServer;
    int ret = 0;
    if ( s->tryListen(AppServerPort) )
    {
#if defined(Q_OS_UNIX)
        QApplication::addLibraryPath("/usr/lib/tex-creator/qt4/plugins");
#endif
        BCore::init();
        BCore::setPath("doc", "doc");
        BCore::setPath("autotext", "autotext");
        BCore::setPath("klm", "klm");
        BCore::setPath("macros", "macros");
        BCore::createUserPath("autotext");
        BCore::createUserPath("klm");
        BCore::createUserPath("macros");
        BCore::loadSettings();
        s->createWindow(args);
        ret = app->exec();
        BCore::saveSettings();
    }
    else
    {
        if ( !args.isEmpty() )
            s->sendOpenFiles(AppServerPort, args);
        else
            s->sendOpenFiles(AppServerPort, QStringList() << ApplicationServer::JustWindow);
    }
    return ret;
}
