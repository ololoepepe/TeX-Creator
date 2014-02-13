#include "macroseditormodule.h"
#include "application.h"
#include "global.h"

#include <BAbstractEditorModule>
#include <BCodeEditor>
#include <BAbstractCodeEditorDocument>
#include <BDirTools>
#include <BeQt>
#include <BSimpleCodeEditorDocument>
#include <BAbstractFileType>

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
#include <QRegExp>
#include <QTextCursor>
#include <QTextBlock>
#include <QFileInfo>
#include <QProcess>
#include <QInputDialog>
#include <QMenu>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QColor>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSet>
#include <QVariantMap>
#include <QByteArray>

#include <QDebug>

#include <climits>

/*============================================================================
================================ TeXCreatorMacroFileType =====================
============================================================================*/

class TeXCreatorMacroFileType : public BAbstractFileType
{
    Q_DECLARE_TR_FUNCTIONS(TeXCreatorMacroFileType)
public:
    TeXCreatorMacroFileType();
    ~TeXCreatorMacroFileType();
public:
    QString id() const;
    QString name() const;
    QString description() const;
    QStringList suffixes() const;
    bool matchesFileName(const QString &fileName) const;
    BracketPairList brackets() const;
protected:
    void highlightBlock(const QString &text);
private:
    Q_DISABLE_COPY(TeXCreatorMacroFileType)
};

/*============================================================================
================================ Static functions ============================
============================================================================*/

static QString macroExecute(const QStringList &args, BAbstractCodeEditorDocument *doc, bool *ok)
{
    if (!doc || args.size() < 1)
        return bRet(ok, false, QString());
    QString cmd = Global::externalTools().value(args.first());
    if (cmd.isEmpty())
        return bRet(ok, false, QString());
    QFileInfo fi(doc->fileName());
    QString path;
    if (fi.exists() && fi.isFile())
        path = fi.path();
    QString out;
    int r = path.isEmpty() ?
                BeQt::execProcess(cmd, args.mid(1), 2 * BeQt::Second, 30 * BeQt::Second, &out, doc->codec()) :
                BeQt::execProcess(path, cmd, args.mid(1), 2 * BeQt::Second, 30 * BeQt::Second, &out, doc->codec());
    if (r)
        return bRet(ok, false, QString());
    return bRet(ok, true, out);
}

static QString macroExecuteD(const QStringList &args, BAbstractCodeEditorDocument *doc, bool *ok)
{
    if (!doc || args.size() < 1)
        return bRet(ok, false, QString());
    QString cmd = Global::externalTools().value(args.first());
    if (cmd.isEmpty())
        return bRet(ok, false, QString());
    QFileInfo fi(doc->fileName());
    QString path;
    if (fi.exists() && fi.isFile())
        path = fi.path();
    bool b = path.isEmpty() ? QProcess::startDetached(cmd, args.mid(1), path) :
                              QProcess::startDetached(cmd, args.mid(1));
    return bRet(ok, b, QString());
}

static QString macroExecuteP(const QStringList &args, BAbstractCodeEditorDocument *doc, bool *ok)
{
    if (!doc || args.size() < 2)
        return bRet(ok, false, QString());
    QString cmd = Global::externalTools().value(args.first());
    if (cmd.isEmpty())
        return bRet(ok, false, QString());
    QFileInfo fi(doc->fileName());
    if (!fi.exists() || !fi.isFile())
        return bRet(ok, false, QString());
    QString path = fi.path();
    QString fn = path + "/" + args.at(1);
    fi.setFile(fn);
    if (!fi.exists() || !fi.isFile())
        return bRet(ok, false, QString());
    QString out;
    QStringList nargs = args.mid(2);
    nargs.prepend(fn);
    if (BeQt::execProcess(path, cmd, nargs, 2 * BeQt::Second, 30 * BeQt::Second, &out, doc->codec()))
        return bRet(ok, false, QString());
    return bRet(ok, true, out);
}

