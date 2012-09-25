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
class QSignalMapper;

#include <bmainwindow.h>

class MainWindow : public BMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow();
    //
    BTextEditor *textEditor() const;
protected:
    bool handleClosing();
    QMap<QString, BAbstractSettingsTab *> userSettingsTabMap() const;
    void handleUserSettings(const QMap<QString, QVariantMap> &settings);
private:
    enum Location
    {
        AutoTextSharedLocation,
        KLMSharedLocation,
        PluginsSharedLocation,
        TranslationsSharedLocation,
        AutoTextUserLocation,
        KLMUserLocation,
        MacrosUserLocations,
        PluginsUserLocation,
        TranslationsUserLocation
    };
    //
    QSignalMapper *mmapperLocations;
    //
    //mMenuFile
    //mMenuEdit
    //TextEditor menus
    QMenu *mmnuView;
      //createPopupMenu()->actions()
    QMenu *mMenuTools;
      //ConsoleWidget actions
      //separator
      QMenu *mmnuOpenDirShared;
        QAction *mactAutoTextShared;
        QAction *mactKLMShared;
        QAction *mactPluginsShared;
        QAction *mactTranslationsShared;
      QMenu *mmnuOpenDirUser;
        QAction *mactAutoTextUser;
        QAction *mactKLMUser;
        QAction *mactMacrosUser;
        QAction *mactPluginsUser;
        QAction *mactTranslationsUser;
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
    void retranslateUi();
    void updateWindowTitle(const QString &fileName);
    void fillMnuView();
    void openLocation(int id);
};

#endif // MAINWINDOW_H
