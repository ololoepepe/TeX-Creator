#include "macrocommand.h"
#include "macrocommandargument.h"
#include "global.h"
#include "macroexecutionstack.h"

#include <BAbstractCodeEditorDocument>
#include <BeQt>

#include <QString>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QChar>
#include <QList>
#include <QMap>
#include <QRegExp>
#include <QCoreApplication>
#include <QPlainTextEdit>
#include <QThread>

/*============================================================================
================================ ThreadHack ==================================
============================================================================*/

class ThreadHack : public QThread
{
public:
    static void waitBlocking(int msecs);
};

/*============================================================================
================================ FunctionInfo ================================
============================================================================*/

struct FunctionInfo
{
public:
    typedef AbstractMacroCommand *(*Function)(const QList<MacroCommandArgument> &);
public:
    Function func;
    int mndArgCount;
    int optArgCount;
public:
    explicit FunctionInfo();
    explicit FunctionInfo(Function f, int mnd = 0, int opt = 0);
public:
    bool isValid() const;
};

/*============================================================================
================================ MultiArgMacroCommand ========================
============================================================================*/

class MultiArgMacroCommand : public AbstractMacroCommand
{
public:
    ~MultiArgMacroCommand();
protected:
    explicit MultiArgMacroCommand(const QList<MacroCommandArgument> &args);
public:
    void clear();
    bool isValid() const;
protected:
    bool compareInternal(const AbstractMacroCommand *other) const;
protected:
    QList<MacroCommandArgument> margs;
};

/*============================================================================
================================ InsertMacroCommand ==========================
============================================================================*/

class InsertMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
public:
    explicit InsertMacroCommand(const QString &txt);
    ~InsertMacroCommand();
private:
    explicit InsertMacroCommand(const MacroCommandArgument &a);

public:
    void clear();
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    bool isValid() const;
    AbstractMacroCommand *clone() const;
    bool append(const QString &txt);
protected:
    bool compareInternal(const AbstractMacroCommand *other) const;
private:
    MacroCommandArgument arg;
};

/*============================================================================
================================ PressMacroCommand ===========================
============================================================================*/

class PressMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
public:
    explicit PressMacroCommand(int k, Qt::KeyboardModifiers m);
    ~PressMacroCommand();
private:
    explicit PressMacroCommand(const MacroCommandArgument &a);
public:
    void clear();
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    bool isValid() const;
    AbstractMacroCommand *clone() const;
protected:
    bool compareInternal(const AbstractMacroCommand *other) const;
private:
    MacroCommandArgument arg;
};

/*============================================================================
================================ WaitMacroCommand ============================
============================================================================*/

class WaitMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit WaitMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ FindMacroCommand ============================
============================================================================*/

class FindMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit FindMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ReplaceMacroCommand =========================
============================================================================*/

class ReplaceMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ReplaceMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ReplaceSelMacroCommand ======================
============================================================================*/

class ReplaceSelMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ReplaceSelMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ReplaceDocMacroCommand ======================
============================================================================*/

class ReplaceDocMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ReplaceDocMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ExecMacroCommand ============================
============================================================================*/

class ExecMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ExecMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ExecFMacroCommand ===========================
============================================================================*/

class ExecFMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ExecFMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ExecDMacroCommand ===========================
============================================================================*/

class ExecDMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ExecDMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ExecFDMacroCommand ===========================
============================================================================*/

class ExecFDMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ExecFDMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static QStringList getArgs(const QString &text, int &i, char opbr, char clbr, int max, char nopbr = '\0',
                           QString *error = 0)
{
    if (text.isEmpty() || i < 0 || max < 0)
        return bRet(error, QString("Argument parcing error"), QStringList());
    QStringList args;
    int depth = 1;
    QString s;
    while (i < text.length() && text.at(i) != nopbr)
    {
        if (text.at(i) == clbr)
        {
            --depth;
            if (depth)
                s += clbr;
        }
        else if (text.at(i) == opbr)
        {
            if (depth)
                s += opbr;
            ++depth;
        }
        else
            s += text.at(i);
        if (!depth)
        {
            args << s;
            if (max >= 0 && args.size() > max)
                return bRet(error, QString("Too many arguments"), QStringList());
            s.clear();
        }
        ++i;
    }
    if (!s.isEmpty())
        return bRet(error, QString("Argument parcing error"), QStringList());
    return bRet(error, QString(), args);
}