static QString macroInsert(const QStringList &args, BAbstractCodeEditorDocument *doc, bool *ok)
{
    if (!doc || args.size() != 1)
        return bRet(ok, false, QString());
    doc->insertText(args.first());
    return bRet(ok, true, QString());
}

static QString macroWait(const QStringList &args, BAbstractCodeEditorDocument *, bool *ok)
{
    if (args.size() < 1)
        return bRet(ok, false, QString());
    bool b = false;
    int n = args.first().toInt(&b);
    if (!b || n < 0)
        return bRet(ok, false, QString());
    QStringList aargs = args.mid(1);
    int k = BeQt::Second;
    if (aargs.contains("ms") || aargs.contains("milliseconds"))
        k = 1;
    else if (aargs.contains("m") || aargs.contains("min") || aargs.contains("minutes"))
        k = BeQt::Minute;
    BeQt::waitNonBlocking(k * n);
    return bRet(ok, true, QString());
}

static QString macroFind(const QStringList &args, BAbstractCodeEditorDocument *doc, bool *ok)
{
    if (!doc || args.size() < 1)
        return bRet(ok, false, QString());
    QStringList aargs = args.mid(1);
    QTextDocument::FindFlags flags = 0;
    if (aargs.contains("cs") || aargs.contains("case-sensitive"))
        flags |= QTextDocument::FindCaseSensitively;
    if (aargs.contains("w") || aargs.contains("words"))
        flags |= QTextDocument::FindWholeWords;
    if (aargs.contains("bw") || args.contains("backward"))
        flags |= QTextDocument::FindBackward;
    bool c = true;
    if (aargs.contains("nc") || aargs.contains("non-cyclic"))
        c = false;
    bool b = doc->findNext(args.first(), flags, c);
    return bRet(ok, true, QString(b ? "true" : "false"));
}

static QString macroReplace(const QStringList &args, BAbstractCodeEditorDocument *doc, bool *ok)
{
    if (!doc || args.size() < 2)
        return bRet(ok, false, QString());
    QStringList aargs = args.mid(2);
    QTextDocument::FindFlags flags = 0;
    if (aargs.contains("cs") || aargs.contains("case-sensitive"))
        flags |= QTextDocument::FindCaseSensitively;
    if (aargs.contains("w") || aargs.contains("words"))
        flags |= QTextDocument::FindWholeWords;
    if (aargs.contains("bw") || args.contains("backward"))
        flags |= QTextDocument::FindBackward;
    bool c = true;
    if (aargs.contains("nc") || aargs.contains("non-cyclic"))
        c = false;
    bool b = doc->findNext(args.first(), flags, c);
    if (!b)
        return bRet(ok, true, QString("false"));
    b = doc->replaceNext(args.at(1));
    return bRet(ok, true, QString(b ? "true" : "false"));
}

static QString macroReplaceNext(const QStringList &args, BAbstractCodeEditorDocument *doc, bool *ok)
{
    if (!doc || args.size() < 1)
        return bRet(ok, false, QString());
    bool b = doc->replaceNext(args.first());
    return bRet(ok, true, QString(b ? "true" : "false"));
}

static QString macroReplaceDoc(const QStringList &args, BAbstractCodeEditorDocument *doc, bool *ok)
{
    if (!doc || args.size() < 2)
        return bRet(ok, false, QString());
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    QStringList aargs = args.mid(2);
    if (aargs.contains("cs") || aargs.contains("case-sensitive"))
        cs = Qt::CaseSensitive;
    int n = doc->replaceInDocument(args.at(0), args.at(1), cs);
    return bRet(ok, true, QString::number(n));
}

