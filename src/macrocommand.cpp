#include "macrocommand.h"
#include "macrocommandargument.h"
#include "global.h"
#include "macroexecutionstack.h"
#include "application.h"

#include <BAbstractCodeEditorDocument>
#include <BeQt>
#include <BTextTools>

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
#include <QTextDocument>
#include <QFileInfo>
#include <QProcess>
#include <QByteArray>
#include <QTextCodec>
#include <QTextStream>

#include <QDebug>

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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
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
    if (text.isEmpty() || i < 0)
        return bRet(error, QString("Argument parcing error"), QStringList());
    QStringList args;
    int depth = 1;
    QString s;
    while (i < text.length() && (text.at(i) != nopbr || depth))
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

static QString getReplaceMndParameters(const QList<MacroCommandArgument> &args, BAbstractCodeEditorDocument *doc,
                                       MacroExecutionStack *stack, QString &first, QString *second = 0)
{
    QString err;
    QString s1 = args.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return err;
    if (second)
    {
        QString s2 = args.at(1).toText(doc, stack, &err);
        if (!err.isEmpty())
            return err;
        *second = s2;
    }
    first = s1;
    return "";
}

static QString getReplaceOptParameters(const QList<MacroCommandArgument> &args, BAbstractCodeEditorDocument *doc,
                                       MacroExecutionStack *stack, int from, Qt::CaseSensitivity &cs,
                                       bool *w = 0, bool *bw = 0, bool *nc = 0)
{
    Qt::CaseSensitivity csx = Qt::CaseInsensitive;
    bool wx = false;
    bool bwx = false;
    bool ncx = false;
    bool csb = false;
    bool wb = false;
    bool bwb = false;
    bool ncb = false;
    for (int i = from; i < args.size(); ++i)
    {
        QString err;
        QString s = args.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return err;
        if (!s.compare("cs", Qt::CaseInsensitive) || !s.compare("case-sensitive", Qt::CaseInsensitive))
        {
            if (csb)
                return "Duplicate parameter";
            csx = Qt::CaseSensitive;
            csb = true;
        }
        else if (!s.compare("ci", Qt::CaseInsensitive) || !s.compare("case-insensitive", Qt::CaseInsensitive))
        {
            if (csb)
                return "Duplicate parameter";
            csx = Qt::CaseInsensitive;
            csb = true;
        }
        else if (!s.compare("w", Qt::CaseInsensitive) || !s.compare("words", Qt::CaseInsensitive)
                || !s.compare("whole-words", Qt::CaseInsensitive))
        {
            if (!w)
                return "Unknown parameter";
            if (wb)
                return "Duplicate parameter";
            wx = true;
            wb = true;
        }
        else if (!s.compare("bw", Qt::CaseInsensitive) || !s.compare("backward", Qt::CaseInsensitive)
                || !s.compare("backward-order", Qt::CaseInsensitive))
        {
            if (!bw)
                return "Unknown parameter";
            if (bwb)
                return "Duplicate parameter";
            bwx = true;
            bwb = true;
        }
        else if (!s.compare("nc", Qt::CaseInsensitive) || !s.compare("non-cyclic", Qt::CaseInsensitive))
        {
            if (!nc)
                return "Unknown parameter";
            if (ncb)
                return "Duplicate parameter";
            ncx = true;
            ncb = true;
        }
        else
        {
            return "Unknown parameter";
        }
    }
    cs = csx;
    bSet(w, wx);
    bSet(bw, bwx);
    bSet(nc, ncx);
    return "";
}

static QTextDocument::FindFlags createFindFlags(Qt::CaseSensitivity cs, bool w, bool bw)
{
    QTextDocument::FindFlags flags = 0;
    if (Qt::CaseSensitive == cs)
        flags |= QTextDocument::FindCaseSensitively;
    if (w)
        flags |= QTextDocument::FindWholeWords;
    if (bw)
        flags |= QTextDocument::FindBackward;
    return flags;
}

static QString getCommand(const MacroCommandArgument &arg, BAbstractCodeEditorDocument *doc,
                          MacroExecutionStack *stack, QString &command)
{
    QString err;
    QString cmd = arg.toText(doc, stack, &err);
    if (!err.isEmpty())
        return err;
    QString cmd2 = Global::externalTools().value(cmd);
    if (!cmd2.isEmpty())
        cmd = cmd2;
    if (cmd.isEmpty())
        return "Invalid parameter";
    command = cmd;
    return "";
}

