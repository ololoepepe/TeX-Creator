#include "src/consolewidget.h"

#include <bcore.h>
#include <bterminal.h>
#include <btexteditor.h>

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
#include <QTextCursor>
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

#include <QDebug>

const QString GroupConsole = "console";
  const QString KeyCompilerCommand = "compiler_command";
  const QString KeyMakeindex = "makeindex";
  const QString KeyDvips = "dvips";
  const QString KeyCompilerArguments = "compiler_arguments";
  const QString KeyAlwaysLatin = "always_latin";

//

QString fileNameNoSuffix(const QString &fileName)
{
    int nlen = fileName.length();
    int slen = QFileInfo(fileName).suffix().length();
    return slen ? fileName.left(nlen - slen - 1) : fileName;
}

//

ConsoleWidget::ConsoleWidget(const QString &settingsGroup, QWidget *parent) :
    QWidget(parent), mCSettingsGroup(settingsGroup)
{
    mTerminal = new BTerminal(this);
    connect( mTerminal, SIGNAL( finished(int) ), this, SLOT( finished(int) ) );
    mEditor = 0;
    mTcfExp.setForeground( QBrush(Qt::blue) );
    mTcfExpB = mTcfExp;
    mTcfExpB.setFontWeight(QFont::Bold);
    mTcfExpI = mTcfExp;
    mTcfExpI.setFontItalic(true);
    mTcfErr.setForeground( QBrush(Qt::red) );
    mTcfErrB = mTcfErr;
    mTcfErrB.setFontWeight(QFont::Bold);
    mTcfErrI = mTcfErr;
    mTcfErrI.setFontItalic(true);
    mmprActions = new QSignalMapper(this);
    connect( mmprActions, SIGNAL( mapped(int) ), this, SLOT( performAction(int) ) );
    initKeyMap();
    initGui();
    loadSettings();
    retranslateUi();
    connect( BCore::instance(), SIGNAL( localeChanged() ), this, SLOT( retranslateUi() ) );
}

//

bool ConsoleWidget::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() != QEvent::KeyPress)
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

void ConsoleWidget::setTextEditor(BTextEditor *editor)
{
    if (mEditor)
    {
        disconnect( mEditor, SIGNAL( documentAvailableChanged(bool) ), this, SLOT( checkCompileAvailable() ) );
        disconnect( mEditor, SIGNAL( documentAvailableChanged(bool) ),
                    consoleAction(OpenPdfAction), SLOT( setEnabled(bool) ) );
        disconnect( mEditor, SIGNAL( documentAvailableChanged(bool) ),
                    consoleAction(OpenPsAction), SLOT( setEnabled(bool) ) );
    }
    mEditor = editor;
    bool b = mEditor && mEditor->isDocumentAvailable();
    consoleAction(OpenPdfAction)->setEnabled(b);
    consoleAction(OpenPsAction)->setEnabled(b);
    checkCompileAvailable();
    if (mEditor)
    {
        connect( mEditor, SIGNAL( documentAvailableChanged(bool) ), this, SLOT( checkCompileAvailable() ) );
        connect( mEditor, SIGNAL( documentAvailableChanged(bool) ),
                 consoleAction(OpenPdfAction), SLOT( setEnabled(bool) ) );
        connect( mEditor, SIGNAL( documentAvailableChanged(bool) ),
                 consoleAction(OpenPsAction), SLOT( setEnabled(bool) ) );
    }
}

void ConsoleWidget::saveSettings()
{
    QScopedPointer<QSettings> s( BCore::newSettingsInstance() );
    if (!s)
        return;
    if ( !mCSettingsGroup.isEmpty() )
        s->beginGroup(mCSettingsGroup);
    s->beginGroup(GroupConsole);
      s->setValue( KeyCompilerCommand, mcmboxCommand->currentText() );
      s->setValue( KeyMakeindex, mcboxMakeindex->isChecked() );
      s->setValue( KeyDvips, mcboxDvips->isChecked() );
      s->setValue( KeyCompilerArguments, mledtParameters->text() );
      s->setValue( KeyAlwaysLatin, mcboxAlwaysLatin->isChecked() );
    s->endGroup();
    if ( !mCSettingsGroup.isEmpty() )
        s->endGroup();
}

