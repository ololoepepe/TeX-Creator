#ifndef EXECMACROCOMMANDS_H
#define EXECMACROCOMMANDS_H

class MacroExecutionStack;

class BAbstractCodeEditorDocument;

class QString;

#include "macrocommand.h"
#include "macrocommandargument.h"

#include <QList>

/*============================================================================
================================ ExecMacroCommand ============================
============================================================================*/

class ExecMacroCommand : public AbstractMacroCommand
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

class ExecFMacroCommand : public AbstractMacroCommand
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

class ExecDMacroCommand : public AbstractMacroCommand
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

class ExecFDMacroCommand : public AbstractMacroCommand
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

#endif // EXECMACROCOMMANDS_H