/*============================================================================
================================ AbstractMacroCommand ========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *AbstractMacroCommand::fromText(QString text, QString *error)
{
    typedef QMap<QString, FunctionInfo> FunctionInfoMap;
    init_once(FunctionInfoMap, infoMap, FunctionInfoMap())
    {
        infoMap.insert("insert", FunctionInfo(&InsertMacroCommand::create, 1));
        infoMap.insert("press", FunctionInfo(&PressMacroCommand::create, 1));
        infoMap.insert("wait", FunctionInfo(&WaitMacroCommand::create, 1, 2));
        infoMap.insert("find", FunctionInfo(&FindMacroCommand::create, 1, 4));
        infoMap.insert("replace", FunctionInfo(&ReplaceMacroCommand::create, 2, 4));
        infoMap.insert("replaceSel", FunctionInfo(&ReplaceSelMacroCommand::create, 2, 1));
        infoMap.insert("replaceDoc", FunctionInfo(&ReplaceDocMacroCommand::create, 2, 1));
        infoMap.insert("exec", FunctionInfo(&ExecMacroCommand::create, 1, -1));
        infoMap.insert("execF", FunctionInfo(&ExecFMacroCommand::create, 2, -1));
        infoMap.insert("execD", FunctionInfo(&ExecDMacroCommand::create, 1, -1));
        infoMap.insert("execFD", FunctionInfo(&ExecFDMacroCommand::create, 2, -1));
    }
    text.remove(QRegExp("%.*"));
    if (text.isEmpty())
        return bRet(error, QString("Empty string"), (AbstractMacroCommand *) 0);
    if (!text.startsWith('\\'))
        return bRet(error, QString("A command must start with \"\\\""), (AbstractMacroCommand *) 0);
    QString commandName;
    int i = Global::indexOfHelper(text, "{", 2);
    commandName += text.mid(1, (i > 0) ? (i - 1) : -1);
    FunctionInfo info = infoMap.value(commandName);
    if (!info.isValid())
        return bRet(error, QString("Invalid function name"), (AbstractMacroCommand *) 0);
    QString err;
    QStringList textArgs = getArgs(text, ++i, '{', '}', info.mndArgCount, '[', &err);
    if (!err.isEmpty())
        return bRet(error, err, (AbstractMacroCommand *) 0);
    textArgs += getArgs(text, ++i, '[', ']', info.optArgCount, '\0', &err);
    if (!err.isEmpty())
        return bRet(error, err, (AbstractMacroCommand *) 0);
    QList<MacroCommandArgument> args;
    foreach (const QString &t, textArgs)
    {
        MacroCommandArgument arg(t);
        if (!arg.isValid())
            return bRet(error, arg.error(), (AbstractMacroCommand *) 0);
        args << arg;
    }
    return bRet(error, QString(), info.func(args));
}

AbstractMacroCommand *AbstractMacroCommand::fromKeyPressEvent(const QKeyEvent *e,
                                                              AbstractMacroCommand *previousCommand)
{
    return fromKeyPressEvent(e, 0, previousCommand);
}

AbstractMacroCommand *AbstractMacroCommand::fromKeyPressEvent(const QKeyEvent *e, QString *error,
                                                              AbstractMacroCommand *previousCommand)
{
    if (!e || e->type() != QEvent::KeyPress)
        return bRet(error, QString("The event is not a key press"), (AbstractMacroCommand *) 0);
    int key = e->key();
    Qt::KeyboardModifiers modifiers = e->modifiers();
    QString text = e->text();
    if (Qt::Key_Control == key || Qt::Key_Alt == key || Qt::Key_Shift == key)
        return bRet(error, QString("The event is a modifier key press"), (AbstractMacroCommand *) 0);
    if (text.isEmpty() || !text.at(0).isPrint() || text.at(0).isSpace()
            || (modifiers & Qt::ControlModifier) || (modifiers & Qt::AltModifier))
        return bRet(error, QString(), new PressMacroCommand(key, modifiers));
    if (!previousCommand || previousCommand->name() != "insert")
        return bRet(error, QString(), new InsertMacroCommand(text));
    if (!dynamic_cast<InsertMacroCommand *>(previousCommand)->append(text))
        return bRet(error, QString(), new InsertMacroCommand(text));
    return bRet(error, QString(), previousCommand);
}

/*============================== Public constructors =======================*/

