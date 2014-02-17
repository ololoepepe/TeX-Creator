#ifndef CODEEDITORMODULEPLUGININTERFACE_H
#define CODEEDITORMODULEPLUGININTERFACE_H

class BCodeEditor;

class QMainWindow;

#include <QtPlugin>

/*============================================================================
================================ CodeEditorModulePluginInterface =============
============================================================================*/

class CodeEditorModulePluginInterface
{
public:
    virtual ~CodeEditorModulePluginInterface() {}
public:
    virtual bool installModule(BCodeEditor *cedtr, QMainWindow *mw) = 0;
    virtual bool uninstallModule(BCodeEditor *cedtr, QMainWindow *mw) = 0;
};

Q_DECLARE_INTERFACE(CodeEditorModulePluginInterface, "TeX-Creator.CodeEditorModulePluginInterface")

#endif // CODEEDITORMODULEPLUGININTERFACE_H
