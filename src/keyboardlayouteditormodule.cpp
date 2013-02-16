#include "keyboardlayouteditormodule.h"
#include "application.h"

#include <BAbstractEditorModule>
#include <BCodeEditor>
#include <BCodeEditorDocument>
#include <BDirTools>

#include <QObject>
#include <QList>
#include <QString>
#include <QAction>
#include <QPointer>
#include <QIcon>
#include <QKeySequence>
#include <QMap>
#include <QChar>
#include <QLocale>
#include <QPoint>

#include <QDebug>

/*============================================================================
================================ KeyboardLayoutMap ===========================
============================================================================*/

/*============================== Public constructors =======================*/

KeyboardLayoutMap::KeyboardLayoutMap()
{
    //
}

KeyboardLayoutMap::KeyboardLayoutMap(const KeyboardLayoutMap &other)
{
    *this = other;
}

KeyboardLayoutMap::KeyboardLayoutMap(const QString &fileName)
{
    load(fileName);
}

/*============================== Public methods ============================*/

bool KeyboardLayoutMap::load(const QString &fileName)
{
    if ( fileName.isEmpty() )
        return false;
    bool ok = false;
    QStringList sl = BDirTools::readTextFile(fileName, "UTF-8", &ok).split('\n', QString::SkipEmptyParts);
    if (!ok)
        return false;
    mdirect.clear();
    mreverse.clear();
    mdirectUnique.clear();
    mreverseUnique.clear();
    foreach (int i, bRangeR(sl.size() - 1, 0))
        if (sl.at(i).at(0) == '#')
            sl.removeAt(i);
    if (sl.isEmpty())
        return true;
    foreach (const QString &s, sl)
    {
        QStringList sl = s.split(' ');
        if (sl.size() != 2 || sl.first().length() != 1 || sl.last().length() != 1)
            continue;
        const QChar &fc = sl.first().at(0);
        const QChar &sc = sl.last().at(0);
        if ( mdirect.contains(fc) || mreverse.contains(sc) )
            continue;
        mdirect.insert(fc, sc);
        mdirectUnique << fc;
        mreverse.insert(sc, fc);
        mreverseUnique << sc;
    }
    foreach (int i, bRangeR(mdirectUnique.size() - 1, 0))
    {
        const QChar &c = mdirectUnique.at(i);
        if ( mreverseUnique.contains(c) )
        {
            mdirectUnique.removeAll(c);
            mreverseUnique.removeAll(c);
        }
    }
    return true;
}

bool KeyboardLayoutMap::isValid() const
{
    return !mdirect.isEmpty() && !mreverse.isEmpty() && !mdirectUnique.isEmpty() && !mreverseUnique.isEmpty();
}

bool KeyboardLayoutMap::switchLayout(QString &text) const
{
    if ( text.isEmpty() || !isValid() )
        return false;
    int direct = 0;
    int reverse = 0;
    foreach (const QChar &c, mdirectUnique)
        direct += text.count(c);
    foreach (const QChar &c, mreverseUnique)
        reverse += text.count(c);
    if (direct == reverse)
        return false;
    const QMap<QChar, QChar> &m = (direct > reverse) ? mdirect : mreverse;
    QList<QChar> keys = m.keys();
    foreach (int i, bRangeD(0, text.length() - 1))
        if ( keys.contains( text.at(i) ) )
            text[i] = m.value( text.at(i) );
    return true;
}

/*============================== Public operators ==========================*/

KeyboardLayoutMap &KeyboardLayoutMap::operator=(const KeyboardLayoutMap &other)
{
    mdirect = other.mdirect;
    mreverse = other.mreverse;
    mdirectUnique = other.mdirectUnique;
    mreverseUnique = other.mreverseUnique;
    return *this;
}

/*============================================================================
================================ KeyboardLayoutEditorModule ==================
============================================================================*/

/*============================== Public constructors =======================*/

