#include "macrocommand.h"
#include "macrocommandargument.h"
#include "macroexecutionstack.h"
#include "macroseditormoduleplugin.h"
#include "unmacrocommand.h"
#include "global.h"
#include "binmacrocommands.h"
#include "execmacrocommands.h"
#include "documentmacrocommands.h"
#include "generalmacrocommands.h"
#include "varfuncmacrocommands.h"
#include "loopmacrocommand.h"

#include <BAbstractCodeEditorDocument>

#include <QString>
#include <QEvent>
#include <QKeyEvent>
#include <QList>
#include <QMap>

#include <QDebug>

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
================================ Global static functions =====================
============================================================================*/

static QStringList getArgs(const QString &text, int &i, char opbr, char clbr, int max, char nopbr = '\0',
                           QString *error = 0)
{
    if (i >= text.length())
        return bRet(error, QString(), QStringList());
    if (text.isEmpty() || i < 0)
        return bRet(error, QString("Argument parcing error"), QStringList());
    QStringList args;
    int depth = 1;
    QString s;
    while (i < text.length() && (text.at(i) != nopbr ||  text.at(i - 1) == '\\' || depth))
    {
        if (text.at(i) == clbr && text.at(i - 1) != '\\')
        //if (text.at(i) == clbr && (text.at(i - 1) != '\\' || text.at(i - 2) == '\\'))
        {
            --depth;
            if (depth)
                s += clbr;
        }
        else if (text.at(i) == opbr && text.at(i - 1) != '\\')
        //else if (text.at(i) == opbr && (text.at(i - 1) != '\\' || text.at(i - 2) == '\\'))
        {
            if (depth)
                s += opbr;
            ++depth;
        }
        else
        {   if (!depth)
                return bRet(error, QString("Argument parcing error"), QStringList());
            s += text.at(i);
        }
        if (!depth)
        {
            args << s;
            if (max >= 0 && args.size() > max)
                return bRet(error, QString("Too many arguments"), QStringList());
            s.clear();
        }
        ++i;
    }
    if (!s.isEmpty() || depth)
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
        infoMap.insert("format", FunctionInfo(&FormatMacroCommand::create, 2));
        infoMap.insert("def", FunctionInfo(&DefMacroCommand::create, 2, 1));
        infoMap.insert("defF", FunctionInfo(&DefFMacroCommand::create, 2, 1));
        infoMap.insert("undef", FunctionInfo(&UndefMacroCommand::create, 1));
        infoMap.insert("defined", FunctionInfo(&DefinedMacroCommand::create, 1));
        infoMap.insert("set", FunctionInfo(&SetMacroCommand::create, 2, 1));
        infoMap.insert("setF", FunctionInfo(&SetFMacroCommand::create, 2));
        infoMap.insert("get", FunctionInfo(&GetMacroCommand::create, 1, 1));
        infoMap.insert("call", FunctionInfo(&CallMacroCommand::create, 1, -1));
        infoMap.insert("var", FunctionInfo(&VarMacroCommand::create, 1, 1));
        infoMap.insert("bin", FunctionInfo(&BinMacroCommand::create, 3, 1));
        infoMap.insert("binM", FunctionInfo(&BinMMacroCommand::create, 3, -1));
        infoMap.insert("un", FunctionInfo(&UnMacroCommand::create, 2, 1));
        infoMap.insert("c", FunctionInfo(&CMacroCommand::create, 1, 1));
        infoMap.insert("multi", FunctionInfo(&MultiMacroCommand::create, 1, -1));
        infoMap.insert("for", FunctionInfo(&ForMacroCommand::create, 5, -1));
        infoMap.insert("while", FunctionInfo(&WhileMacroCommand::create, 2, -1));
        infoMap.insert("doWhile", FunctionInfo(&DoWhileMacroCommand::create, 2, -1));
        infoMap.insert("until", FunctionInfo(&UntilMacroCommand::create, 2, -1));
        infoMap.insert("doUntil", FunctionInfo(&DoUntilMacroCommand::create, 2, -1));
        infoMap.insert("if", FunctionInfo(&IfMacroCommand::create, 2, 1));
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
    int ind = Global::indexOfHelper(text, "%");
    if (ind >= 0)
        text.remove(ind, text.length() - ind);
    //text.remove("\\e");
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
    if (i + 1 == text.length() && (text.at(i) == '[' || text.at(i) == '{'))
        return bRet(error, QString("Argument parcing error"), (AbstractMacroCommand *) 0);
    textArgs += getArgs(text, ++i, '[', ']', info.optArgCount, '\0', &err);
    if (!err.isEmpty())
        return bRet(error, err, (AbstractMacroCommand *) 0);
    if (i < text.length())
        return bRet(error, QString("Argument parcing error"), (AbstractMacroCommand *) 0);
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
    if (key <= 0)
        return bRet(error, QString("Invalid key press"), (AbstractMacroCommand *) 0);
    if (Qt::Key_Control == key || Qt::Key_Alt == key || Qt::Key_Shift == key)
        return bRet(error, QString("The event is a modifier key press"), (AbstractMacroCommand *) 0);
    if ((modifiers & Qt::ControlModifier) && (modifiers & Qt::ShiftModifier) && (modifiers & Qt::AltModifier)
            && (modifiers & Qt::MetaModifier))
        return bRet(error, QString("The event is a locale change key press"), (AbstractMacroCommand *) 0);
    if (!(modifiers & Qt::ControlModifier) && !(modifiers & Qt::AltModifier) && Qt::Key_Return == key)
        text = "\n";
    if (text.isEmpty() || (!text.at(0).isPrint() && text.at(0) != '\n')
            || (modifiers & Qt::ControlModifier) || (modifiers & Qt::AltModifier))
        return bRet(error, QString(), new PressMacroCommand(key, modifiers));
    if (!previousCommand || previousCommand->name() != "insert")
        return bRet(error, QString(), new InsertMacroCommand(Global::toVisibleText(text)));
    if (!dynamic_cast<InsertMacroCommand *>(previousCommand)->append(Global::toVisibleText(text)))
        return bRet(error, QString(), new InsertMacroCommand(Global::toVisibleText(text)));
    return bRet(error, QString(), previousCommand);
}

/*============================== Public constructors =======================*/

AbstractMacroCommand::~AbstractMacroCommand()
{
    //
}

/*============================== Protected constructors ====================*/

AbstractMacroCommand::AbstractMacroCommand(const QList<MacroCommandArgument> &args)
{
    margs = args;
}

/*============================== Public methods ============================*/

void AbstractMacroCommand::clear()
{
    margs.clear();
}

bool AbstractMacroCommand::isValid() const
{
    if (margs.isEmpty())
        return false;
    foreach (const MacroCommandArgument &a, margs)
        if (!a.isValid())
            return false;
    return true;
}

bool AbstractMacroCommand::compare(const AbstractMacroCommand *other) const
{
    if (!other || other->name() != name())
        return false;
    return other && other->name() == name() && compareInternal(other);
}

/*============================== Protected methods =========================*/

bool AbstractMacroCommand::compareInternal(const AbstractMacroCommand *other) const
{
    return other && margs == other->margs;
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
