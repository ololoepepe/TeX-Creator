#ifndef BINMACROCOMMANDS_H
#define BINMACROCOMMANDS_H

class MacroExecutionStack;

class BAbstractCodeEditorDocument;

class QString;

#include "macrocommand.h"
#include "macrocommandargument.h"

#include <QList>

/*============================================================================
================================ BinMacroCommand =============================
============================================================================*/

class BinMacroCommand : public AbstractMacroCommand
{
public:
    typedef QString (*BinaryFunction)(QString &, const QString &);
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
    static BinaryFunction binaryFunction(const QString &op);
    static bool hasBinaryFunction(const QString &op);
private:
    explicit BinMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ BinMMacroCommand ============================
============================================================================*/

class BinMMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit BinMMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

#endif // BINMACROCOMMANDS_H