QAction *ConsoleWidget::consoleAction(Action actId) const
{
    return mActMap.value(actId);
}

QList<QAction *> ConsoleWidget::consoleActions() const
{
    return mActMap.values();
}

//

void ConsoleWidget::performAction(int actId)
{
    switch (actId)
    {
    case ClearAction:
        mTerminal->clearEdit();
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
    default:
        break;
    }
}

//

void ConsoleWidget::loadSettings()
{
    QScopedPointer<QSettings> s( BCore::newSettingsInstance() );
    if (!s)
        return;
    if ( !mCSettingsGroup.isEmpty() )
        s->beginGroup(mCSettingsGroup);
    s->beginGroup(GroupConsole);
      QString command = s->value(KeyCompilerCommand).toString();
      mcboxMakeindex->setChecked( s->value(KeyMakeindex).toBool() );
      mcboxDvips->setChecked( s->value(KeyDvips).toBool() );
      mledtParameters->setText( s->value(KeyCompilerArguments).toString() );
      mcboxAlwaysLatin->setChecked( s->value(KeyAlwaysLatin, true).toBool() );
    s->endGroup();
    if ( !mCSettingsGroup.isEmpty() )
        s->endGroup();
    //
    int ind = mcmboxCommand->findText(command);
    if (ind >= 0)
        mcmboxCommand->setCurrentIndex(ind);
}

void ConsoleWidget::initKeyMap()
{
    mkeyMap.insert(1040, 'F');
    mkeyMap.insert(1042 , 'D');
    mkeyMap.insert(1043 , 'U');
    mkeyMap.insert(1044 , 'L');
    mkeyMap.insert(1045 , 'T');
    mkeyMap.insert(1047 , 'P');
    mkeyMap.insert(1048 , 'B');
    mkeyMap.insert(1049 , 'Q');
    mkeyMap.insert(1050 , 'R');
    mkeyMap.insert(1051 , 'K');
    mkeyMap.insert(1052 , 'V');
    mkeyMap.insert(1053 , 'Y');
    mkeyMap.insert(1054 , 'J');
    mkeyMap.insert(1055 , 'G');
    mkeyMap.insert(1056 , 'H');
    mkeyMap.insert(1057 , 'C');
    mkeyMap.insert(1058 , 'N');
    mkeyMap.insert(1059 , 'E');
    mkeyMap.insert(1060 , 'A');
    mkeyMap.insert(1062 , 'W');
    mkeyMap.insert(1063 , 'X');
    mkeyMap.insert(1064 , 'I');
    mkeyMap.insert(1065 , 'O');
    mkeyMap.insert(1067 , 'S');
    mkeyMap.insert(1068 , 'M');
    mkeyMap.insert(1071 , 'Z');
}

void ConsoleWidget::initGui()
{
    mvlt = new QVBoxLayout(this);
      mvlt->setContentsMargins(2, 2, 2, 2);
      mtbar = new QToolBar(this);
        mtbar->setIconSize( QSize(24, 24) );
        createAction(ClearAction, "", "", true)->setIcon( QIcon( BCore::beqtIcon("editclear") ) );
        createAction(CompileAction, "compfile.png", "Ctrl+Shift+B");
        createAction(CompileAndOpenAction, "run_build.png", "Ctrl+Shift+R");
        createAction(OpenPdfAction, "pdf.png", "");
        createAction(OpenPsAction, "postscript.png", "");
        mtbar->addSeparator();
        mlblCommand = new QLabel(this);
        mtbar->addWidget(mlblCommand);
        mcmboxCommand = new QComboBox(this);
          QStringList sl;
          sl << "pdflatex";
          sl << "pdftex";
          sl << "latex";
          sl << "tex";
          mcmboxCommand->addItems(sl);
        mtbar->addWidget(mcmboxCommand);
        mtbar->addSeparator();
        mcboxMakeindex = new QCheckBox(this);
        mtbar->addWidget(mcboxMakeindex);
        mcboxDvips = new QCheckBox(this);
        mtbar->addWidget(mcboxDvips);
        mtbar->addSeparator();
        mlblParameters = new QLabel;
        mtbar->addWidget(mlblParameters);
        mledtParameters = new QLineEdit;
        mtbar->addWidget(mledtParameters);
        mtbar->addSeparator();
        mcboxAlwaysLatin = new QCheckBox(this);
          mcboxAlwaysLatin->setChecked(true);
        mtbar->addWidget(mcboxAlwaysLatin);
      mvlt->addWidget(mtbar);
      QWidget *wgt = mTerminal->widget();
        wgt->setParent(this);
        wgt->installEventFilter(this);
      mvlt->addWidget(wgt);
}

