#include "consolewidget.h"
#include "application.h"
#include "consolesettingstab.h"
#include "maindocumenteditormodule.h"
#include "client.h"
#include "application.h"

#include <BApplication>
#include <BTerminalWidget>
#include <BAbstractTerminalDriver>
#include <BLocalTerminalDriver>
#include <BCodeEditor>
#include <BCodeEditorDocument>
#include <BPlainTextEdit>
#include <BSettingsDialog>
#include <BDirTools>

#include <QWidget>
#include <QProcess>
#include <QVBoxLayout>
#include <QToolBar>
#include <QSize>
#include <QAction>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QComboBox>
#include <QStringList>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QString>
#include <QVariant>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QList>
#include <QEvent>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QBrush>
#include <QFont>
#include <QList>
#include <QMap>
#include <QKeySequence>
#include <QSignalMapper>
#include <QCheckBox>
#include <QSettings>
#include <QKeyEvent>
#include <QApplication>
#include <QScopedPointer>
#include <QCursor>
#include <QPoint>
#include <QDialog>
#include <QPushButton>
#include <QVariantMap>
#include <QByteArray>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

#include <QDebug>

/*============================================================================
================================ ConsoleWidget ===============================
============================================================================*/

/*============================== Public constructors =======================*/

ConsoleWidget::ConsoleWidget(BCodeEditor *cedtr, QWidget *parent) :
    QWidget(parent)
{
    mcedtr = cedtr;
    mopen = false;
    mremote = false;
    mlocalDriver = new BLocalTerminalDriver(this);
    mremoteDriver = 0; //TODO: Initialize
    if (cedtr)
        connect( cedtr, SIGNAL( currentDocumentChanged(BCodeEditorDocument *) ),
                 this, SLOT( checkActions(BCodeEditorDocument *) ) );
    mmprActions = new QSignalMapper(this);
    connect( mmprActions, SIGNAL( mapped(int) ), this, SLOT( performAction(int) ) );
    initKeyMap();
    initGui();
    connect( bApp, SIGNAL( languageChanged() ), this, SLOT( retranslateUi() ) );
    retranslateUi();
}

/*============================== Public methods ============================*/

bool ConsoleWidget::eventFilter(QObject *object, QEvent *event)
{
    if (!ConsoleSettingsTab::getAlwaysLatinEnabled() || event->type() != QEvent::KeyPress)
        return QWidget::eventFilter(object, event);
    QKeyEvent *ke = static_cast<QKeyEvent *>(event);
    int key = ke->key();
    if ( !mkeyMap.contains(key) )
        return false;
    QString text = ke->text();
    if ( text.isEmpty() || !text.at(0).isPrint() )
        return false;
    int keyn = mkeyMap.value(key);
    QChar c(keyn);
    if ( text.at(0).isLower() )
        c = c.toLower();
    text = QString(c);
    QKeyEvent ken(QEvent::KeyPress, keyn, ke->modifiers(), text);
    QApplication::sendEvent(object, &ken);
    return true;
}

QAction *ConsoleWidget::consoleAction(Action actId) const
{
    return mactMap.value(actId);
}

QList<QAction *> ConsoleWidget::consoleActions(bool withSeparators) const
{
    QList<QAction *> list;
    list << consoleAction(ClearAction);
    if (withSeparators)
        list << Application::createSeparator();
    list << consoleAction(CompileAction);
    list << consoleAction(CompileAndOpenAction);
    if (withSeparators)
        list << Application::createSeparator();
    list << consoleAction(OpenPdfAction);
    list << consoleAction(OpenPsAction);
    if (withSeparators)
        list << Application::createSeparator();
    list << consoleAction(SettingsAction);
    return list;
}

/*============================== Static private methods ====================*/

QString ConsoleWidget::fileNameNoSuffix(const QString &fileName)
{
    int nlen = fileName.length();
    int slen = QFileInfo(fileName).suffix().length();
    return slen ? fileName.left(nlen - slen - 1) : fileName;
}