AbstractMacroCommand::AbstractMacroCommand()
{
    //
}

AbstractMacroCommand::~AbstractMacroCommand()
{
    //
}

/*============================== Public methods ============================*/

void AbstractMacroCommand::clear()
{
    //
}

bool AbstractMacroCommand::compare(const AbstractMacroCommand *other) const
{
    if (!other || other->name() != name())
        return false;
    return other && other->name() == name() && compareInternal(other);
}

/*============================================================================
================================ FunctionInfo ================================
============================================================================*/

/*============================== Public constructors =======================*/

FunctionInfo::FunctionInfo()
{
    func = 0;
    mndArgCount = 0;
    optArgCount = 0;
}
FunctionInfo::FunctionInfo(Function f, int mnd, int opt)
{
    func = f;
    mndArgCount = mnd;
    optArgCount = opt;
}

/*============================== Public methods ============================*/

bool FunctionInfo::isValid() const
{
    return func && mndArgCount >= 0;
}

/*============================================================================
================================ ThreadHack ==================================
============================================================================*/

/*============================== Static public methods =====================*/

void ThreadHack::waitBlocking(int msecs)
{
    currentThread()->msleep(msecs);
}

/*============================================================================
================================ MultiArgMacroCommand ========================
============================================================================*/

/*============================== Public constructors =======================*/

MultiArgMacroCommand::~MultiArgMacroCommand()
{
    //
}

/*============================== Protected constructors ====================*/

MultiArgMacroCommand::MultiArgMacroCommand(const QList<MacroCommandArgument> &args)
{
    margs = args;
}

/*============================== Public methods ============================*/

void MultiArgMacroCommand::clear()
{
    margs.clear();
}

bool MultiArgMacroCommand::isValid() const
{
    if (margs.isEmpty())
        return false;
    foreach (const MacroCommandArgument &a, margs)
        if (!a.isValid())
            return false;
    return true;
}

/*============================== Protected methods =========================*/

bool MultiArgMacroCommand::compareInternal(const AbstractMacroCommand *other) const
{
    const MultiArgMacroCommand *o = dynamic_cast<const MultiArgMacroCommand *>(other);
    return o && margs == o->margs;
}

/*============================================================================
================================ InsertMacroCommand ==========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *InsertMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 1) ? new InsertMacroCommand(args.first()) : 0;
}

/*============================== Public constructors =======================*/

InsertMacroCommand::InsertMacroCommand(const QString &txt)
{
    arg.fromText(txt);
}

InsertMacroCommand::~InsertMacroCommand()
{
    //
}

/*============================== Private constructors ======================*/

InsertMacroCommand::InsertMacroCommand(const MacroCommandArgument &a)
{
    arg = a;
}

/*============================== Public methods ============================*/

void InsertMacroCommand::clear()
{
    arg.clear();
}

QString InsertMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    if (!doc || !stack || !arg.isValid())
        return "false";
    doc->insertText(arg.toText(doc, stack));
    return "true";
}