KeyboardLayoutEditorModule::KeyboardLayoutEditorModule(QObject *parent) :
    BAbstractEditorModule(parent)
{
    mactSwitch = new QAction(this);
      mactSwitch->setEnabled(false);
      mactSwitch->setIcon( Application::icon("charset") );
      mactSwitch->setShortcut( QKeySequence("Ctrl+L") );
      connect( mactSwitch.data(), SIGNAL( triggered() ), this, SLOT( switchLayout() ) );
    mactReload = new QAction(this);
      mactReload->setIcon( Application::icon("reload") );
      connect( mactReload.data(), SIGNAL( triggered() ), this, SLOT( reloadMap() ) );
    mactOpenDir = new QAction(this);
      mactOpenDir->setIcon( Application::icon("folder_open") );
      connect( mactOpenDir.data(), SIGNAL( triggered() ), this, SLOT( openUserDir() ) );
    //
    reloadMap();
    connect( bApp, SIGNAL( languageChanged() ), this, SLOT( retranslateUi() ) );
    retranslateUi();
}

/*============================== Public methods ============================*/

QString KeyboardLayoutEditorModule::id() const
{
    return "keyboard_layout";
}

QAction *KeyboardLayoutEditorModule::action(int type)
{
    switch (type)
    {
    case SwitchSelectedTextLayoutAction:
        return mactSwitch.data();
    case ReloadKLMAction:
        return mactReload.data();
    case OpenUserKLMDirAction:
        return mactOpenDir.data();
    default:
        return 0;
    }
}

QList<QAction *> KeyboardLayoutEditorModule::actions(bool extended)
{
    QList<QAction *> list;
    list << action(SwitchSelectedTextLayoutAction);
    if (extended)
    {
        list << action(ReloadKLMAction);
        list << action(OpenUserKLMDirAction);
    }
    return list;
}

/*============================== Public slots ==============================*/

void KeyboardLayoutEditorModule::switchLayout()
{
    BCodeEditorDocument *doc = currentDocument();
    if ( !doc || !doc->hasSelection() )
        return;
    QString txt = doc->selectedText();
    QPoint ps = doc->selectionStart();
    QPoint pe = doc->selectionEnd();
    if ( !mmap.switchLayout(txt) )
        return;
    doc->insertText(txt);
    doc->selectText(ps, pe);
}

void KeyboardLayoutEditorModule::reloadMap()
{
    QString fn = BDirTools::findResource("klm/en-" + QLocale::system().name().left(2) + ".klm");
    mmap.load(fn);
    checkSwitchAction();
}

void KeyboardLayoutEditorModule::openUserDir()
{
    bApp->openLocalFile( BDirTools::findResource("klm", BDirTools::UserOnly) );
}

/*============================== Protected methods =========================*/

void KeyboardLayoutEditorModule::currentDocumentChanged(BCodeEditorDocument *)
{
    checkSwitchAction();
}

void KeyboardLayoutEditorModule::documentHasSelectionChanged(bool)
{
    checkSwitchAction();
}

/*============================== Private methods ===========================*/

void KeyboardLayoutEditorModule::checkSwitchAction()
{
    if ( mactSwitch.isNull() )
        return;
    mactSwitch->setEnabled( currentDocument() && currentDocument()->hasSelection() && mmap.isValid() );
}

/*============================== Private slots =============================*/

void KeyboardLayoutEditorModule::retranslateUi()
{
    if ( !mactSwitch.isNull() )
    {
        mactSwitch->setText( tr("Switch layout", "act text") );
        mactSwitch->setToolTip( tr("Switch selected text layout", "act toolTip") );
        mactSwitch->setWhatsThis( tr("Use this action to switch selected text layout (e.g. from EN to RU)",
                                     "act whatsThis") );
    }
    if ( !mactReload.isNull() )
    {
        mactReload->setText( tr("Reload keyboard layout map", "act text") );
        //TODO: toolTip and whatsThis
    }
    if ( !mactOpenDir.isNull() )
    {
        mactOpenDir->setText( tr("Open user keyboard layout map folder", "act text") );
        //TODO: toolTip and whatsThis
    }
}