static QString executeMacroCommandArgument(const QString &arg, BAbstractCodeEditorDocument *doc, bool *ok = 0)
{
    typedef QString (*Function)(const QStringList &, BAbstractCodeEditorDocument *, bool *);
    typedef QMap<QString, Function> FunctionMap;
    init_once(FunctionMap, fmap, FunctionMap())
    {
        fmap.insert("execute", &macroExecute);
        fmap.insert("executeD", &macroExecuteD);
        fmap.insert("executeP", &macroExecuteP);
        fmap.insert("insert", &macroInsert);
        fmap.insert("wait", &macroWait);
        fmap.insert("find", &macroFind);
        fmap.insert("replace", &macroReplace);
        fmap.insert("replaceNext", &macroReplaceNext);
        fmap.insert("replaceDoc", &macroReplaceDoc);
    }
    if (!arg.startsWith("\\"))
        return bRet(ok, true, arg);
    QString fn;
    int i = 1;
    while (i < arg.length() && arg.at(i) != '{')
        fn += arg.at(i++);
    Function f = fmap.value(fn);
    if (!f)
        return "";
    QStringList sl;
    int depth = 1;
    QString s;
    ++i;
    while (i < arg.length() && arg.at(i) != '[')
    {
        if (arg.at(i) == '}')
        {
            --depth;
            if (depth)
                s += '}';
        }
        else if (arg.at(i) == '{')
        {
            if (depth)
                s += '{';
            ++depth;
        }
        else
            s += arg.at(i);
        if (!depth)
        {
            bool b = false;
            QString a = executeMacroCommandArgument(s, doc, &b);
            if (b)
                sl << a;
            else
                return bRet(ok, false, QString());
            s.clear();
        }
        ++i;
    }
    if (!s.isEmpty())
        return "";
    depth = 1;
    ++i;
    while (i < arg.length())
    {
        if (arg.at(i) == ']')
        {
            --depth;
            if (depth)
                s += ']';
        }
        else if (arg.at(i) == '[')
        {
            ++depth;
            if (depth)
                s += '[';
        }
        else
            s += arg.at(i);
        if (!depth)
        {
            bool b = false;
            QString a = executeMacroCommandArgument(s, doc, &b);
            if (b)
                sl << a;
            else
                return bRet(ok, false, QString());
            s.clear();
        }
        ++i;
    }
    if (!s.isEmpty())
        return "";
    return f(sl, doc, ok);
}

static void executeMacroCommand(const QString &command, BAbstractCodeEditorDocument *doc)
{
    executeMacroCommandArgument(command, doc);
}

/*============================================================================
================================ TeXCreatorMacroFileType =====================
============================================================================*/

/*============================== Public constructors =======================*/

TeXCreatorMacroFileType::TeXCreatorMacroFileType()
{
    //
}

TeXCreatorMacroFileType::~TeXCreatorMacroFileType()
{
    //
}

/*============================== Public methods ============================*/

QString TeXCreatorMacroFileType::id() const
{
    return "TeX Creator macro";
}

QString TeXCreatorMacroFileType::name() const
{
    return tr("TeX Creator macro", "name");
}

QString TeXCreatorMacroFileType::description() const
{
    return tr("TeX Creator macro files", "description");
}

QStringList TeXCreatorMacroFileType::suffixes() const
{
    return QStringList() << "tcm";
}

bool TeXCreatorMacroFileType::matchesFileName(const QString &fileName) const
{
    return suffixes().contains(QFileInfo(fileName).suffix(), Qt::CaseInsensitive);
}

BAbstractFileType::BracketPairList TeXCreatorMacroFileType::brackets() const
{
    BracketPairList list;
    list << createBracketPair("{", "}", "\\");
    return list;
}

/*============================== Protected methods =========================*/