QString InsertMacroCommand::name() const
{
    return "insert";
}

QString InsertMacroCommand::toText() const
{
    return arg.isValid() ? ("\\insert{" + arg.toText() + "}") : QString();
}

bool InsertMacroCommand::isValid() const
{
    return arg.isValid();
}

AbstractMacroCommand *InsertMacroCommand::clone() const
{
    return new InsertMacroCommand(arg);
}

bool InsertMacroCommand::append(const QString &txt)
{
    if (txt.isEmpty() || !arg.isText())
        return false;
    return arg.fromText(arg.toText() + txt);
}

/*============================== Protected methods =========================*/

bool InsertMacroCommand::compareInternal(const AbstractMacroCommand *other) const
{
    const InsertMacroCommand *o = dynamic_cast<const InsertMacroCommand *>(other);
    return o && arg == o->arg;
}

/*============================================================================
================================ PressMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *PressMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 1) ? new PressMacroCommand(args.first()) : 0;
}

/*============================== Public constructors =======================*/

PressMacroCommand::PressMacroCommand(int k, Qt::KeyboardModifiers m)
{
    arg.fromText(QKeySequence(k | m).toString(QKeySequence::PortableText));
}

PressMacroCommand::~PressMacroCommand()
{
    //
}

/*============================== Private constructors ======================*/

PressMacroCommand::PressMacroCommand(const MacroCommandArgument &a)
{
    arg = a;
}

/*============================== Public methods ============================*/

void PressMacroCommand::clear()
{
    arg.clear();
}

QString PressMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    if (!doc || !stack || !arg.isValid())
        return "false";
    QPlainTextEdit *ptedt = doc->findChild<QPlainTextEdit *>();
    if (!ptedt)
        return "false";
    QKeySequence ks(arg.toText(doc, stack));
    if (ks.isEmpty())
        return "false";
    for (int i = 0; i < (int) ks.count(); ++i)
    {
        int key = ~Qt::KeyboardModifierMask & ks[i];
        Qt::KeyboardModifiers modifiers = static_cast<Qt::KeyboardModifiers>(Qt::KeyboardModifierMask & ks[i]);
        QCoreApplication::postEvent(ptedt, new QKeyEvent(QEvent::KeyPress, key, modifiers));
    }
    return "true";
}

QString PressMacroCommand::name() const
{
    return "press";
}

QString PressMacroCommand::toText() const
{
    return arg.isValid() ? ("\\press{" + arg.toText() + "}") : QString();
}

bool PressMacroCommand::isValid() const
{
    return arg.isValid();
}

AbstractMacroCommand *PressMacroCommand::clone() const
{
    return new PressMacroCommand(arg);
}

/*============================== Protected methods =========================*/

bool PressMacroCommand::compareInternal(const AbstractMacroCommand *other) const
{
    const PressMacroCommand *o = dynamic_cast<const PressMacroCommand *>(other);
    return o && arg == o->arg;
}

