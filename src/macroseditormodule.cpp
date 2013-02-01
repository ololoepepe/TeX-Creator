#include "macroseditormodule.h"
#include "application.h"

#include <BAbstractEditorModule>
#include <BCodeEditor>
#include <BCodeEditorDocument>
#include <BDirTools>
#include <BPlainTextEdit>

#include <QObject>
#include <QList>
#include <QString>
#include <QAction>
#include <QVariant>
#include <QPointer>
#include <QIcon>
#include <QFileDialog>
#include <QApplication>
#include <QKeyEvent>
#include <QEvent>
#include <QStringList>
#include <QVBoxLayout>
#include <QLayout>
#include <QKeySequence>
#include <QVariant>

#include <QDebug>

/*============================================================================
================================ Macro =======================================
============================================================================*/

/*============================== Public constructors =======================*/

Macro::Macro()
{
    //
}

Macro::Macro(const Macro &other)
{
    *this = other;
}

Macro::Macro(const QString &fileName)
{
    load(fileName);
}

/*============================== Public methods ============================*/

bool Macro::load(const QString &fileName)
{
    if ( fileName.isEmpty() )
        return false;
    bool ok = false;
    QStringList sl = BDirTools::readTextFile(fileName, "UTF-8", &ok).split('\n', QString::SkipEmptyParts);
    if (!ok)
        return false;
    clear();
    if ( sl.isEmpty() )
        return true;
    foreach ( int i, bRange(sl.size() - 1, 0, -1) )
        if (sl.at(i).at(0) == '#')
            sl.removeAt(i);
    if ( sl.isEmpty() )
        return true;
    foreach (const QString &s, sl)
    {
        QStringList sl = s.split(' ');
        if ( !bRange(2, 3).contains( sl.size() ) )
            continue;
        KeyPress k;
        bool ok = false;
        k.key = sl.first().toInt(&ok);
        if (!ok)
            continue;
        ok = false;
        k.modifiers = static_cast<Qt::KeyboardModifiers>( sl.at(1).toInt(&ok) );
        if (!ok)
            continue;
        if (sl.size() == 3)
            k.text = sl.last();
        mkeys << k;
    }
    return true;
}

bool Macro::save(const QString &fileName) const
{
    if ( fileName.isEmpty() || !isValid() )
        return false;
    QString s;
    foreach (const KeyPress &k, mkeys)
        s += (QString::number(k.key) + " " + QString::number(k.modifiers) +
              ( !k.text.isEmpty() ? k.text : QString() ) + "\n");
    return BDirTools::writeTextFile(fileName, s, "UTF-8");
}

bool Macro::isValid() const
{
    return !mkeys.isEmpty();
}

void Macro::clear()
{
    mkeys.clear();
}

bool Macro::recordKeyPress(QEvent *e, QString *s)
{
    if (!e || e->type() != QEvent::KeyPress)
        return false;
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    KeyPress k;
    k.key = ke->key();
    if (Qt::Key_Control == k.key || Qt::Key_Alt == k.key || Qt::Key_Shift == k.key)
        return false;
    k.modifiers = ke->modifiers();
    k.text = ke->text();
    mkeys << k;
    if (s)
        *s = keyPressToText(k);
    return true;
}

void Macro::apply(QObject *object) const
{
    if ( !object || !isValid() )
        return;
    foreach (const KeyPress &k, mkeys)
    {
        QApplication::postEvent( object, new QKeyEvent(QEvent::KeyPress, k.key, k.modifiers, k.text) );
        QApplication::processEvents();
    }
}

QString Macro::toText() const
{
    QString s;
    foreach (const KeyPress &k, mkeys)
        s += (keyPressToText(k) + "\n");
    if ( !s.isNull() )
        s.remove(s.length() - 1, 1);
    return s;
}

/*============================== Public operators ==========================*/

Macro &Macro::operator=(const Macro &other)
{
    mkeys = other.mkeys;
    return *this;
}

/*============================== Static private methods ====================*/

QString Macro::keyPressToText(const KeyPress &k)
{
    if ( !k.text.isEmpty() && k.text.at(0).isPrint() && !k.text.at(0).isSpace() &&
         !(k.modifiers & Qt::ControlModifier) && !(k.modifiers & Qt::AltModifier) )
        return k.text;
    return QKeySequence(k.key | k.modifiers).toString(QKeySequence::NativeText);
}

/*============================================================================
================================ MacrosEditorModule ==========================
============================================================================*/

/*============================== Public constructors =======================*/