QList<ConsoleWidget::File> ConsoleWidget::dependencies(const QString &fileText, BCodeEditor *editor, bool *ok)
{
    if (ok)
        *ok = true;
    QList<ConsoleWidget::File> list;
    if (fileText.isEmpty())
        return list;
    QStringList fns;
    static const QString input = "((?<=\\\\input )(.*))";
    static const QString includegraphics = "((?<=\\includegraphics)(.*)(?=\\}))";
    static QRegularExpression rx("(" + input + "|" + includegraphics + ")");
    static QRegExp rx2("^(\\[.*\\])?\\{");
    QRegularExpressionMatchIterator i = rx.globalMatch(fileText);
    while ( i.hasNext() )
        fns << i.next().capturedTexts();
    fns.removeAll("");
    if ( !fns.isEmpty() )
    {
        foreach ( int i, bRange(0, fns.size() - 1) )
        {
            fns[i].remove(rx2);
            if (fns.at(i).right(1) == "\\")
                fns[i].remove(fns.at(i).length() - 1, 1);
            if (fns.at(i).at(0) == '\"')
                fns[i].remove(0, 1);
            if (fns.at(i).at(fns.at(i).length() - 1) == '\"')
                fns[i].remove(fns.at(i).length() - 1, 1);
        }
    }
    fns.removeDuplicates();
    return list;
}

/*============================== Private methods ===========================*/

void ConsoleWidget::initKeyMap()
{
    mkeyMap.insert(1040, 'F');
    mkeyMap.insert(1042, 'D');
    mkeyMap.insert(1043, 'U');
    mkeyMap.insert(1044, 'L');
    mkeyMap.insert(1045, 'T');
    mkeyMap.insert(1047, 'P');
    mkeyMap.insert(1048, 'B');
    mkeyMap.insert(1049, 'Q');
    mkeyMap.insert(1050, 'R');
    mkeyMap.insert(1051, 'K');
    mkeyMap.insert(1052, 'V');
    mkeyMap.insert(1053, 'Y');
    mkeyMap.insert(1054, 'J');
    mkeyMap.insert(1055, 'G');
    mkeyMap.insert(1056, 'H');
    mkeyMap.insert(1057, 'C');
    mkeyMap.insert(1058, 'N');
    mkeyMap.insert(1059, 'E');
    mkeyMap.insert(1060, 'A');
    mkeyMap.insert(1062, 'W');
    mkeyMap.insert(1063, 'X');
    mkeyMap.insert(1064, 'I');
    mkeyMap.insert(1065, 'O');
    mkeyMap.insert(1067, 'S');
    mkeyMap.insert(1068, 'M');
    mkeyMap.insert(1071, 'Z');
}

void ConsoleWidget::initGui()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      mtbar = new QToolBar(this);
        createAction(ClearAction, "editclear", "", true);
        mtbar->addSeparator();
        createAction(CompileAction, "compfile", "Ctrl+B");
        createAction(CompileAndOpenAction, "run_build", "Ctrl+R");
        mtbar->addSeparator();
        createAction(OpenPdfAction, "pdf");
        createAction(OpenPsAction, "postscript");
        mtbar->addSeparator();
        createAction(SettingsAction, "configure", "", true);
      vlt->addWidget(mtbar);
      checkActions(mcedtr ? mcedtr->currentDocument() : 0);
      mtermwgt = new BTerminalWidget(BTerminalWidget::ProgrammaticMode, this);
        mtermwgt->findChild<BPlainTextEdit *>()->installEventFilter(this);
      connect( mtermwgt, SIGNAL( finished(int) ), this, SLOT( finished(int) ) );
      vlt->addWidget(mtermwgt);
}

QAction *ConsoleWidget::createAction(int id, const QString &iconFileName, const QString &shortcut, bool enabled)
{
    if ( id < 0 || mactMap.contains(id) )
        return 0;
    QAction *act = new QAction(this);
    act->setEnabled(enabled);
    if ( !iconFileName.isEmpty() )
        act->setIcon( BApplication::icon(iconFileName) );
    if ( !shortcut.isEmpty() )
    {
        act->setShortcut( QKeySequence(shortcut) );
        act->setShortcutContext(Qt::WindowShortcut);
    }
    mmprActions->setMapping(act, id);
    connect( act, SIGNAL( triggered() ), mmprActions, SLOT( map() ) );
    mactMap.insert(id, act);
    mtbar->addAction(act);
    return act;
}