void TeXCreatorMacroFileType::highlightBlock(const QString &text)
{
    //comments
    int comInd = text.indexOf('%');
    while (comInd > 0 && text.at(comInd - 1) == '\\')
        comInd = text.indexOf('%', comInd + 1);
    BCodeEdit::setBlockComment(currentBlock(), comInd);
    if (comInd >= 0)
        setFormat(comInd, text.length() - comInd, QColor(Qt::darkGray));
    QString ntext = text.left(comInd);
    //commands
    QRegExp rx("(\\\\[a-zA-Z]*|\\\\#|\\\\\\$|\\\\%|\\\\&|\\\\_|\\\\\\{|\\\\\\})+");
    int pos = rx.indexIn(ntext);
    while (pos >= 0)
    {
        int len = rx.matchedLength();
        setFormat(pos, len, QColor(Qt::red).lighter(70));
        pos = rx.indexIn(ntext, pos + len);
    }
}

/*============================================================================
================================ MacroCommand ================================
============================================================================*/

/*============================== Public constructors =======================*/

MacroCommand::MacroCommand(const QString &t)
{
    init();
    fromText(t);
}

MacroCommand::MacroCommand(const QEvent *e)
{
    init();
    fromKeyPress(e);
}

MacroCommand::MacroCommand(const MacroCommand &other)
{
    init();
    *this = other;
}

/*============================== Public methods ============================*/

void MacroCommand::clear()
{
    key = 0;
    modifiers = 0;
    text.clear();
    command.clear();
}

void MacroCommand::execute(BAbstractCodeEditorDocument *doc) const
{
    if (!doc || !isValid())
        return;
    if (key)
        QApplication::postEvent(doc->findChild<QPlainTextEdit *>(),
                                new QKeyEvent(QEvent::KeyPress, key, modifiers, text));
    else if (!command.isEmpty())
        executeMacroCommand(command, doc);
}

bool MacroCommand::fromText(const QString &t)
{
    clear();
    if (t.isEmpty())
        return false;
    if (t.startsWith("\\") && !t.startsWith("\\ "))
    {
        command = t; //TODO: Check validity
    }
    else
    {
        QStringList sl = t.split(QRegExp("\\s+"));
        if (!bRangeD(1, 2).contains(sl.size()))
            return false;
        QKeySequence ks(sl.first(), QKeySequence::PortableText);
        if (ks.isEmpty())
            return false;
        key = ~Qt::KeyboardModifierMask & ks[0];
        modifiers = static_cast<Qt::KeyboardModifiers>(Qt::KeyboardModifierMask & ks[0]);
        if (sl.size() == 2)
            text = sl.last();
    }
    return true;
}

bool MacroCommand::fromKeyPress(const QEvent *e)
{
    clear();
    if (!e || e->type() != QEvent::KeyPress)
        return false;
    const QKeyEvent *ke = static_cast<const QKeyEvent *>(e);
    int k = ke->key();
    if (Qt::Key_Control == k || Qt::Key_Alt == k || Qt::Key_Shift == k)
        return false;
    key = k;
    modifiers = ke->modifiers();
    text = ke->text();
    return true;
}

QString MacroCommand::toText() const
{
    if (key)
    {
        QString s = QKeySequence(key | modifiers).toString(QKeySequence::PortableText);
        if (!text.isEmpty() && text.at(0).isPrint() && !text.at(0).isSpace() &&
             !(modifiers & Qt::ControlModifier) && !(modifiers & Qt::AltModifier))
            s += " " + text;
        return s;
    }
    else
    {
        return command;
    }
}

bool MacroCommand::isValid() const
{
    return key || !command.isEmpty();
}

/*============================== Public operators ==========================*/

MacroCommand &MacroCommand::operator =(const MacroCommand &other)
{
    key = other.key;
    modifiers = other.modifiers;
    text = other.text;
    command = other.command;
    return *this;
}

/*============================== Private methods ===========================*/

void MacroCommand::init()
{
    key = 0;
    modifiers = 0;
}

/*============================================================================
================================ Macro =======================================
============================================================================*/

/*============================== Public constructors =======================*/

Macro::Macro(const QString &fileName)
{
    fromFile(fileName);
}

