#include "macrocommandargument.h"
#include "macrocommand.h"
#include "global.h"

#include <BeQtGlobal>

#include <QString>
#include <QRegExp>
#include <QSet>

#include <QDebug>

/*============================================================================
================================ MacroCommandArgument ========================
============================================================================*/

/*============================== Static public methods =====================*/

bool MacroCommandArgument::isCommand(QString txt)
{
    typedef QSet<QString> StringSet;
    init_once(StringSet, escSet, StringSet())
    {
        escSet.insert("\\\\");
        escSet.insert("\\n");
        escSet.insert("\\t");
        escSet.insert("\\{");
        escSet.insert("\\}");
        escSet.insert("\\[");
        escSet.insert("\\]");
        escSet.insert("\\$");
        escSet.insert("\\%");
    }
    //txt.remove(QRegExp("%.*"));
    int ind = Global::indexOfHelper(txt, "%");
    if (ind >= 0)
        txt.remove(ind, txt.length() - ind);
    if (txt.isEmpty())
        return false;
    if (!txt.startsWith('\\') || escSet.contains(txt.mid(0, 2)))
        return false;
    return true;
}

bool MacroCommandArgument::isText(QString txt)
{
    return !isCommand(txt);
}

/*============================== Public constructors =======================*/

MacroCommandArgument::MacroCommandArgument()
{
    init();
}

MacroCommandArgument::MacroCommandArgument(const QString &txt)
{
    init();
    fromText(txt);
}

MacroCommandArgument::MacroCommandArgument(const MacroCommandArgument &other)
{
    init();
    *this = other;
}

MacroCommandArgument::~MacroCommandArgument()
{
    delete command;
}

/*============================== Public methods ============================*/

void MacroCommandArgument::clear()
{
    text.clear();
    delete command;
    command = 0;
    err.clear();
}

bool MacroCommandArgument::fromText(const QString &txt)
{
    clear();
    if (isText(txt))
    {
        text = txt;
        return true;
    }
    else if (isCommand(txt))
    {
        command = AbstractMacroCommand::fromText(txt, &err);
        return command;
    }
    else
    {
        err = "Failed to initialize argument";
        return false;
    }
}

QString MacroCommandArgument::toText() const
{
    return (command && command->isValid()) ? command->toText() : text;
}

QString MacroCommandArgument::toText(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                                     QString *error) const
{
    return (command && command->isValid()) ? command->execute(doc, stack, error) : bRet(error, QString(), text);
}

bool MacroCommandArgument::isCommand() const
{
    return command;
}

bool MacroCommandArgument::isText() const
{
    return !text.isEmpty();
}

bool MacroCommandArgument::isValid() const
{
    return !text.isEmpty() || (command && command->isValid());
}

QString MacroCommandArgument::error() const
{
    return err;
}

/*============================== Public operators ==========================*/

MacroCommandArgument &MacroCommandArgument::operator =(const MacroCommandArgument &other)
{
    text = other.text;
    delete command;
    command = other.command ? other.command->clone() : 0;
    err = other.err;
    return *this;
}

bool MacroCommandArgument::operator ==(const MacroCommandArgument &other) const
{
    if ((command && !other.command) || (!command && other.command) || text != other.text || err != other.err)
        return false;
    return !command || command->compare(other.command);
}

bool MacroCommandArgument::operator !=(const MacroCommandArgument &other) const
{
    return !(*this == other);
}

/*============================== Private methods ===========================*/

void MacroCommandArgument::init()
{
    command = 0;
}
