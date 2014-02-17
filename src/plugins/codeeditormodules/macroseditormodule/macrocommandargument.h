#ifndef MACROCOMMANDARGUMENT_H
#define MACROCOMMANDARGUMENT_H

class AbstractMacroCommand;
class MacroExecutionStack;

class BAbstractCodeEditorDocument;

#include <QString>

/*============================================================================
================================ MacroCommandArgument ========================
============================================================================*/

class MacroCommandArgument
{
public:
    static bool isCommand(QString txt);
    static bool isText(QString txt);
public:
    explicit MacroCommandArgument();
    explicit MacroCommandArgument(const QString &txt);
    MacroCommandArgument(const MacroCommandArgument &other);
    ~MacroCommandArgument();
public:
    void clear();
    bool fromText(const QString &txt);
    QString toText() const;
    QString toText(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    bool isCommand() const;
    bool isText() const;
    bool isValid() const;
    QString error() const;
public:
    MacroCommandArgument &operator =(const MacroCommandArgument &other);
    bool operator ==(const MacroCommandArgument &other) const;
    bool operator !=(const MacroCommandArgument &other) const;
private:
    void init();
private:
    QString text;
    AbstractMacroCommand *command;
    QString err;
};

#endif // MACROCOMMANDARGUMENT_H