Macro::Macro(const Macro &other)
{
    *this = other;
}

/*============================== Public methods ============================*/

void Macro::clear()
{
    mcommands.clear();
}

void Macro::execute(BAbstractCodeEditorDocument *doc, BSimpleCodeEditorDocument *cedt) const
{
    if (!doc || !isValid())
        return;
    QPoint ss;
    QPoint se;
    QPoint p;
    if (cedt)
    {
        ss = cedt->selectionStart();
        se = cedt->selectionEnd();
        p = cedt->cursorPosition();
    }
    int n = 0;
    foreach (const MacroCommand &c, mcommands)
    {
        if (cedt)
        {
            cedt->selectLines(n, n);
            ++n;
        }
        c.execute(doc);
        QApplication::processEvents();
    }
    if (cedt)
    {
        cedt->moveCursor(p);
        cedt->selectText(ss, se);
    }
}

bool Macro::recordKeyPress(const QEvent *e, QString *s)
{
    MacroCommand c(e);
    if (!c.isValid())
        return false;
    mcommands << c;
    return bRet(s, c.toText(), true);
}

bool Macro::fromText(const QString &text)
{
    clear();
    QStringList sl = text.split('\n', QString::SkipEmptyParts);
    foreach (const QString &s, sl)
    {
        if (s.startsWith("%"))
            continue;
        MacroCommand c(s);
        if (!c.isValid())
            continue;
        mcommands << c;
    }
    return true;
}

bool Macro::fromFile(const QString &fileName)
{
    clear();
    if (fileName.isEmpty())
        return false;
    bool ok = false;
    QString text = BDirTools::readTextFile(fileName, "UTF-8", &ok);
    return ok && fromText(text);
}

QString Macro::toText() const
{
    QString s;
    foreach (const MacroCommand &c, mcommands)
        s += (c.toText() + "\n");
    if (!s.isEmpty())
        s.remove(s.length() - 1, 1);
    return s;
}

bool Macro::toFile(const QString &fileName) const
{
    if (fileName.isEmpty() || !isValid())
        return false;
    return BDirTools::writeTextFile(fileName, toText(), "UTF-8");
}

bool Macro::isValid() const
{
    return !mcommands.isEmpty();
}

/*============================== Public operators ==========================*/