MacrosEditorModule::MacrosEditorModule(QObject *parent) :
    BAbstractEditorModule(parent)
{
    mplaying = false;
    mrecording = false;
    mprevDoc = 0;
    //
    mactStartStop = new QAction(this);
      connect( mactStartStop.data(), SIGNAL( triggered() ), this, SLOT( startStopRecording() ) );
    mactClear = new QAction(this);
      mactClear->setIcon( Application::icon("editclear") );
      connect( mactClear.data(), SIGNAL( triggered() ), this, SLOT( clearMacro() ) );
    mactPlay = new QAction(this);
      mactPlay->setIcon( Application::icon("player_play") );
      connect( mactPlay.data(), SIGNAL( triggered() ), this, SLOT( playMacro() ) );
    mactShowHide = new QAction(this);
      connect( mactShowHide.data(), SIGNAL( triggered() ), this, SLOT( showHideMacrosConsole() ) );
    mactLoad = new QAction(this);
      mactLoad->setIcon( Application::icon("fileopen") );
      connect( mactLoad.data(), SIGNAL( triggered() ), this, SLOT( loadMacro() ) );
    mactSaveAs = new QAction(this);
      mactSaveAs->setIcon( Application::icon("filesaveas") );
      connect( mactSaveAs.data(), SIGNAL( triggered() ), this, SLOT( saveMacroAs() ) );
    mactOpenDir = new QAction(this);
      mactOpenDir->setIcon( Application::icon("folder_open") );
      connect( mactOpenDir.data(), SIGNAL( triggered() ), this, SLOT( openUserDir() ) );
    mptedt = new QPlainTextEdit;
      mptedt->setFixedHeight(100);
      mptedt->setReadOnly(true);
    //
    connect( bApp, SIGNAL( languageChanged() ), this, SLOT( retranslateUi() ) );
    retranslateUi();
}
MacrosEditorModule::~MacrosEditorModule()
{
    if ( !mptedt.isNull() && !mptedt->parentWidget() )
        delete mptedt;
}

/*============================== Public methods ============================*/

QString MacrosEditorModule::id() const
{
    return "macros";
}

QAction *MacrosEditorModule::action(int type)
{
    switch (type)
    {
    case StartStopRecordingAction:
        return mactStartStop.data();
    case ClearAction:
        return mactClear.data();
    case PlayAction:
        return mactPlay.data();
    case ShowHideAction:
        return mactShowHide.data();
    case LoadAction:
        return mactLoad.data();
    case SaveAsAction:
        return mactSaveAs.data();
    case OpenUserMacrosDirAction:
        return mactOpenDir.data();
    default:
        return 0;
    }
}

QList<QAction *> MacrosEditorModule::actions(bool extended)
{
    QList<QAction *> list;
    list << action(StartStopRecordingAction);
    list << action(PlayAction);
    list << action(ClearAction);
    list << action(ShowHideAction);
    if (extended)
    {
        list << action(LoadAction);
        list << action(SaveAsAction);
        list << action(OpenUserMacrosDirAction);
    }
    return list;
}

bool MacrosEditorModule::eventFilter(QObject *, QEvent *e)
{
    if (!mrecording)
        return false;
    QString txt;
    if ( !mmacro.recordKeyPress(e, &txt) )
        return false;
    if ( !mptedt.isNull() )
        mptedt->appendPlainText(txt);
    return false;
}

/*============================== Public slots ==============================*/

void MacrosEditorModule::startStopRecording()
{
    if (mplaying)
        return;
    mrecording = !mrecording;
    resetStartStopAction();
    checkActions();
}

void MacrosEditorModule::clearMacro()
{
    if (mplaying)
        return;
    mmacro.clear();
    if ( !mptedt.isNull() )
        mptedt->clear();
}

void MacrosEditorModule::playMacro()
{
    BCodeEditorDocument *doc = currentDocument();
    if ( !doc || mplaying || mrecording || !mmacro.isValid() )
        return;
    mplaying = true;
    checkActions();
    mmacro.apply( doc->findChild<BPlainTextEdit *>() );
    mplaying = false;
    checkActions();
}

void MacrosEditorModule::showHideMacrosConsole()
{
    if ( mptedt.isNull() || !editor() )
        return;
    mptedt->setVisible( !mptedt->isVisible() );
    resetShowHideAction();
}

bool MacrosEditorModule::loadMacro(const QString &fileName)
{
    if (mplaying || mrecording)
        return false;
    QString fn = fileName;
    if ( fn.isEmpty() )
        fn = QFileDialog::getOpenFileName(
                    editor(), tr("Open", "fdlg caption"),
                    BDirTools::findResource("macros", BDirTools::UserOnly), fileDialogFilter() );
    bool b = !fn.isEmpty() && mmacro.load(fn);
    if ( b && !mptedt.isNull() )
        mptedt->setPlainText( mmacro.toText() );
    checkActions();
    return b;
}

bool MacrosEditorModule::saveMacroAs(const QString &fileName)
{
    if ( mrecording || !mmacro.isValid() )
        return false;
    QString fn = fileName;
    if ( fn.isEmpty() )
        fn = QFileDialog::getSaveFileName(
                    editor(), tr("Save", "fdlg caption"),
                    BDirTools::findResource("macros", BDirTools::UserOnly), fileDialogFilter() );
    return !fn.isEmpty() && mmacro.save(fn);
}

void MacrosEditorModule::openUserDir()
{
    bApp->openLocalFile( BDirTools::findResource("macros") );
}

/*============================== Protected methods =========================*/

