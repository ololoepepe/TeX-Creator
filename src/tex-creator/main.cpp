#include "applicationserver.h"
#include "mainwindow.h"
#include "application.h"
#include "cache.h"
#include "global.h"
#include "client.h"

#include <TeXSampleGlobal>
#include <TOperationResult>

#include <BApplication>
#include <BDirTools>
#include <BTranslator>
#include <BLogger>
#include <BAboutDialog>
#include <BVersion>

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
#include <QMetaType>
#include <QSettings>

#include <QDebug>

Q_DECLARE_METATYPE(QTextCodec *)

static QString resource(const QString &subpath)
{
    return BDirTools::findResource(subpath, BDirTools::GlobalOnly);
}

int main(int argc, char *argv[])
{
    qRegisterMetaType<QTextCodec *>();
    tInit();
    QApplication app(argc, argv);
    QApplication::setApplicationName("TeX Creator");
    QApplication::setApplicationVersion("3.4.3-beta");
    QApplication::setOrganizationName("TeXSample Team");
    QApplication::setOrganizationDomain("https://github.com/TeXSample-Team/TeX-Creator");
    QFont fnt = QApplication::font();
    fnt.setPointSize(10);
    QApplication::setFont(fnt);
    QStringList args = app.arguments();
    args.removeFirst();
    args.removeDuplicates();
    QString home = QDir::home().dirName();
    ApplicationServer s(9950 + qHash(home) % 10, QCoreApplication::applicationName() + "3" + home);
    int ret = 0;
    if (!s.testServer())
    {
        s.listen();
#if defined(BUILTIN_RESOURCES)
        Q_INIT_RESOURCE(tex_creator);
        Q_INIT_RESOURCE(tex_creator_doc);
        Q_INIT_RESOURCE(tex_creator_symbols);
        Q_INIT_RESOURCE(tex_creator_translations);
#endif
        Application bapp;
        Application::resetProxy();
        Q_UNUSED(bapp)
        //Compatibility
        if (bSettings->value("Global/version").value<BVersion>() < BVersion("3.0.0-pa"))
        {
            bSettings->remove("General/CodeEditor");
            bSettings->remove("BeQt/Core/deactivated_plugins");
            bSettings->remove("CodeEditor/document_driver_state");
            bSettings->remove("CodeEditor/search_moudle_state");
            bSettings->remove("CodeEditor/edit_font_family");
            bSettings->remove("CodeEditor/font_family");
            bSettings->remove("CodeEditor/font_point_size");
            bSettings->remove("SamplesWidget");
            bSettings->remove("TeXSample");
            bSettings->remove("editor");
            bSettings->remove("main_window");
            bSettings->remove("TexsampleWidget/table_header_state");
            bSettings->remove("TexsampleWidget/add_sample_dialog_size");
            bSettings->remove("TexsampleWidget/edit_sample_dialog_size");
            bSettings->remove("Console/compiler_commands");
            bSettings->remove("Console/compiler_name");
            bSettings->remove("Console/compiler_options");
            bSettings->remove("Console/dvips_enabled");
            bSettings->remove("Console/makeindex_enabled");
            sCache->clear();
        }
        if (bSettings->value("Global/version").value<BVersion>() < BVersion("3.4.0-beta"))
        {
            bSettings->remove("Macros/ExternalTools");
        }
        bSettings->setValue("Global/version", BVersion(QCoreApplication::applicationVersion()));
        Application::setThemedIconsEnabled(false);
        Application::setPreferredIconFormats(QStringList() << "png");
        QIcon icn = Application::icon("tex");
        QApplication::setWindowIcon(icn);
        Application::installTranslator(new BTranslator("qt"));
        Application::installTranslator(new BTranslator("beqt"));
        Application::installTranslator(new BTranslator("texsample"));
        Application::installTranslator(new BTranslator("tex-creator"));
        BAboutDialog::setDefaultMinimumSize(800, 400);
        Application::setApplicationCopyrightPeriod("2012-2014");
        Application::setApplicationDescriptionFile(resource("description") + "/DESCRIPTION.txt");
        Application::setApplicationChangeLogFile(resource("changelog") + "/ChangeLog.txt");
        Application::setApplicationLicenseFile(resource("copying") + "/COPYING.txt");
        Application::setApplicationAuthorsFile(resource("infos/authors.beqt-info"));
        Application::setApplicationTranslationsFile(resource("infos/translators.beqt-info"));
        Application::setApplicationThanksToFile(resource("infos/thanks-to.beqt-info"));
        Application::aboutDialogInstance()->setupWithApplicationData();
        BDirTools::createUserLocations(QStringList() << "autotext" << "klm" << "macros" << "texsample");
        Application::createInitialWindow(args);
        Application::loadSettings();
        Application::loadPlugins(QStringList() << "editor-module");
        if (Global::checkForNewVersions())
            Application::checkForNewVersions();
        ret = app.exec();
        Application::saveSettings();
#if defined(BUILTIN_RESOURCES)
        Q_CLEANUP_RESOURCE(tex_creator);
        Q_CLEANUP_RESOURCE(tex_creator_doc);
        Q_CLEANUP_RESOURCE(tex_creator_symbols);
        Q_CLEANUP_RESOURCE(tex_creator_translations);
#endif
    }
    else
    {
        if (args.isEmpty())
            args << "";
        s.sendMessage(args);
    }
    tCleanup();
    return ret;
}