Macro &Macro::operator=(const Macro &other)
{
    mcommands = other.mcommands;
    return *this;
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
    mft = new TeXCreatorMacroFileType;
    //
    mactStartStop = new QAction(this);
      connect(mactStartStop.data(), SIGNAL(triggered()), this, SLOT(startStopRecording()));
    mactClear = new QAction(this);
      mactClear->setIcon(Application::icon("editclear"));
      connect(mactClear.data(), SIGNAL(triggered()), this, SLOT(clearMacro()));
    mactPlay = new QAction(this);
      mactPlay->setIcon(Application::icon("player_play"));
      connect(mactPlay.data(), SIGNAL(triggered()), this, SLOT(playMacro()));
      QMenu *mnu = new QMenu;
        mactPlay5 = new QAction(this);
          connect(mactPlay5.data(), SIGNAL(triggered()), this, SLOT(playMacro5()));
        mnu->addAction(mactPlay5.data());
        mactPlay10 = new QAction(this);
          connect(mactPlay10.data(), SIGNAL(triggered()), this, SLOT(playMacro10()));
        mnu->addAction(mactPlay10.data());
        mactPlay20 = new QAction(this);
          connect(mactPlay20.data(), SIGNAL(triggered()), this, SLOT(playMacro20()));
        mnu->addAction(mactPlay20.data());
        mactPlay50 = new QAction(this);
          connect(mactPlay50.data(), SIGNAL(triggered()), this, SLOT(playMacro50()));
        mnu->addAction(mactPlay50.data());
        mactPlay100 = new QAction(this);
          connect(mactPlay100.data(), SIGNAL(triggered()), this, SLOT(playMacro100()));
        mnu->addAction(mactPlay100.data());
        mactPlayN = new QAction(this);
          connect(mactPlayN.data(), SIGNAL(triggered()), this, SLOT(playMacroN()));
        mnu->addAction(mactPlayN.data());
      mactPlay->setMenu(mnu);
    mactShowHide = new QAction(this);
      connect(mactShowHide.data(), SIGNAL(triggered()), this, SLOT(showHideMacrosConsole()));
    mactLoad = new QAction(this);
      mactLoad->setIcon(Application::icon("fileopen"));
      connect(mactLoad.data(), SIGNAL(triggered()), this, SLOT(loadMacro()));
    mactSaveAs = new QAction(this);
      mactSaveAs->setIcon(Application::icon("filesaveas"));
      connect(mactSaveAs.data(), SIGNAL(triggered()), this, SLOT(saveMacroAs()));
    mactOpenDir = new QAction(this);
      mactOpenDir->setIcon(Application::icon("folder_open"));
      connect(mactOpenDir.data(), SIGNAL(triggered()), this, SLOT(openUserDir()));
    mspltr = new QSplitter(Qt::Horizontal);
      mspltr->setFixedHeight(100);
      mcedt = new BSimpleCodeEditorDocument(0);
        mcedt->init();
        mcedt->setFileType(mft);
        connect(mcedt->findChild<QPlainTextEdit *>(), SIGNAL(textChanged()), this, SLOT(ptedtTextChanged()));
      mspltr->addWidget(mcedt.data());
      mlstwgt = new QListWidget;
      connect(mlstwgt.data(), SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
              this, SLOT(lstwgtCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));
      mspltr->addWidget(mlstwgt.data());
    //
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
    retranslateUi();
    connect(bApp, SIGNAL(reloadMacros()), this, SLOT(reloadMacros()));
    reloadMacros();
}
MacrosEditorModule::~MacrosEditorModule()
{
    if (!mspltr.isNull())
        delete mspltr;
    if (mft)
        delete mft;
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
    if (!mmacro.recordKeyPress(e, &txt))
        return false;
    if (!mcedt.isNull())
        appendPtedtText(txt);
    return false;
}

QByteArray MacrosEditorModule::saveState() const
{
    QVariantMap m;
    m.insert("splitter_state", !mspltr.isNull() ? mspltr->saveState() : QByteArray());
    return BeQt::serialize(m);
}

void MacrosEditorModule::restoreState(const QByteArray &state)
{
    QVariantMap m = BeQt::deserialize(state).toMap();
    if (!mspltr.isNull())
        mspltr->restoreState(m.value("splitter_state").toByteArray());
}

/*============================== Public slots ==============================*/

void MacrosEditorModule::startStopRecording()
{
    if (mplaying)
        return;
    mrecording = !mrecording;
    if (mrecording)
        clearMacro();
    resetStartStopAction();
    checkActions();
}

void MacrosEditorModule::clearMacro()
{
    if (mplaying)
        return;
    mmacro.clear();
    if (!mcedt.isNull())
        clearPtedt();
    checkActions();
}

void MacrosEditorModule::playMacro(int n)
{
    if (n <= 0)
        n = 1;
    BAbstractCodeEditorDocument *doc = currentDocument();
    if (!doc || mplaying || mrecording || !mmacro.isValid())
        return;
    mplaying = true;
    checkActions();
    if (!mcedt.isNull())
        mcedt->setReadOnly(true);
    for (int i = 0; i < n; ++i)
        mmacro.execute(doc, mcedt.data());
    if (!mcedt.isNull())
        mcedt->setReadOnly(false);
    mplaying = false;
    checkActions();
}

void MacrosEditorModule::playMacro5()
{
    playMacro(5);
}

void MacrosEditorModule::playMacro10()
{
    playMacro(10);
}

