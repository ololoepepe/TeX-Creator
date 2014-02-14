#include "macroexecutionstack.h"

MacroExecutionStack::MacroExecutionStack(MacroExecutionStack *parent)
{
    mparent = parent;
}
