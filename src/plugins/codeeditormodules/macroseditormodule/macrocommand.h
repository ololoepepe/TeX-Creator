#ifndef MACROCOMMAND_H
#define MACROCOMMAND_H

class MacroExecutionStack;

class BAbstractCodeEditorDocument;

class QString;
class QKeyEvent;

/*============================================================================
================================ AbstractMacroCommand ========================
============================================================================*/

class AbstractMacroCommand
{
public:
    static AbstractMacroCommand *fromText(QString text, QString *error = 0);
    static AbstractMacroCommand *fromKeyPressEvent(const QKeyEvent *e, AbstractMacroCommand *previousCommand = 0);
    static AbstractMacroCommand *fromKeyPressEvent(const QKeyEvent *e, QString *error,
                                                   AbstractMacroCommand *previousCommand = 0);
public:
    explicit AbstractMacroCommand();
    virtual ~AbstractMacroCommand() = 0;
public:
    virtual void clear();
    virtual QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                            QString *error = 0) const = 0;
    virtual QString name() const = 0;
    virtual QString toText() const = 0;
    virtual bool isValid() const = 0;
    virtual AbstractMacroCommand *clone() const = 0;
    bool compare(const AbstractMacroCommand *other) const;
protected:
    virtual bool compareInternal(const AbstractMacroCommand *other) const = 0;
};

#endif // MACROCOMMAND_H