void MacrosEditorModule::playMacro20()
{
    playMacro(20);
}

void MacrosEditorModule::playMacro50()
{
    playMacro(50);
}

void MacrosEditorModule::playMacro100()
{
    playMacro(100);
}

void MacrosEditorModule::playMacroN()
{
    bool ok = false;
    int n = QInputDialog::getInt(editor(), tr("Enter a number", "idlg title"), tr("Number of iterations:", "lbl text"),
                                 1, 1, INT_MAX, 1, &ok);
    if (!ok)
        return;
    playMacro(n);
}

void MacrosEditorModule::showHideMacrosConsole()
{
    if (mspltr.isNull() || !editor())
        return;
    mspltr->setVisible(!mspltr->isVisible());
    resetShowHideAction();
}

bool MacrosEditorModule::loadMacro(const QString &fileName)
{
    if (mplaying || mrecording)
        return false;
    QString fn = fileName;
    if (fn.isEmpty())
        fn = QFileDialog::getOpenFileName(
                    editor(), tr("Open", "fdlg caption"),
                    BDirTools::findResource("macros", BDirTools::UserOnly), fileDialogFilter());
    bool b = !fn.isEmpty() && mmacro.fromFile(fn);
    if (b && !mcedt.isNull())
        setPtedtText(mmacro.toText());
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
                    BDirTools::findResource("macros", BDirTools::UserOnly), fileDialogFilter());
    if (!fn.isEmpty() && QFileInfo(fn).suffix().compare("tcm", Qt::CaseInsensitive))
        fn += ".tcm";
    return !fn.isEmpty() && mmacro.toFile(fn);
}

void MacrosEditorModule::openUserDir()
{
    bApp->openLocalFile(BDirTools::findResource("macros"));
}

void MacrosEditorModule::reloadMacros()
{
    if (mcedt.isNull() || mlstwgt.isNull())
        return;
    //if (mcedt->isModified())
        //;
    //
    mlstwgt->clear();
    mcedt->setText("");
    QSet<QString> fnames;
    foreach (const QString &path, Application::locations("macros"))
    {
        foreach (const QString &fn, BDirTools::entryList(path, QStringList() << "*.tcm", QDir::Files))
        {
            if (fnames.contains(fn))
                continue;
            fnames.insert(fn);
            QString text = BDirTools::readTextFile(fn, "UTF-8");
            QListWidgetItem *lwi = new QListWidgetItem(QFileInfo(fn).baseName());
            lwi->setData(Qt::UserRole, text);
            mlstwgt->addItem(lwi);
        }
    }
}

/*============================== Protected methods =========================*/

void MacrosEditorModule::editorSet(BCodeEditor *edr)
{
    if (edr && !mspltr.isNull())
    {
        QVBoxLayout *vlt = static_cast<QVBoxLayout *>(edr->layout());
        vlt->insertWidget(0, mspltr.data());
        mspltr->hide();
    }
    resetStartStopAction();
    resetShowHideAction();
    checkActions();
}

void MacrosEditorModule::editorUnset(BCodeEditor *edr)
{
    if (edr && !mspltr.isNull())
    {
        mspltr->hide();
        QVBoxLayout *vlt = static_cast<QVBoxLayout *>(edr->layout());
        vlt->removeWidget(mspltr.data());
        mspltr->setParent(0);
    }
    resetStartStopAction();
    resetShowHideAction();
    checkActions();
}

void MacrosEditorModule::currentDocumentChanged(BAbstractCodeEditorDocument *doc)
{
    if (mprevDoc)
        mprevDoc->findChild<QPlainTextEdit *>()->removeEventFilter(this);
    mprevDoc = doc;
    if (doc)
        doc->findChild<QPlainTextEdit *>()->installEventFilter(this);
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
    return tr("TeX Creator macros", "fdlg filter") + " (*.tcm)";
}

/*============================== Private methods ===========================*/

