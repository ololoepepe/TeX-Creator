#ifndef DOCUMENTMACROCOMMANDS_H
#define DOCUMENTMACROCOMMAND_H

class MacroExecutionStack;

class BAbstractCodeEditorDocument;

class QString;

#include "macrocommand.h"
#include "macrocommandargument.h"

#include <QList>
#include <Qt>

/*============================================================================
================================ InsertMacroCommand ==========================
============================================================================*/

class InsertMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
public:
    explicit InsertMacroCommand(const QString &txt);
protected:
    explicit InsertMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
    bool append(const QString &txt);
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
protected:
    explicit PressMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ FindMacroCommand ============================
============================================================================*/

class FindMacroCommand : public AbstractMacroCommand
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

class ReplaceMacroCommand : public AbstractMacroCommand
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

class ReplaceSelMacroCommand : public AbstractMacroCommand
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

class ReplaceDocMacroCommand : public AbstractMacroCommand
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

#endif // DOCUMENTMACROCOMMANDS_H
