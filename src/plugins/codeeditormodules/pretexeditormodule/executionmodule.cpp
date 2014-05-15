#include "executionmodule.h"

#include <QStringList>
#include <QString>

ExecutionModule::ExecutionModule()
{
    //
}

QStringList ExecutionModule::specFuncNames()
{
    static const QStringList names = QStringList() << "set" << "renewFunc"
    << "newVar" << "newLocalVar" << "newGlobalVar" << "tryNewVar" << "tryNewLocalVar" << "tryNewGlobalVar"
    << "newFunc" << "newLocalFunc" << "newGlobalFunc" << "tryNewFunc" << "tryNewLocalFunc" << "tryNewGlobalFunc"
    << "newArray" << "newLocalArray" << "newGlobalArray" << "tryNewArray" << "tryNewLocalArray" << "tryNewGlobalArray";
    return names;
}

QStringList ExecutionModule::funcNames()
{
    static const QStringList names = QStringList() << "+" << "-" << "*";
    return names;
}
