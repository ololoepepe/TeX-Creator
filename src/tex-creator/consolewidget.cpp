/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
**
** This file is part of TeX Creator.
**
** TeX Creator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** TeX Creator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with TeX Creator.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "consolewidget.h"

#include "application.h"
#include "maindocumenteditormodule.h"
#include "settings.h"
#include "texsample/remoteterminaldriver.h"
#include "texsample/texsamplecore.h"

#include <TNetworkClient>
#include <TTexCompiler>

#include <BAbstractCodeEditorDocument>
#include <BAbstractTerminalDriver>
#include <BCodeEditor>
#include <BGuiTools>
#include <BLocalTerminalDriver>
#include <BPlainTextEdit>
#include <BTerminalWidget>

#include <QAction>
#include <QBrush>
#include <QComboBox>
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>
#include <QRegExp>
#include <QShortcut>
#include <QSignalMapper>
#include <QString>
#include <QStringList>
#include <QToolBar>
#include <QVariant>
#include <QVariantMap>
#include <QVBoxLayout>
#include <QWidget>

/*============================================================================
================================ ConsoleWidget ===============================
============================================================================*/

/*============================== Public constructors =======================*/

ConsoleWidget::ConsoleWidget(BCodeEditor *cedtr, QWidget *parent) :
    QWidget(parent)
{
    malwaysLatin = Settings::Console::alwaysLatinEnabled();
    mcedtr = cedtr;
    mmakeindex = false;
    mdvips = false;
    mopen = false;
    mremote = false;
    if (cedtr) {
        connect(cedtr, SIGNAL(currentDocumentChanged(BAbstractCodeEditorDocument *)),
                this, SLOT(checkActions(BAbstractCodeEditorDocument *)));
        MainDocumentEditorModule *mdmdl = static_cast<MainDocumentEditorModule *>(mcedtr->module("main_document"));
        connect(mdmdl, SIGNAL(mainDocumentChanged(BAbstractCodeEditorDocument *)),
                this, SLOT(checkActions(BAbstractCodeEditorDocument *)));
    }
    mmprActions = new QSignalMapper(this);
    connect(mmprActions, SIGNAL(mapped(int)), this, SLOT(performAction(int)));
    //
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
    createAction(SwitchCompilerAction, "", "", true);
    updateSwitchCompilerAction();
    createAction(SettingsAction, "configure", "", true);
    vlt->addWidget(mtbar);
    checkActions(mcedtr ? mcedtr->currentDocument() : 0);
    mtermwgt = new BTerminalWidget(BTerminalWidget::ProgrammaticMode, this);
    mtermwgt->innerEdit()->installEventFilter(this);
    connect(mtermwgt, SIGNAL(finished(int)), this, SLOT(finished(int)));
    vlt->addWidget(mtermwgt);
    QSignalMapper *mpr = new QSignalMapper(this);
    connect(mpr, SIGNAL(mapped(int)), this, SLOT(inputLatinKey(int)));
    foreach (int i, bRangeD('A', 'Z')) {
        QString s = "Shift+" + QString(QChar(i));
        QShortcut *sc = new QShortcut(QKeySequence(s), mtermwgt);
        sc->setContext(Qt::WidgetWithChildrenShortcut);
        bSetMapping(mpr, sc, SIGNAL(activated()), i);
        s = QString(QChar(i));
        sc = new QShortcut(QKeySequence(s), mtermwgt);
        sc->setContext(Qt::WidgetWithChildrenShortcut);
        bSetMapping(mpr, sc, SIGNAL(activated()), i);
    }
    //
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
    retranslateUi();
}

/*============================== Public methods ============================*/

bool ConsoleWidget::eventFilter(QObject *, QEvent *event)
{
    if (!malwaysLatin || event->type() != QEvent::ShortcutOverride)
        return false;
    mmodifiers = static_cast<QKeyEvent *>(event)->modifiers();
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
        list << BGuiTools::createSeparator();
    list << consoleAction(CompileAction);
    list << consoleAction(CompileAndOpenAction);
    if (withSeparators)
        list << BGuiTools::createSeparator();
    list << consoleAction(OpenPdfAction);
    list << consoleAction(OpenPsAction);
    if (withSeparators)
        list << BGuiTools::createSeparator();
    list << consoleAction(SwitchCompilerAction);
    list << consoleAction(SettingsAction);
    return list;
}

/*============================== Public slots ==============================*/

