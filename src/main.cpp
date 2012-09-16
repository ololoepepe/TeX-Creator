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

#include <QDebug>

#define connect QObject::connect

const quint16 AppServerPort = 9021;

void initCore();

//

int main(int argc, char *argv[])
{
    QApplication *app = new QApplication(argc, argv);
    QApplication::setApplicationName("TeX Creator 2");
    QApplication::setApplicationVersion("0.2.0pa1");
    QApplication::setOrganizationName("Andrey Bogdanov");
    QApplication::setOrganizationDomain("https://github.com/TeXSample-Team/TeX-Creator");
    QApplication::setWindowIcon( QIcon(":/logo.png") );
    QStringList args = app->arguments();
    args.removeFirst();
    args.removeDuplicates();
    ApplicationServer *s = new ApplicationServer;
    int ret = 0;
    if ( s->tryListen(AppServerPort) )
    {
        initCore();
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

void initCore()
{
    //platform specific
#if defined(Q_OS_MAC)
    QApplication::addLibraryPath("/usr/lib/tex-creator/qt4/plugins"); //TODO
    QString sharedPluginsDir = "/usr/lib/tex-creator/plugins"; //TODO
    BCore::setSharedRoot("/usr/share/tex-creator"); //TODO
    BCore::setUserRoot(QDir::homePath() + "/Library/Application Support/Tex Creator");
#elif defined(Q_OS_UNIX)
    QApplication::addLibraryPath("/usr/lib/tex-creator/qt4/plugins");
    QString sharedPluginsDir = "/usr/lib/tex-creator/plugins";
    BCore::setSharedRoot("/usr/share/tex-creator");
    BCore::setUserRoot(QDir::homePath() + "/.tex-creator");
#elif defined(Q_OS_WIN)
    QString sharedPluginsDir = QApplication::applicationDirPath() + "/plugins";
    BCore::setSharedRoot( QApplication::applicationDirPath() );
    BCore::setUserRoot(QDir::homePath() + "/TeX Creator");
#endif
    //dirs
    BCore::setPath("macros", "macros");
    BCore::setPath("plugins", "plugins");
    BCore::setPath("translations", "translations");
    BCore::createUserPath("macros");
    BCore::createUserPath("plugins");
    BCore::createUserPath("translations");
    //translations
    BCore::addStandardTranslator(BCore::QtTranslator);
    BCore::addStandardTranslator(BCore::BCoreTranslator);
    BCore::addStandardTranslator(BCore::BGuiTranslator);
    BCore::addTranslator(":/res/translations/tex-creator");
    BCore::addTranslator(BCore::user("translations") + "/tex-creator");
    //plugins
    BCore::loadPlugins( BCore::user("plugins") );
    BCore::loadPlugins(sharedPluginsDir);
}
