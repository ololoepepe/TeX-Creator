#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class BTextEditor;
class BAbstractSettingsTab;

class SymbolsWidget;
class ConsoleWidget;

class QString;
class QAction;
class QMenu;
class QDockWidget;

#include <bmainwindow.h>

class MainWindow : public BMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow();
    //
    BTextEditor *textEditor() const;
protected:
    void retranslateUi();
    bool handleClosing();
    QMap<QString, BAbstractSettingsTab *> userSettingsTabMap() const;
    void handleUserSettings(const QMap<QString, QVariantMap> &settings);
private:
    //mMenuFile
    //mMenuEdit
    //TextEditor menus
    QMenu *mMenuView;
      //createPopupMenu()->actions()
    QMenu *mMenuTools;
      //ConsoleWidget actions
    //
    //TextEditorToolBars
    //
    BTextEditor *mTextEditor;
    SymbolsWidget *mSymbolsWgt;
    ConsoleWidget *mConsoleWidget;
    QDockWidget *mDwgtSymbols;
    QDockWidget *mDockWidgetConsole;
    //
    //StatusBar
    //
    void saveSettings();
    void loadSettings();
    void initTextEditor();
    void initDockWidgets();
    void initSymbolsWidget();
    void initConsoleWidget();
    void initMenuBar();
private slots:
    void updateWindowTitle(const QString &fileName);
};

#endif // MAINWINDOW_H