QAction *ConsoleWidget::createAction(int id, const QString &iconFileName, const QString &shortcut, bool enabled)
{
    if ( id < 0 || mActMap.contains(id) )
        return 0;
    QAction *act = new QAction(this);
    act->setEnabled(enabled);
    if ( !iconFileName.isEmpty() )
        act->setIcon( QIcon(":/res/ico/" + iconFileName) );
    if ( !shortcut.isEmpty() )
    {
        act->setShortcut( QKeySequence(shortcut) );
        act->setShortcutContext(Qt::WindowShortcut);
    }
    mmprActions->setMapping(act, id);
    connect( act, SIGNAL( triggered() ), mmprActions, SLOT( map() ) );
    mActMap.insert(id, act);
    mtbar->addAction(act);
    return act;
}

void ConsoleWidget::compile(bool run)
{
    if (mTerminal->isStarted() || !mEditor)
        return;
    if ( mEditor->hasMainDocument() )
    {
        if ( !mEditor->performAction(BTextEditor::SaveAllDocumentsAction) )
            return;
    }
    else
    {
        if ( !mEditor->performAction(BTextEditor::SaveDocumentAction) )
            return;
    }
    mFileName = mEditor->mainDocumentFileName();
    if ( mFileName.isEmpty() )
        mFileName = mEditor->currentDocumentFileName();
    if ( mFileName.isEmpty() )
    {
        mTerminal->appendLine(tr("File name is not specified", "textEdit text"), mTcfErr);
        return;
    }
    if ( !QFile(mFileName).exists() )
    {
        mTerminal->appendLine(tr("File does not exist", "textEdit text"), mTcfErr);
        return;
    }
    mRun = run && mcmboxCommand->currentText().contains("pdf");
    mWorkingDir = QFileInfo(mFileName).path();
    startCompiler();
}

void ConsoleWidget::open(bool pdf)
{
    QString fn = mFileName;
    mFileName.clear();
    if ( fn.isEmpty() )
        fn = mEditor->hasMainDocument() ? mEditor->mainDocumentFileName() : mEditor->currentDocumentFileName();
    if ( fn.isEmpty() )
        return;
    QString fns = fileNameNoSuffix(fn) + (pdf ? ".pdf" : ".ps");
    if ( !QFile(fns).exists() ||
         !QDesktopServices::openUrl( QUrl::fromLocalFile(fns) ) )
    {
        mTerminal->appendText(tr("Failed to open", "edit text"),  mTcfErr);
        mTerminal->appendLine(" " + fns, mTcfErrI);
    }
}

void ConsoleWidget::startCompiler()
{
    QStringList args;
    QString prog = mcmboxCommand->currentText();
    args << mledtParameters->text().split(' ');
    args << "\"" + mFileName + "\"";
    if ( mTerminal->start(prog, args, mWorkingDir, 5000) )
        executingMessage(prog);
    else
        failedMessage(prog);
    checkCompileAvailable();
}

void ConsoleWidget::startMakeindex()
{
    QString prog = "makeindex";
    QStringList args;
    args << fileNameNoSuffix(mFileName);
    if ( mTerminal->start(prog, args, mWorkingDir, 5000) )
        executingMessage(prog);
    else
        failedMessage(prog);
    checkCompileAvailable();
}

void ConsoleWidget::startDvips()
{
    QString prog = "dvips";
    QStringList args;
    args << fileNameNoSuffix(mFileName);
    if ( mTerminal->start(prog, args, mWorkingDir, 5000) )
        executingMessage(prog);
    else
        failedMessage(prog);
    checkCompileAvailable();
}