void ConsoleWidget::compile(bool op)
{
    if (mtermwgt->isActive() || !mcedtr)
        return;
    MainDocumentEditorModule *mdmdl = static_cast<MainDocumentEditorModule *>( mcedtr->module("main_document") );
    if ( mdmdl->mainDocument() )
    {
        if ( !mcedtr->saveAllDocuments() )
            return;
    }
    else
    {
        if ( !mcedtr->saveCurrentDocument() )
            return;
    }
    BCodeEditorDocument *doc = mdmdl->mainDocument() ? mdmdl->mainDocument() : mcedtr->currentDocument();
    if (!doc)
        return noFileNameError();
    mfileName = doc->fileName();
    QFileInfo fi(mfileName);
    if ( !fi.exists() || !fi.isFile() )
        return mtermwgt->appendLine(tr("File does not exist", "termwgt text") + "\n", BTerminalWidget::CriticalFormat);
    QString cmd = ConsoleSettingsTab::getCompilerName();
    mopen = op && cmd.contains("pdf");
    mremote = ConsoleSettingsTab::getUseRemoteCompiler() && sClient->isAuthorized();
    mtermwgt->setDriver(mremote ? mremoteDriver : mlocalDriver);
    if (mremote)
    {
        bool makeindex = ConsoleSettingsTab::getMakeindexEnabled();
        bool dvips = ConsoleSettingsTab::getDvipsEnabled();
        QVariantMap m;
        QString text = doc->text();
        if (text.isEmpty())
        {
            //TODO: Show message
            return;
        }
        m.insert("file_name", QFileInfo(mfileName).fileName());
        m.insert("text", text);
        bool ok = false;
        QList<File> deps = dependencies(text, mcedtr, &ok);
        //
        return; //TODO
        //
        QVariantList vl;
        foreach (const File &f, deps)
        {
            QVariantMap m;
            m.insert("file_name", QFileInfo(f.fileName).fileName());
            m.insert("data", f.data);
            vl << m;
        }
        m.insert("aux_files", m);
        m.insert("compiler", cmd);
        m.insert("makeindex", ConsoleSettingsTab::getMakeindexEnabled());
        m.insert("dvips", ConsoleSettingsTab::getDvipsEnabled());
        m.insert("options", ConsoleSettingsTab::getCompilerOptions());
        m.insert("commands", ConsoleSettingsTab::getCompilerCommands());
        mtermwgt->appendLine(tr("Starting remote compilation", "termwgt text") + " (" + cmd
                             + (makeindex ? "+makeindex" : "") + (dvips ? "+dvips" : "") + ") "
                             + tr("for", "termwgt text") + " " + mfileName + "...", BTerminalWidget::MessageFormat);
        mtermwgt->terminalCommand(m);
    }
    else
    {
        QStringList args;
        args << ConsoleSettingsTab::getCompilerOptions();
        args << ("\"" + mfileName + "\"");
        args << ConsoleSettingsTab::getCompilerCommands();
        start(cmd, args);
    }
    setUiEnabled(!mtermwgt->isActive());
}

void ConsoleWidget::open(bool pdf)
{
    QString fn = mfileName;
    mfileName.clear();
    MainDocumentEditorModule *mdmdl = static_cast<MainDocumentEditorModule *>( mcedtr->module("main_document") );
    if ( fn.isEmpty() )
        fn = mdmdl->mainDocument() ? mdmdl->mainDocumentFileName() : mcedtr->currentDocumentFileName();
    if ( fn.isEmpty() )
        return noFileNameError();
    QString fns = fileNameNoSuffix(fn) + (pdf ? ".pdf" : ".ps");
    QFileInfo fi(fns);
    if ( !fi.exists() || !fi.isFile() || !QDesktopServices::openUrl( QUrl::fromLocalFile(fns) ) )
        mtermwgt->appendLine(tr("Failed to open", "termwgt text") + " " + fns + "\n", BTerminalWidget::CriticalFormat);
}

void ConsoleWidget::start(const QString &command, const QStringList &args)
{
    mcommand = command;
    mtermwgt->setWorkingDirectory( QFileInfo(mfileName).path() );
    mtermwgt->appendLine(tr("Executing", "termwgt text") + " " + command + " " +
                         tr("for", "termwgt text") + " " + mfileName + "...", BTerminalWidget::MessageFormat);
    mtermwgt->terminalCommand(command, args);
}

void ConsoleWidget::start(const QString &command, const QString &arg)
{
    start(command, QStringList() << arg);
}

void ConsoleWidget::noFileNameError()
{
    mtermwgt->appendLine(tr("File name is not specified", "termwgt text") + "\n", BTerminalWidget::MessageFormat);
}

void ConsoleWidget::showSettings()
{
    BSettingsDialog sd( new ConsoleSettingsTab, window() );
    sd.move( QCursor::pos() - QPoint(100, 150) );
    sd.exec();
}

void ConsoleWidget::setUiEnabled(bool b)
{
    mtbar->setEnabled(b);
    if (b)
        checkActions(mcedtr ? mcedtr->currentDocument() : 0);
}

/*============================== Private slots =============================*/

