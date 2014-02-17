#ifndef GENERALMACROCOMMANDS_H
#define GENERALMACROCOMMANDS_H

class MacroExecutionStack;

class BAbstractCodeEditorDocument;

class QString;

#include "macrocommand.h"
#include "macrocommandargument.h"

#include <QList>

/*============================================================================
================================ MultiMacroCommand ===========================
============================================================================*/

class MultiMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit MultiMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ForMacroCommand =============================
============================================================================*/

class ForMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ForMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ IfMacroCommand ==============================
============================================================================*/

class IfMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit IfMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ WaitMacroCommand ============================
============================================================================*/

class WaitMacroCommand : public AbstractMacroCommand
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

#endif // GENERALMACROCOMMANDS_H
