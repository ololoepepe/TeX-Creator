#ifndef EXECUTIONMODULE_H
#define EXECUTIONMODULE_H

#include <QStringList>

class ExecutionModule
{
public:
    explicit ExecutionModule();
public:
    static QStringList specFuncNames();
    static QStringList funcNames();
};

#endif // EXECUTIONMODULE_H
