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
//#include <QListWidget>

static QString resource(const QString &subpath)
{
    return BDirTools::findResource(subpath, BDirTools::GlobalOnly);
}

int main(int argc, char *argv[])
{
    tInit();
    QApplication app(argc, argv);
    QApplication::setApplicationName("TeX Creator");
    QApplication::setApplicationVersion("2.1.2");
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
        Application bapp;
        Q_UNUSED(bapp)
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
        Application::setApplicationCopyrightPeriod("2012-2013");
        Application::setApplicationDescriptionFile(resource("description") + "/DESCRIPTION.txt");
        Application::setApplicationChangeLogFile(resource("changelog") + "/ChangeLog.txt");
        Application::setApplicationLicenseFile(resource("copying") + "/COPYING.txt");
        Application::setApplicationAuthorsFile(resource("infos/authors.beqt-info"));
        Application::setApplicationTranslationsFile(resource("infos/translators.beqt-info"));
        Application::setApplicationThanksToFile(resource("infos/thanks-to.beqt-info"));
        ad->setupWithApplicationData();
        BDirTools::createUserLocations(QStringList() << "autotext" << "klm" << "macros" << "texsample");
        Application::createInitialWindow(args);
        Application::loadSettings();
        /*QDialog *dlg = new QDialog;
        QListWidget *l = new QListWidget;
        QListWidgetItem *i = new QListWidgetItem;
        i->setFlags(i->flags () | Qt::ItemIsEditable);
        l->addItem(i);
        l->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked);
        l->show();
        QObject::connect(l, SIGNAL(itemChanged(QListWidgetItem*)), dlg, SLOT(exec()));*/
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
        if ( args.isEmpty() )
            args << "";
        s.sendMessage(args);
    }
    tCleanup();
    return ret;
}