void ConsoleWidget::executingMessage(const QString &program)
{
    mTerminal->appendText(tr("Executing", "textEdit text"), mTcfExp);
    mTerminal->appendText(" " + program + " ", mTcfExpB);
    mTerminal->appendText(tr("for", "edit text"), mTcfExp);
    mTerminal->appendText(" " + mFileName, mTcfExpI);
    mTerminal->appendLine("...", mTcfExp);
}

void ConsoleWidget::failedMessage(const QString &program)
{
    mTerminal->appendText(tr("Failed to start", "edit text"), mTcfErr);
    mTerminal->appendLine(" " + program, mTcfErrB);
    mTerminal->appendLine("");
}

void ConsoleWidget::finishedMessage(const QString &program, int code)
{
    mTerminal->appendText(program + " ", mTcfExpB);
    mTerminal->appendLine(tr("finished with code", "edit text") +
                         " " + QString::number(code), mTcfExp);
    mTerminal->appendLine("");
}

//

void ConsoleWidget::retranslateUi()
{
    consoleAction(ClearAction)->setText( tr("Clear console", "action text") );
    consoleAction(ClearAction)->setToolTip( tr("Clear console", "action toolTip") );
    consoleAction(ClearAction)->setWhatsThis( tr("Use this action to clear the contents of console",
                                                 "act whatsThis") );
    consoleAction(CompileAction)->setText( tr("Compile", "action text") );
    consoleAction(CompileAction)->setToolTip( tr("Compile current document", "action toolTip") );
    consoleAction(CompileAction)->setWhatsThis( tr("Use this action to compile current document into "
                                                   ".pdf, .ps, or .dvi file", "act whatsThis") );
    consoleAction(CompileAndOpenAction)->setText( tr("Compile and open", "action text") );
    consoleAction(CompileAndOpenAction)->setToolTip( tr("Compile and open current document", "action toolTip") );
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
    mcboxMakeindex->setText( tr("makeindex", "cbox text") );
    mcboxMakeindex->setToolTip( tr("Run makeindex before compilation", "cbox toolTip") );
    mcboxMakeindex->setWhatsThis( tr("Set this option to run the makeindex utility before the compiler",
                                     "cbox whatsThis") );
    mlblCommand->setText(tr("Compiler:", "label text") + " ");
    mcboxDvips->setText( tr("dvips", "cbox text") );
    mcboxDvips->setToolTip( tr("Run dvips after compilation", "cbox toolTip") );
    mcboxDvips->setWhatsThis( tr("Set this option to run the dvips utility after the compiler", "cbox whatsThis") );
    mlblParameters->setText(tr("Compiler parameters:", "label text") + " ");
    mcboxAlwaysLatin->setText( tr("Always Latin", "cbox text") );
    mcboxAlwaysLatin->setToolTip( tr("If checked, Latin letters will always be entered, ignoring keyboard layout",
                                     "cbox toolTip") );
    mcboxAlwaysLatin->setWhatsThis( tr("Set this option if you always enter latin only characters into console, "
                                       "so you will not have to switch keyboard layout", "cbox whatsThis") );
}

void ConsoleWidget::checkCompileAvailable()
{
    bool b = !mTerminal->isStarted() && mEditor &&
            mEditor->isDocumentAvailable();
    consoleAction(CompileAction)->setEnabled(b);
    consoleAction(CompileAndOpenAction)->setEnabled(b);
}

void ConsoleWidget::finished(int exitCode)
{
    QString prog = mTerminal->program();
    finishedMessage(prog, exitCode);
    if ("makeindex" == prog)
    {
        if ( mcboxDvips->isChecked() )
        {
            startDvips();
        }
        else
        {
            checkCompileAvailable();
            if (mRun && !exitCode)
                open();
        }
    }
    else if ("dvips" == prog)
    {
        checkCompileAvailable();
        if (mRun && !exitCode)
            open();
    }
    else
    {
        if ( mcboxMakeindex->isChecked() )
        {
            startMakeindex();
        }
        else if ( mcboxDvips->isChecked() )
        {
            startDvips();
        }
        else
        {
            checkCompileAvailable();
            if (mRun && !exitCode)
                open();
        }
    }
}