void MacrosEditorModule::resetStartStopAction()
{
    if (mactStartStop.isNull())
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
    if (mactShowHide.isNull())
        return;
    if (!mspltr.isNull() && mspltr->isVisible())
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

void MacrosEditorModule::appendPtedtText(const QString &text)
{
    if (mcedt.isNull())
        return;
    QPlainTextEdit *ptedt = mcedt->findChild<QPlainTextEdit *>();
    ptedt->blockSignals(true);
    mcedt->setText(mcedt->text() + "\n" + text);
    ptedt->blockSignals(false);
}

void MacrosEditorModule::setPtedtText(const QString &text)
{
    if (mcedt.isNull())
        return;
    QPlainTextEdit *ptedt = mcedt->findChild<QPlainTextEdit *>();
    ptedt->blockSignals(true);
    mcedt->setText(text);
    ptedt->blockSignals(false);
}

void MacrosEditorModule::clearPtedt()
{
    if (mcedt.isNull())
        return;
    QPlainTextEdit *ptedt = mcedt->findChild<QPlainTextEdit *>();
    ptedt->blockSignals(true);
    mcedt->setText("");
    ptedt->blockSignals(false);
}

/*============================== Private slots =============================*/

void MacrosEditorModule::retranslateUi()
{
    if (!mactClear.isNull())
    {
        mactClear->setText(tr("Clear", "act text"));
        mactClear->setToolTip(tr("Clear current macro", "act toolTip"));
        mactClear->setWhatsThis(tr("Use this action to clear currntly loaded or recorded macro. "
                                   "The corresponding file will not be deleted", "act whatsThis"));
    }
    if (!mactPlay.isNull())
    {
        mactPlay->setText(tr("Play", "act text"));
        mactPlay->setToolTip(tr("Play current macro", "act toolTip"));
        mactPlay->setWhatsThis(tr("Use this action to activate previously loaded or recorded macro", "act whatsThis"));
    }
    if (!mactPlay5.isNull())
        mactPlay5->setText(tr("Play 5 times", "act text"));
    if (!mactPlay10.isNull())
        mactPlay10->setText(tr("Play 10 times", "act text"));
    if (!mactPlay20.isNull())
        mactPlay20->setText(tr("Play 20 times", "act text"));
    if (!mactPlay50.isNull())
        mactPlay50->setText(tr("Play 50 times", "act text"));
    if (!mactPlay100.isNull())
        mactPlay100->setText(tr("Play 100 times", "act text"));
    if (!mactPlayN.isNull())
        mactPlayN->setText(tr("Play N times", "act text"));
    if (!mactLoad.isNull())
    {
        mactLoad->setText(tr("Load...", "act text"));
        mactLoad->setToolTip(tr("Load macro", "act toolTip"));
        mactLoad->setWhatsThis(tr("Use this action to load previously saved macro from file", "act whatsThis"));
    }
    if (!mactSaveAs.isNull())
    {
        mactSaveAs->setText(tr("Save as...", "act text"));
        mactSaveAs->setToolTip(tr("Save current macro as...", "act toolTip"));
        mactSaveAs->setWhatsThis(tr("Use this action to save current macro to a file", "act whatsThis"));
    }
    if (!mactOpenDir.isNull())
    {
        mactOpenDir->setText(tr("Open user macros dir", "act text"));
        //TODO: toolTip and whatsThis
    }
    resetStartStopAction();
    resetShowHideAction();
}

void MacrosEditorModule::ptedtTextChanged()
{
    if (mcedt.isNull())
        return;
    mmacro.fromText(mcedt->text());
    checkActions();
}

void MacrosEditorModule::lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (mcedt.isNull())
        return;
    //TODO
    //
    if (!current)
        return;
    QString text = current->data(Qt::UserRole).toString();
    mmacro.fromText(text);
    mcedt->setText(text);
    checkActions();
}
