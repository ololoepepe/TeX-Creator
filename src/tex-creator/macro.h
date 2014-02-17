#ifndef MACRO_H
#define MACRO_H

class MacroExecutionStack;
class AbstractMacroCommand;

class BAbstractCodeEditorDocument;
class BCodeEditor;

class QString;
class QKeyEvent;

#include <QList>

/*============================================================================
================================ Macro =======================================
============================================================================*/

class Macro
{
public:
    explicit Macro();
    explicit Macro(const QString &fileName);
    Macro(const Macro &other);
    ~Macro();
public:
    void clear();
    void execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, BCodeEditor *edtr,
                 QString *error = 0) const;
    bool recordKeyPress(const QKeyEvent *e, QString *error = 0);
    bool fromText(const QString &text, QString *error = 0);
    bool fromFile(const QString &fileName, QString *error = 0);
    QString toText(QString *error = 0) const;
    bool toFile(const QString &fileName, QString *error = 0) const;
    bool isValid() const;
    bool isEmpty() const;
public:
    Macro &operator=(const Macro &other);
private:
    QList<AbstractMacroCommand *> mcommands;
};

#endif // MACRO_H