static QString getWorkingDir(BAbstractCodeEditorDocument *doc, QString &dir)
{
    QFileInfo fi(doc->fileName());
    if (!fi.exists() || !fi.isFile())
        return "Document does not exist on disk";
    dir = fi.path();
    return "";
}

static QString getFileName(const MacroCommandArgument &arg, BAbstractCodeEditorDocument *doc,
                           MacroExecutionStack *stack, QString &fileName, QString *workingDir = 0)
{
    QString path;
    QString err = getWorkingDir(doc, path);
    if (!err.isEmpty())
        return err;
    QString fn = path + "/" + arg.toText(doc, stack, &err);
    if (!err.isEmpty())
        return err;
    if (fn.isEmpty())
        return "Invalid parameter";
    QFileInfo fi(fn);
    if (!fi.exists() || !fi.isFile())
        return "File does not exist";
    fileName = fn;
    bSet(workingDir, path);
    return "";
}

static QString getCommandArgs(const QList<MacroCommandArgument> &args, int from, BAbstractCodeEditorDocument *doc,
                              MacroExecutionStack *stack, QStringList &arguments)
{
    QStringList sl;
    for (int i = from; i < args.size(); ++i)
    {
        QString err;
        QString s = args.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return err;
        sl << s;
    }
    arguments = sl;
    return "";
}

