#ifndef MACROSEDITORMODULEPLUGIN_H
#define MACROSEDITORMODULEPLUGIN_H

class MacrosEditorModule;

class BAbstractSettingsTab;
class BCodeEditor;

class QString;
class QPixmap;
class QMainWindow;
class QByteArray;
class QMenu;
class QDockWidget;

#include <CodeEditorModulePluginInterface>

#include <BPluginInterface>
#include <BGuiPluginInterface>

#include <QObject>
#include <QtPlugin>
#include <QVariantMap>
#include <QMap>

/*============================================================================
================================ MacrosSettingsTab ===========================
============================================================================*/

class MacrosEditorModulePlugin : public QObject, public CodeEditorModulePluginInterface, public BPluginInterface,
        public BGuiPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(BPluginInterface)
    Q_INTERFACES(BGuiPluginInterface)
    Q_INTERFACES(CodeEditorModulePluginInterface)
public:
    static void setMacrosModuleState(const QByteArray &state);
    static void setSaveMacroStack(bool b);
    static void setExternalTools(const QMap<QString, QString> &map);
    static QByteArray macrosModuleState();
    static bool saveMacroStack();
    static QMap<QString, QString> externalTools();
public:
    explicit MacrosEditorModulePlugin();
    ~MacrosEditorModulePlugin();
public:
    QString type() const;
    QString name() const;
    bool prefereStaticInfo() const;
    PluginInfoStatic staticInfo() const;
    PluginInfo info() const;
    void activate();
    void deactivate();
    QPixmap pixmap() const;
    BAbstractSettingsTab *settingsTab() const;
    void handleSettings(const QVariantMap &s);
    bool installModule(BCodeEditor *cedtr, QMainWindow *mw);
    bool uninstallModule(BCodeEditor *cedtr, QMainWindow *mw);
private slots:
    void retranslateUi();
private:
    struct ModuleComponents
    {
        MacrosEditorModule *module;
        BCodeEditor *editor;
        QMainWindow *window;
        QMenu *menu;
        QDockWidget *dock;
    public:
        explicit ModuleComponents();
        explicit ModuleComponents(BCodeEditor *cedtr, QMainWindow *mw);
    public:
        void retranslate();
        void uninstall();
        bool isValid() const;
    };
private:
    QMap<BCodeEditor *, ModuleComponents> mmap;
};

#endif // MACROSEDITORMODULEPLUGIN_H