/*============================================================================
================================ WaitMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *WaitMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 3) ? new WaitMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

WaitMacroCommand::WaitMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString WaitMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    if (!doc || !stack || !isValid())
        return "false";
    QString ns = margs.first().toText(doc, stack);
    if (ns.isEmpty())
        return "false";
    bool ok = false;
    int n = ns.toInt(&ok);
    if (n < 0 || !ok)
        return "false";
    int k = BeQt::Second;
    bool b = false;
    bool kb = false;
    bool bb = false;
    for (int i = 1; i < margs.size(); ++i)
    {
        QString as = margs.at(1).toText(doc, stack);
        if (!as.compare("ms", Qt::CaseInsensitive) || !as.compare("milliseconds", Qt::CaseInsensitive))
        {
            if (kb)
                return "false";
            k = 1;
            kb = true;
        }
        else if (!as.compare("s", Qt::CaseInsensitive) || !as.compare("seconds", Qt::CaseInsensitive))
        {
            if (kb)
                return "false";
            k = BeQt::Second;
            kb = true;
        }
        else if (!as.compare("b", Qt::CaseInsensitive) || !as.compare("blocking", Qt::CaseInsensitive))
        {
            if (bb)
                return "false";
            b = true;
            bb = true;
        }
        else if (!as.compare("nb", Qt::CaseInsensitive) || !as.compare("non-blocking", Qt::CaseInsensitive))
        {
            if (bb)
                return "false";
            b = false;
            bb = true;
        }
    }
    if (b)
        BeQt::waitNonBlocking(k * n);
    else
        ThreadHack::waitBlocking(k * n);
    return "true";
}

QString WaitMacroCommand::name() const
{
    return "wait";
}

QString WaitMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\wait{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *WaitMacroCommand::clone() const
{
    return new WaitMacroCommand(margs);
}

/*============================================================================
================================ FindMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *FindMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 5) ? new FindMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

FindMacroCommand::FindMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString FindMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    //
}

QString FindMacroCommand::name() const
{
    //
}

QString FindMacroCommand::toText() const
{
    //
}

AbstractMacroCommand *FindMacroCommand::clone() const
{
    //
}

/*============================================================================
================================ ReplaceMacroCommand =========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ReplaceMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 6) ? new ReplaceMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ReplaceMacroCommand::ReplaceMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ReplaceMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    //
}

QString ReplaceMacroCommand::name() const
{
    //
}

QString ReplaceMacroCommand::toText() const
{
    //
}

AbstractMacroCommand *ReplaceMacroCommand::clone() const
{
    //
}

/*============================================================================
================================ ReplaceSelMacroCommand ======================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ReplaceSelMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new ReplaceSelMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ReplaceSelMacroCommand::ReplaceSelMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ReplaceSelMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    //
}

QString ReplaceSelMacroCommand::name() const
{
    //
}

QString ReplaceSelMacroCommand::toText() const
{
    //
}

AbstractMacroCommand *ReplaceSelMacroCommand::clone() const
{
    //
}

/*============================================================================
================================ ReplaceDocMacroCommand ======================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ReplaceDocMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new ReplaceDocMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ReplaceDocMacroCommand::ReplaceDocMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ReplaceDocMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    //
}

QString ReplaceDocMacroCommand::name() const
{
    //
}

QString ReplaceDocMacroCommand::toText() const
{
    //
}

AbstractMacroCommand *ReplaceDocMacroCommand::clone() const
{
    //
}

/*============================================================================
================================ ExecMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1) ? new ExecMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecMacroCommand::ExecMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    //
}

QString ExecMacroCommand::name() const
{
    //
}

QString ExecMacroCommand::toText() const
{
    //
}

AbstractMacroCommand *ExecMacroCommand::clone() const
{
    //
}

/*============================================================================
================================ ExecFMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecFMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new ExecFMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecFMacroCommand::ExecFMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecFMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    //
}

QString ExecFMacroCommand::name() const
{
    //
}

QString ExecFMacroCommand::toText() const
{
    //
}

AbstractMacroCommand *ExecFMacroCommand::clone() const
{
    //
}

/*============================================================================
================================ ExecDMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecDMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1) ? new ExecDMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecDMacroCommand::ExecDMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecDMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    //
}

QString ExecDMacroCommand::name() const
{
    //
}

QString ExecDMacroCommand::toText() const
{
    //
}

AbstractMacroCommand *ExecDMacroCommand::clone() const
{
    //
}

/*============================================================================
================================ ExecFDMacroCommand ==========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecFDMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new ExecFDMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecFDMacroCommand::ExecFDMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecFDMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    //
}

QString ExecFDMacroCommand::name() const
{
    //
}

QString ExecFDMacroCommand::toText() const
{
    //
}

AbstractMacroCommand *ExecFDMacroCommand::clone() const
{
    //
}
