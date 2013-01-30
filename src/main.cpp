#include "applicationserver.h"
#include "mainwindow.h"
#include "application.h"

#include <BApplication>
#include <BDirTools>
#include <BTranslator>
#include <BLogger>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QFont>
#include <QRect>

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("TeX Creator");
    QApplication::setApplicationVersion("2.0.0-pa1");
    QApplication::setOrganizationName("TeXSample Team");
    QApplication::setOrganizationDomain("https://github.com/TeXSample-Team/TeX-Creator");
    //QApplication::setWindowIcon( QIcon(":/tex-creator.png") );
    QFont fnt = QApplication::font();
    fnt.setPointSize(10);
    QApplication::setFont(fnt);
    QStringList args = app.arguments();
    args.removeFirst();
    args.removeDuplicates();
    ApplicationServer s( QApplication::applicationName() + QDir::home().dirName() );
    int ret = 0;
    if ( !s.testServer() )
    {
        s.listen();
#if defined(BUILTIN_RESOURCES)
        Q_INIT_RESOURCE(tex_creator);
        Q_INIT_RESOURCE(tex_creator_doc);
        Q_INIT_RESOURCE(tex_creator_symbols);
        Q_INIT_RESOURCE(tex_creator_translations);
#endif
#if defined(Q_OS_UNIX)
        QApplication::addLibraryPath( QDir( QApplication::applicationDirPath() +
                                            "../lib/tex-creator" ).absolutePath() );
#endif
        Application bapp;
        Application::setThemedIconsEnabled(false);
        Application::setPreferredIconFormats(QStringList() << "png");
        Application::installTranslator( new BTranslator("beqt") );
        Application::installTranslator( new BTranslator("tex-creator") );
        BDirTools::createUserLocations(QStringList() << "autotext" << "klm" << "macros" << "texsample/cache");

        Application::createInitialWindow(args);
        Application::loadSettings();
        ret = app.exec();
        Application::saveSettings();
    }
    else
    {
        if ( args.isEmpty() )
            args << "";
        s.sendMessage(args);
    }
    return ret;
}
