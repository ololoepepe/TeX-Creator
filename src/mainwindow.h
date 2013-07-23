#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class BCodeEditor;

class SymbolsWidget;
class TexsampleWidget;
class ConsoleWidget;

class QString;
class QAction;
class QMenu;
class QSignalMapper;
class QCloseEvent;
class QToolBar;
class QByteArray;
class QLabel;

#include <BCodeEdit>
#include <BApplication>

#include <QMainWindow>
#include <QTextCodec>

/*============================================================================
================================ MainWindow ==================================
============================================================================*/

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow();
    ~MainWindow();
public:
    static QByteArray getWindowGeometry();
    static QByteArray getWindowState();
    static void setWindowGeometry(const QByteArray &geometry);
    static void setWindowState(const QByteArray &state);
public:
    BCodeEditor *codeEditor() const;
    ConsoleWidget *consoleWidget() const;
protected:
    void closeEvent(QCloseEvent *e);
private:
    void initCodeEditor();
    void initDockWidgets();
    void initMenus();
    void retranslateActSpellCheck();
private slots:
    void retranslateUi();
    void updateWindowTitle(const QString &fileName);
    void reloadAutotext();
    void switchSpellCheck();
private:
    QSignalMapper *mmprAutotext;
    QSignalMapper *mmprOpenFile;
    //
    BCodeEditor *mcedtr;
    SymbolsWidget *msymbolsWgt;
    TexsampleWidget *mtexsampleWgt;
    ConsoleWidget *mconsoleWgt;
    //
    QMenu *mmnuFile;
      QAction *mactQuit;
    QMenu *mmnuEdit;
      QMenu *mmnuAutotext;
    QMenu *mmnuDocument;
      QAction *mactSpellCheck;
    QMenu *mmnuView;
    QMenu *mmnuConsole;
    QMenu *mmnuMacros;
    QMenu *mmnuTools;
      QAction *mactReloadAutotext;
      //separator
      QAction *mactOpenAutotextUserFolder;
    QMenu *mmnuTexsample;
    QMenu *mmnuHelp;
    //
    QToolBar *mtbarOpen;
    QToolBar *mtbarSave;
    QToolBar *mtbarUndoRedo;
    QToolBar *mtbarClipboard;
    QToolBar *mtbarDocument;
    QToolBar *mtbarSearch;
    QToolBar *mtbarMacros;
private:
    Q_DISABLE_COPY(MainWindow)
};

#endif // MAINWINDOW_H
