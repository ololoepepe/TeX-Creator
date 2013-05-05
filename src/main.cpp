#include "applicationserver.h"
#include "mainwindow.h"
#include "application.h"

#include <TeXSampleGlobal>

#include <BApplication>
#include <BDirTools>
#include <BTranslator>
#include <BLogger>
#include <BAboutDialog>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QFont>
#include <QPixmap>
#include <QHash>

#include <QDebug>

int main(int argc, char *argv[])
{
    tRegister();
    QApplication app(argc, argv);
    QApplication::setApplicationName("TeX Creator");
    QApplication::setApplicationVersion("2.0.0-beta2");
    QApplication::setOrganizationName("TeXSample Team");
    QApplication::setOrganizationDomain("https://github.com/TeXSample-Team/TeX-Creator");
    QFont fnt = QApplication::font();
    fnt.setPointSize(10);
    QApplication::setFont(fnt);
    QStringList args = app.arguments();
    args.removeFirst();
    args.removeDuplicates();
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    ApplicationServer s(9940 + qHash(QDir::home().dirName()) % 10);
#else
    ApplicationServer s(QApplication::applicationName() + QDir::home().dirName());
#endif
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
        QIcon icn = Application::icon("tex");
        QApplication::setWindowIcon(icn);
        Application::installTranslator(new BTranslator("qt"));
        Application::installTranslator(new BTranslator("beqt"));
        Application::installTranslator(new BTranslator("texsample"));
        Application::installTranslator(new BTranslator("tex-creator"));
        BAboutDialog *ad = Application::aboutDialogInstance();
        ad->setMinimumSize(650, 400);
        ad->setOrganization(QApplication::organizationName(), "2012-2013");
        ad->setWebsite( QApplication::organizationDomain() );
        ad->setPixmap( icn.pixmap( icn.availableSizes().first() ) );
        ad->setDescriptionFile(BDirTools::findResource("description", BDirTools::GlobalOnly) + "/DESCRIPTION.txt");
        ad->setChangeLogFile(BDirTools::findResource("changelog", BDirTools::GlobalOnly) + "/ChangeLog.txt");
        ad->setLicenseFile(BDirTools::findResource("copying", BDirTools::GlobalOnly) + "/COPYING.txt");
        ad->setAuthorsFile( BDirTools::findResource("infos/authors.beqt-info", BDirTools::GlobalOnly) );
        ad->setTranslatorsFile( BDirTools::findResource("infos/translators.beqt-info", BDirTools::GlobalOnly) );
        ad->setThanksToFile( BDirTools::findResource("infos/thanks-to.beqt-info", BDirTools::GlobalOnly) );
        BDirTools::createUserLocations(QStringList() << "autotext" << "klm" << "macros" << "texsample");
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
