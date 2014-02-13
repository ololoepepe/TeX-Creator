#include "macrocommand.h"
#include "macrocommandargument.h"
#include "global.h"

#include <BAbstractCodeEditorDocument>

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

/*============================================================================
================================ InsertMacroCommand ==========================
============================================================================*/

class InsertMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
public:
    explicit InsertMacroCommand(const QString &txt);
    explicit InsertMacroCommand(const MacroCommandArgument &a);
    ~InsertMacroCommand();
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
    explicit PressMacroCommand(const MacroCommandArgument &a);
    ~PressMacroCommand();
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
            if (args.size() > max)
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
    typedef AbstractMacroCommand *(*CreateMacroCommandFunction)(const QList<MacroCommandArgument> &);
    static QMap<QString, int> mndArgCountMap;
    static QMap<QString, int> optArgCountMap;
    static QMap<QString, CreateMacroCommandFunction> funcMap;
    static bool init = false;
    if (!init && (init = true))
    {
        mndArgCountMap.insert("insert", 1);
        optArgCountMap.insert("insert", 0);
        funcMap.insert("insert", &InsertMacroCommand::create);
        mndArgCountMap.insert("press", 1);
        optArgCountMap.insert("press", 0);
        funcMap.insert("press", &PressMacroCommand::create);
    }
    text.remove(QRegExp("%.*"));
    if (text.isEmpty())
        return bRet(error, QString("Empty string"), (AbstractMacroCommand *) 0);
    if (!text.startsWith('\\'))
        return bRet(error, QString("A command must start with \"\\\""), (AbstractMacroCommand *) 0);
    QString commandName;
    int i = Global::indexOfHelper(text, "{", 2);
    commandName += text.mid(1, (i > 0) ? (i - 1) : -1);
    CreateMacroCommandFunction func = funcMap.value(commandName);
    if (!func)
        return bRet(error, QString("Invalid function name"), (AbstractMacroCommand *) 0);
    QString err;
    QStringList textArgs = getArgs(text, ++i, '{', '}', mndArgCountMap.value(commandName), '[', &err);
    if (!err.isEmpty())
        return bRet(error, err, (AbstractMacroCommand *) 0);
    textArgs += getArgs(text, ++i, '[', ']', optArgCountMap.value(commandName), '\0', &err);
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
    return bRet(error, QString(), func(args));
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

InsertMacroCommand::InsertMacroCommand(const MacroCommandArgument &a)
{
    arg = a;
}

InsertMacroCommand::~InsertMacroCommand()
{
    //
}

/*============================== Public methods ============================*/

void InsertMacroCommand::clear()
{
    arg.clear();
}

QString InsertMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    if (!doc || !arg.isValid())
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

PressMacroCommand::PressMacroCommand(const MacroCommandArgument &a)
{
    arg = a;
}

PressMacroCommand::~PressMacroCommand()
{
    //
}

/*============================== Public methods ============================*/

void PressMacroCommand::clear()
{
    arg.clear();
}

QString PressMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack) const
{
    if (!doc || !arg.isValid())
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