static QString processOutputToText(const QByteArray &output)
{
    QTextCodec *codec = BTextTools::guessTextCodec(output, Application::locale());
    if (!codec)
        return QString(output).replace('\n', "\\n").replace('\t', "\\t").remove('\r');
    QTextStream in(output);
    in.setCodec(codec);
    return in.readAll().replace('\n', "\\n").replace('\t', "\\t").remove('\r');
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

QString InsertMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !arg.isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString text = arg.toText(doc, stack, &err).replace("\\n", "\n").replace("\\t", "\t");
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    doc->insertText(text);
    return bRet(error, QString(), QString("true"));
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

QString PressMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !arg.isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QPlainTextEdit *ptedt = doc->findChild<QPlainTextEdit *>();
    if (!ptedt)
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QKeySequence ks(arg.toText(doc, stack, &err));
    if (ks.isEmpty() || !err.isEmpty())
        return bRet(error, !err.isEmpty() ? err : QString("Invalid parameter"), QString("false"));
    for (int i = 0; i < (int) ks.count(); ++i)
    {
        int key = ~Qt::KeyboardModifierMask & ks[i];
        Qt::KeyboardModifiers modifiers = static_cast<Qt::KeyboardModifiers>(Qt::KeyboardModifierMask & ks[i]);
        QCoreApplication::postEvent(ptedt, new QKeyEvent(QEvent::KeyPress, key, modifiers));
    }
    return bRet(error, QString(), QString("true"));
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

QString WaitMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString ns = margs.first().toText(doc, stack, &err);
    if (ns.isEmpty() || !err.isEmpty())
        return bRet(error, !err.isEmpty() ? err : QString("Invalid parameter"), QString("false"));
    bool ok = false;
    int n = ns.toInt(&ok);
    if (n < 0 || !ok)
        return bRet(error, QString("Invalid parameter"), QString("false"));
    int k = BeQt::Second;
    bool b = false;
    bool kb = false;
    bool bb = false;
    for (int i = 1; i < margs.size(); ++i)
    {
        QString as = margs.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
        if (!as.compare("ms", Qt::CaseInsensitive) || !as.compare("milliseconds", Qt::CaseInsensitive))
        {
            if (kb)
                return bRet(error, QString("Duplicate parameter"), QString("false"));
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
        else
        {
            return bRet(error, QString("Unknown parameter"), QString("false"));
        }
    }
    if (b)
        ThreadHack::waitBlocking(k * n);
    else
        BeQt::waitNonBlocking(k * n);
    return bRet(error, QString(), QString("true"));
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

QString FindMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    bool w = false;
    bool bw = false;
    bool nc = false;
    QString what;
    QString err = getReplaceMndParameters(margs, doc, stack, what);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    err = getReplaceOptParameters(margs, doc, stack, 1, cs, &w, &bw, &nc);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    bool b = doc->findNext(what, createFindFlags(cs, w, bw), !nc);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString FindMacroCommand::name() const
{
    return "find";
}

QString FindMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\find{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *FindMacroCommand::clone() const
{
    return new FindMacroCommand(margs);
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

QString ReplaceMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                                     QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    bool w = false;
    bool bw = false;
    bool nc = false;
    QString what;
    QString ntext;
    QString err = getReplaceMndParameters(margs, doc, stack, what, &ntext);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    err = getReplaceOptParameters(margs, doc, stack, 2, cs, &w, &bw, &nc);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    bool b = doc->findNext(what, createFindFlags(cs, w, bw), !nc);
    if (!b)
        bRet(error, QString(), QString("false"));
    b = doc->replaceNext(ntext);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString ReplaceMacroCommand::name() const
{
    return "replace";
}

QString ReplaceMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\replace{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ReplaceMacroCommand::clone() const
{
    return new ReplaceMacroCommand(margs);
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

QString ReplaceSelMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                                        QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("-1"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    QString what;
    QString ntext;
    QString err = getReplaceMndParameters(margs, doc, stack, what, &ntext);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    err = getReplaceOptParameters(margs, doc, stack, 2, cs);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    int n = doc->replaceInSelection(what, ntext, cs);
    return bRet(error, QString(), QString::number(n));
}

QString ReplaceSelMacroCommand::name() const
{
    return "replaceSel";
}

QString ReplaceSelMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\replaceSel{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ReplaceSelMacroCommand::clone() const
{
    return new ReplaceSelMacroCommand(margs);
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

QString ReplaceDocMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                                        QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("-1"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    QString what;
    QString ntext;
    QString err = getReplaceMndParameters(margs, doc, stack, what, &ntext);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    err = getReplaceOptParameters(margs, doc, stack, 2, cs);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    int n = doc->replaceInDocument(what, ntext, cs);
    return bRet(error, QString(), QString::number(n));
}

QString ReplaceDocMacroCommand::name() const
{
    return "replaceDoc";
}

QString ReplaceDocMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\replaceDoc{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ReplaceDocMacroCommand::clone() const
{
    return new ReplaceDocMacroCommand(margs);
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

QString ExecMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QStringList args;
    err = getCommandArgs(margs, 1, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString dir;
    getWorkingDir(doc, dir);
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    if (!dir.isEmpty())
        proc.setWorkingDirectory(dir);
    BeQt::startProcess(&proc, cmd, args);
    BeQt::waitNonBlocking(&proc, SIGNAL(started()), 30 * BeQt::Second);
    if (proc.state() != QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process start timeout"), QString());
    }
    BeQt::waitNonBlocking(&proc, SIGNAL(finished(int)), 5 * BeQt::Minute);
    if (proc.state() == QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process execution timeout"), QString());
    }
    return bRet(error, QString(), processOutputToText(proc.readAll()));
}

QString ExecMacroCommand::name() const
{
    return "exec";
}

QString ExecMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\exec{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecMacroCommand::clone() const
{
    return new ExecMacroCommand(margs);
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

QString ExecFMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString fn;
    QString dir;
    err = getFileName(margs.at(1), doc, stack, fn, &dir);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QStringList args;
    err = getCommandArgs(margs, 2, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    args.prepend(fn);
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.setWorkingDirectory(dir);
    BeQt::startProcess(&proc, cmd, args);
    BeQt::waitNonBlocking(&proc, SIGNAL(started()), 30 * BeQt::Second);
    if (proc.state() != QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process start timeout"), QString());
    }
    BeQt::waitNonBlocking(&proc, SIGNAL(finished(int)), 5 * BeQt::Minute);
    if (proc.state() == QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process execution timeout"), QString());
    }
    return bRet(error, QString(), processOutputToText(proc.readAll()));
}

QString ExecFMacroCommand::name() const
{
    return "execF";
}

QString ExecFMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\execF{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecFMacroCommand::clone() const
{
    return new ExecFMacroCommand(margs);
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

QString ExecDMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QStringList args;
    err = getCommandArgs(margs, 1, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString dir;
    getWorkingDir(doc, dir);
    bool b = !dir.isEmpty() ? QProcess::startDetached(cmd, args, dir) : QProcess::startDetached(cmd, args);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString ExecDMacroCommand::name() const
{
    return "execD";
}

QString ExecDMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\execD{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecDMacroCommand::clone() const
{
    return new ExecDMacroCommand(margs);
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

QString ExecFDMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString fn;
    QString dir;
    err = getFileName(margs.at(1), doc, stack, fn, &dir);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QStringList args;
    err = getCommandArgs(margs, 2, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    args.prepend(fn);
    bool b = !dir.isEmpty() ? QProcess::startDetached(cmd, args, dir) : QProcess::startDetached(cmd, args);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString ExecFDMacroCommand::name() const
{
    return "execFD";
}

QString ExecFDMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\execFD{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecFDMacroCommand::clone() const
{
    return new ExecFDMacroCommand(margs);
}