void ConsoleWidget::retranslateUi()
{
    consoleAction(ClearAction)->setText( tr("Clear console", "action text") );
    consoleAction(ClearAction)->setToolTip( tr("Clear console", "action toolTip") );
    consoleAction(ClearAction)->setWhatsThis( tr("Use this action to clear the contents of console",
                                                 "act whatsThis") );
    consoleAction(CompileAction)->setText( tr("Compile", "action text") );
    QString sc = consoleAction(CompileAction)->shortcut().toString(QKeySequence::NativeText);
    consoleAction(CompileAction)->setToolTip(tr("Compile current document", "action toolTip") + " (" + sc + ")");
    consoleAction(CompileAction)->setWhatsThis( tr("Use this action to compile current document into "
                                                   ".pdf, .ps, or .dvi file", "act whatsThis") );
    consoleAction(CompileAndOpenAction)->setText( tr("Compile and open", "action text") );
    sc = consoleAction(CompileAndOpenAction)->shortcut().toString(QKeySequence::NativeText);
    consoleAction(CompileAndOpenAction)->setToolTip(tr("Compile and open current document",
                                                       "action toolTip") + " (" + sc + ")");
    consoleAction(CompileAndOpenAction)->setWhatsThis( tr("Use this action to compile current document "
                                                          "and then open the produced file", "act whatsThis") );
    consoleAction(OpenPdfAction)->setText( tr("Show PDF", "action text") );
    consoleAction(OpenPdfAction)->setToolTip( tr("Show current document using default PDF reader", "action toolTip") );
    consoleAction(OpenPdfAction)->setWhatsThis( tr("Use this action to open the .pdf file corresponding "
                                                   "to the current document", "act whatsThis") );
    consoleAction(OpenPsAction)->setText( tr("Show PS", "action text") );
    consoleAction(OpenPsAction)->setToolTip( tr("Show current document using default PS reader", "action toolTip") );
    consoleAction(OpenPsAction)->setWhatsThis( tr("Use this action to open the .ps file corresponding "
                                                  "to the current document", "act whatsThis") );
    consoleAction(SettingsAction)->setText( tr("Console settings...", "action text") );
    consoleAction(SettingsAction)->setToolTip( tr("Configure console", "action toolTip") );
}

void ConsoleWidget::performAction(int actId)
{
    switch (actId)
    {
    case ClearAction:
        mtermwgt->clearEdit();
        break;
    case CompileAction:
        compile();
        break;
    case CompileAndOpenAction:
        compile(true);
        break;
    case OpenPdfAction:
        open(true);
        break;
    case OpenPsAction:
        open(false);
        break;
    case SettingsAction:
        showSettings();
        break;
    default:
        break;
    }
}

void ConsoleWidget::checkActions(BCodeEditorDocument *doc)
{
    QString fnns = fileNameNoSuffix( doc ? doc->fileName() : QString() );
    consoleAction(CompileAction)->setEnabled(doc);
    consoleAction(CompileAndOpenAction)->setEnabled(doc);
    QFileInfo fi(fnns + ".pdf");
    consoleAction(OpenPdfAction)->setEnabled( fi.exists() && fi.isFile() );
    fi.setFile(fnns + ".ps");
    consoleAction(OpenPsAction)->setEnabled( fi.exists() && fi.isFile() );
}

void ConsoleWidget::finished(int exitCode)
{
    if (mremote)
        return mtermwgt->appendLine(tr("Remote compilation finished with code", "termwgt text") + " "
                                    + QString::number(exitCode) + "\n", BTerminalWidget::MessageFormat);
    mtermwgt->appendLine(mcommand + " " + tr("finished with code", "termwgt text") + " " +
                         QString::number(exitCode) + "\n", BTerminalWidget::MessageFormat);
    if ("makeindex" == mcommand)
    {
        if ( ConsoleSettingsTab::getDvipsEnabled() )
        {
            start( "dvips",  fileNameNoSuffix(mfileName) );
        }
        else
        {
            setUiEnabled(true);
            if (mopen && !exitCode)
                open();
        }
    }
    else if ("dvips" == mcommand)
    {
        setUiEnabled(true);
        if (mopen && !exitCode)
            open();
    }
    else
    {
        if ( ConsoleSettingsTab::getMakeindexEnabled() )
        {
            start( "makeindex", fileNameNoSuffix(mfileName) );
        }
        else if ( ConsoleSettingsTab::getDvipsEnabled() )
        {
            start( "dvips", fileNameNoSuffix(mfileName) );
        }
        else
        {
            setUiEnabled(true);
            if (mopen && !exitCode)
                open();
        }
    }
}
