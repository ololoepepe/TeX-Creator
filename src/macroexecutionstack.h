#ifndef MACROEXECUTIONSTACK_H
#define MACROEXECUTIONSTACK_H

/*============================================================================
================================ MacroExecutionStack =========================
============================================================================*/

class MacroExecutionStack
{
public:
    explicit MacroExecutionStack(MacroExecutionStack *parent = 0);
private:
    MacroExecutionStack *mparent;
};

#endif // MACROEXECUTIONSTACK_H