void MacrosEditorModule::editorSet(BCodeEditor *edr)
{
    if ( edr && !mptedt.isNull() )
    {
        QVBoxLayout *vlt = static_cast<QVBoxLayout *>( edr->layout() );
        vlt->insertWidget( 0, mptedt.data() );
        mptedt->hide();
    }
    resetStartStopAction();
    resetShowHideAction();
    checkActions();
}

void MacrosEditorModule::editorUnset(BCodeEditor *edr)
{
    if ( edr && !mptedt.isNull() )
    {
        mptedt->hide();
        QVBoxLayout *vlt = static_cast<QVBoxLayout *>( edr->layout() );
        vlt->removeWidget( mptedt.data() );
        mptedt->setParent(0);
    }
    resetStartStopAction();
    resetShowHideAction();
    checkActions();
}

void MacrosEditorModule::currentDocumentChanged(BCodeEditorDocument *doc)
{
    if (mprevDoc)
        mprevDoc->findChild<BPlainTextEdit *>()->removeEventFilter(this);
    mprevDoc = doc;
    if (doc)
        doc->findChild<BPlainTextEdit *>()->installEventFilter(this);
    if (mplaying)
        mplaying = false;
    if (mrecording)
        startStopRecording();
    resetStartStopAction();
    checkActions();
}

/*============================== Static private methods ====================*/

QString MacrosEditorModule::fileDialogFilter()
{
    return tr("TeX Creator macros", "fdlg filter") + " (*.tsm)";
}

/*============================== Private methods ===========================*/

void MacrosEditorModule::resetStartStopAction()
{
    if ( mactStartStop.isNull() )
        return;
    mactStartStop->setEnabled(currentDocument() && !mplaying);
    if (mrecording)
    {
        mactStartStop->setIcon( Application::icon("player_stop") );
        mactStartStop->setText( tr("Stop recording", "act text") );
        mactStartStop->setToolTip( tr("Stop recording macro", "act toolTip") );
        //TODO: whatsThis
    }
    else
    {
        mactStartStop->setIcon( Application::icon("player_record") );
        mactStartStop->setText( tr("Start recording", "act text") );
        mactStartStop->setToolTip( tr("Start recording macro", "act toolTip") );
        //TODO: whatsThis
    }
}

void MacrosEditorModule::resetShowHideAction()
{
    if ( mactShowHide.isNull() )
        return;
    if ( !mptedt.isNull() && mptedt->isVisible() )
    {
        mactShowHide->setIcon( Application::icon("1uparrow") );
        mactShowHide->setText( tr("Hide console", "act text") );
        mactShowHide->setToolTip( tr("Hide macros console", "act toolTip") );
        //TODO: whatsThis
    }
    else
    {
        mactShowHide->setIcon( Application::icon("1downarrow") );
        mactShowHide->setText( tr("Show console", "act text") );
        mactShowHide->setToolTip( tr("Show macros console", "act toolTip") );
        //TODO: whatsThis
    }
}

void MacrosEditorModule::checkActions()
{
    bool b = currentDocument();
    if ( !mactClear.isNull() )
        mactClear->setEnabled(!mplaying);
    if ( !mactPlay.isNull() )
        mactPlay->setEnabled( b && !mplaying && !mrecording && mmacro.isValid() );
    if ( !mactSaveAs.isNull() )
        mactSaveAs->setEnabled( !mrecording && mmacro.isValid() );
}

/*============================== Private slots =============================*/

void MacrosEditorModule::retranslateUi()
{
    if ( !mactClear.isNull() )
    {
        mactClear->setText( tr("Clear", "act text") );
        mactClear->setToolTip( tr("Clear current macro", "act toolTip") );
        mactClear->setWhatsThis( tr("Use this action to clear currntly loaded or recorded macro. "
                                    "The corresponding file will not be deleted", "act whatsThis") );
    }
    if ( !mactPlay.isNull() )
    {
        mactPlay->setText( tr("Play", "act text") );
        mactPlay->setToolTip( tr("Play current macro", "act toolTip") );
        mactPlay->setWhatsThis( tr("Use this action to activate previously loaded or recorded macro",
                                   "act whatsThis") );
    }
    if ( !mactLoad.isNull() )
    {
        mactLoad->setText( tr("Load...", "act text") );
        mactLoad->setToolTip( tr("Load macro", "act toolTip") );
        mactLoad->setWhatsThis( tr("Use this action to load previously saved macro from file", "act whatsThis") );
    }
    if ( !mactSaveAs.isNull() )
    {
        mactSaveAs->setText( tr("Save as...", "act text") );
        mactSaveAs->setToolTip( tr("Save current macro as...", "act toolTip") );
        mactSaveAs->setWhatsThis( tr("Use this action to save current macro to a file", "act whatsThis") );
    }
    if ( !mactOpenDir.isNull() )
    {
        mactOpenDir->setText( tr("Open user macros dir", "act text") );
        //TODO: toolTip and whatsThis
    }
    resetStartStopAction();
    resetShowHideAction();
}