void ConsoleWidget::updateSwitchCompilerAction()
{
    malwaysLatin = Settings::Console::alwaysLatinEnabled();
    QAction *act = consoleAction(SwitchCompilerAction);
    act->setIcon(Application::icon(Settings::Console::useRemoteCompiler() ? "remote" : "local"));
    act->setText(Settings::Console::useRemoteCompiler() ? tr("Compiler: remote", "action text") :
                                                          tr("Compiler: local", "action text"));
    act->setToolTip(Settings::Console::useRemoteCompiler() ? tr("Using remote compiler", "action toolTip") :
                                                             tr("Using local compiler", "action toolTip"));
}

/*============================== Static private methods ====================*/

QString ConsoleWidget::fileNameNoSuffix(const QString &fileName)
{
    int nlen = fileName.length();
    int slen = QFileInfo(fileName).suffix().length();
    return slen ? fileName.left(nlen - slen - 1) : fileName;
}

/*============================== Private methods ===========================*/

void ConsoleWidget::compile(bool op)
{
    if (mtermwgt->isActive() || !mcedtr)
        return;
    MainDocumentEditorModule *mdmdl = static_cast<MainDocumentEditorModule *>(mcedtr->module("main_document"));
    if (mdmdl->mainDocument()) {
        if (!mcedtr->saveAllDocuments())
            return;
    } else {
        if (!mcedtr->saveCurrentDocument())
            return;
    }
    if (mcedtr->isBuisy()) {
        setUiEnabled(false);
        if (!mcedtr->waitForAllDocumentsProcessed())
            return setUiEnabled(true);
        setUiEnabled(true);
    }
    BAbstractCodeEditorDocument *doc = mdmdl->mainDocument() ? mdmdl->mainDocument() : mcedtr->currentDocument();
    if (!doc)
        return noFileNameError();
    mfileName = doc->fileName();
    QFileInfo fi(mfileName);
    if (!fi.exists() || !fi.isFile())
        return mtermwgt->appendLine(tr("File does not exist", "termwgt text") + "\n", BTerminalWidget::CriticalFormat);
    bool rem = Settings::Console::useRemoteCompiler();
    TNetworkClient *client = tSmp->client();
    if (rem && !client->isAuthorized()) {
        mtermwgt->appendLine(tr("You are not connected to TeXSample, will now try to connect...", "termwgt text"),
                             BTerminalWidget::MessageFormat);
        if (!client->isValid())
            bApp->showSettings(Application::TexsampleSettings);
        if (client->isValid())
            client->connectToServer();
        if (client->state() != TNetworkClient::DisconnectedState)
            BeQt::waitNonBlocking(client, SIGNAL(stateChanged(TNetworkClient::State)), 10 * BeQt::Second);
    }
    if (rem && !client->isAuthorized()) {
        if (Settings::Console::hasFallbackToLocalCompiler()) {
            if (!Settings::Console::fallbackToLocalCompiler())
                return mtermwgt->appendLine(tr("Unable to start remote compiler", "termwgt text"),
                                            BTerminalWidget::CriticalFormat);
        } else {
            QMessageBox msg(window());
            msg.setWindowTitle(tr("No TeXSample connection", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Warning);
            msg.setText(tr("You are going to use remote compiler, but you are not connected to TeXSample service",
                           "msgbox text"));
            msg.setInformativeText(tr("Do you want to use local compiler then?", "msgbox informativeText"));
            msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msg.setDefaultButton(QMessageBox::Yes);
            QPushButton *btn1 = msg.addButton(tr("Yes, always", "btn text"), QMessageBox::AcceptRole);
            QPushButton *btn2 = msg.addButton(tr("No, never", "btn text"), QMessageBox::RejectRole);
            if (msg.exec() == QMessageBox::No)
                return;
            if (msg.clickedButton() == btn1) {
                Settings::Console::setFallbackToLocalCompiler(true);
            } else if (msg.clickedButton() == btn2)
            {
                Settings::Console::setFallbackToLocalCompiler(false);
                return mtermwgt->appendLine(tr("Unable to start remote compiler", "termwgt text"),
                                            BTerminalWidget::CriticalFormat);;
            }
        }
    }
    mremote = rem && client->isAuthorized();
    if (rem != mremote) {
        mtermwgt->appendLine(tr("Remote compiler is not available, will use local compiler", "termwgt text"),
                             BTerminalWidget::WarningFormat);
    }
    QString cmd = Settings::Compiler::compiler().toString().toLower();
    mopen = op && cmd.contains("pdf");
    mtermwgt->setDriver(mremote ? (BAbstractTerminalDriver *) new RemoteTerminalDriver :
                                  (BAbstractTerminalDriver *) new BLocalTerminalDriver);
    setUiEnabled(false);
    //NOTE: Makeindex may only be used on documents which contain "\makeindex" command
    //The command may be in one of the included files, so we also check for "\input texsample.tex"
    mmakeindex = Settings::Compiler::makeindexEnabled();
    bool mi = false;
    if (mmakeindex) {
        QString makeindexPattern = "^\\s*\\\\makeindex\\s*$";
        QString texsamplePattern = QString("^\\s*\\\\include\\s+(%1|\"%1\")\\s*$").arg("texsample\\.tex");
        QString text = doc->text();
        if (text.contains(QRegExp(makeindexPattern)))
            mi = true;
        if (!mi && text.contains(QRegExp(texsamplePattern)))
            mi = true;
    }
    mmakeindex = mmakeindex && mi;
    //NOTE: Only "tex" and "latex" commands produce .dvi files
    mdvips = Settings::Compiler::dvipsEnabled() && !cmd.contains("pdf");
    if (mremote) {
        QVariantMap m;
        m.insert("file_name", mfileName);
        m.insert("codec_name", doc->codecName());
        mtermwgt->appendLine(tr("Starting remote compilation", "termwgt text") + " (" + cmd
                             + (Settings::Compiler::makeindexEnabled() ? "+makeindex" : "")
                             + (Settings::Compiler::dvipsEnabled() ? "+dvips" : "") + ") "
                             + tr("for", "termwgt text") + " " + mfileName + "...", BTerminalWidget::MessageFormat);
        mtermwgt->terminalCommand(m);
    } else {
        QStringList args;
        args << Settings::Compiler::compilerOptions();
        args << "\"" + mfileName + "\"";
        args << Settings::Compiler::compilerCommands();
        start(cmd, args);
    }
    setUiEnabled(!mtermwgt->isActive());
}

QAction *ConsoleWidget::createAction(int id, const QString &iconFileName, const QString &shortcut, bool enabled)
{
    if (id < 0 || mactMap.contains(id))
        return 0;
    QAction *act = new QAction(this);
    act->setEnabled(enabled);
    if (!iconFileName.isEmpty())
        act->setIcon(Application::icon(iconFileName));
    if (!shortcut.isEmpty()) {
        act->setShortcut(QKeySequence(shortcut));
        act->setShortcutContext(Qt::WindowShortcut);
    }
    mmprActions->setMapping(act, id);
    connect(act, SIGNAL(triggered()), mmprActions, SLOT(map()));
    mactMap.insert(id, act);
    mtbar->addAction(act);
    return act;
}

void ConsoleWidget::noFileNameError()
{
    mtermwgt->appendLine(tr("File name is not specified", "termwgt text") + "\n", BTerminalWidget::MessageFormat);
}

void ConsoleWidget::open(bool pdf)
{
    QString fn = mfileName;
    mfileName.clear();
    MainDocumentEditorModule *mdmdl = static_cast<MainDocumentEditorModule *>(mcedtr->module("main_document"));
    if (fn.isEmpty())
        fn = mdmdl->mainDocument() ? mdmdl->mainDocumentFileName() : mcedtr->currentDocumentFileName();
    if (fn.isEmpty())
        return noFileNameError();
    QString fns = fileNameNoSuffix(fn) + (pdf ? ".pdf" : ".ps");
    QFileInfo fi(fns);
    if (!fi.exists() || !fi.isFile() || !bApp->openLocalFile(fns))
        mtermwgt->appendLine(tr("Failed to open", "termwgt text") + " " + fns + "\n", BTerminalWidget::CriticalFormat);
}

void ConsoleWidget::setUiEnabled(bool b)
{
    mtbar->setEnabled(b);
    if (b)
        checkActions(mcedtr ? mcedtr->currentDocument() : 0);
}

void ConsoleWidget::start(const QString &command, const QStringList &args)
{
    mcommand = command;
    mtermwgt->setWorkingDirectory(QFileInfo(mfileName).path());
    mtermwgt->appendLine(tr("Executing", "termwgt text") + " " + command + " " +
                         tr("for", "termwgt text") + " " + mfileName + "...", BTerminalWidget::MessageFormat);
    mtermwgt->terminalCommand(command, args);
}

void ConsoleWidget::start(const QString &command, const QString &arg)
{
    start(command, QStringList() << arg);
}

/*============================== Private slots =============================*/

void ConsoleWidget::checkActions(BAbstractCodeEditorDocument *doc)
{
    MainDocumentEditorModule *mdmdl = static_cast<MainDocumentEditorModule *>(mcedtr->module("main_document"));
    if (mdmdl->mainDocument())
        doc = mdmdl->mainDocument();
    QString fnns = fileNameNoSuffix(doc ? doc->fileName() : QString());
    consoleAction(CompileAction)->setEnabled(doc);
    consoleAction(CompileAndOpenAction)->setEnabled(doc);
    QFileInfo fi(fnns + ".pdf");
    consoleAction(OpenPdfAction)->setEnabled(fi.exists() && fi.isFile());
    fi.setFile(fnns + ".ps");
    consoleAction(OpenPsAction)->setEnabled(fi.exists() && fi.isFile());
}

void ConsoleWidget::finished(int exitCode)
{
    if (mremote) {
        mtermwgt->appendLine(tr("Remote compilation finished with code", "termwgt text") + " "
                             + QString::number(exitCode) + "\n", BTerminalWidget::MessageFormat);
        if (mopen && !exitCode)
            open();
    } else {
        mtermwgt->appendLine(mcommand + " " + tr("finished with code", "termwgt text") + " " +
                             QString::number(exitCode) + "\n", BTerminalWidget::MessageFormat);
        if ("makeindex" == mcommand) {
            if (mdvips) {
                start("dvips", fileNameNoSuffix(mfileName));
            } else {
                setUiEnabled(true);
                if (mopen && !exitCode)
                    open();
            }
        } else if ("dvips" == mcommand) {
            setUiEnabled(true);
            if (mopen && !exitCode)
                open();
        } else {
            if (mmakeindex) {
                start("makeindex", fileNameNoSuffix(mfileName));
            } else if (mdvips) {
                start("dvips", fileNameNoSuffix(mfileName));
            } else {
                setUiEnabled(true);
                if (mopen && !exitCode)
                    open();
            }
        }
    }
}

void ConsoleWidget::inputLatinKey(int key)
{
    QString s(QChar(key + ((Qt::ShiftModifier == mmodifiers) ? 0 : ('a' - 'A'))));
    QKeyEvent ke(QEvent::KeyPress, key, mmodifiers, s);
    mtermwgt->innerEdit()->removeEventFilter(this);
    Application::sendEvent(mtermwgt->innerEdit(), &ke);
    mtermwgt->innerEdit()->installEventFilter(this);
}

void ConsoleWidget::performAction(int actId)
{
    switch (actId) {
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
    case SwitchCompilerAction:
        Settings::Console::setUseRemoteCompiler(!Settings::Console::useRemoteCompiler());
        bApp->updateConsoleSettings();
        break;
    case SettingsAction:
        bApp->showConsoleSettings();
        break;
    default:
        break;
    }
}

void ConsoleWidget::retranslateUi()
{
    consoleAction(ClearAction)->setText(tr("Clear console", "action text"));
    consoleAction(ClearAction)->setToolTip(tr("Clear console", "action toolTip"));
    consoleAction(ClearAction)->setWhatsThis(tr("Use this action to clear the contents of console",
                                                "act whatsThis"));
    consoleAction(CompileAction)->setText(tr("Compile", "action text"));
    QString sc = consoleAction(CompileAction)->shortcut().toString(QKeySequence::NativeText);
    consoleAction(CompileAction)->setToolTip(tr("Compile current document", "action toolTip") + " (" + sc + ")");
    consoleAction(CompileAction)->setWhatsThis(tr("Use this action to compile current document into "
                                                  ".pdf, .ps, or .dvi file", "act whatsThis"));
    consoleAction(CompileAndOpenAction)->setText(tr("Compile and open", "action text"));
    sc = consoleAction(CompileAndOpenAction)->shortcut().toString(QKeySequence::NativeText);
    consoleAction(CompileAndOpenAction)->setToolTip(tr("Compile and open current document",
                                                       "action toolTip") + " (" + sc + ")");
    consoleAction(CompileAndOpenAction)->setWhatsThis(tr("Use this action to compile current document "
                                                         "and then open the produced file", "act whatsThis"));
    consoleAction(OpenPdfAction)->setText(tr("Show PDF", "action text"));
    consoleAction(OpenPdfAction)->setToolTip(tr("Show current document using default PDF reader", "action toolTip"));
    consoleAction(OpenPdfAction)->setWhatsThis(tr("Use this action to open the .pdf file corresponding "
                                                  "to the current document", "act whatsThis"));
    consoleAction(OpenPsAction)->setText(tr("Show PS", "action text"));
    consoleAction(OpenPsAction)->setToolTip(tr("Show current document using default PS reader", "action toolTip"));
    consoleAction(OpenPsAction)->setWhatsThis(tr("Use this action to open the .ps file corresponding "
                                                 "to the current document", "act whatsThis"));
    updateSwitchCompilerAction();
    consoleAction(SettingsAction)->setText(tr("Console settings...", "action text"));
    consoleAction(SettingsAction)->setToolTip(tr("Configure console", "action toolTip"));
}
