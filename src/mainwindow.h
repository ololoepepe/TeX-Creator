#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class BCodeEditor;

class SymbolsWidget;
class SamplesWidget;
class ConsoleWidget;

class QString;
class QAction;
class QMenu;
class QDockWidget;
class QSignalMapper;
class QCloseEvent;
class QToolBar;
class QByteArray;

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
private slots:
    void retranslateUi();
    void updateWindowTitle(const QString &fileName);
    void checkAutotextMenu(bool documentAvailable);
    void reloadAutotext();
private:
    QSignalMapper *mmprAutotext;
    QSignalMapper *mmprOpenFile;
    //
    BCodeEditor *cedtr;
    SymbolsWidget *swgt;
    SamplesWidget *smpwgt;
    ConsoleWidget *cwgt;
    QDockWidget *dwgtSymbols;
    QDockWidget *dwgtSamples;
    QDockWidget *dwgtConsole;
    //
    QMenu *mnuFile;
      QAction *actQuit;
    QMenu *mnuEdit;
      QMenu *mmnuAutotext;
    QMenu *mmnuDocument;
    QMenu *mnuView;
    QMenu *mmnuTools;
      QAction *mactReloadAutotext;
      //separator
      QAction *mactOpenAutotextUserFolder;
    QMenu *mnuTexsample;
    QMenu *mnuHelp;
    //
    QToolBar *tbarOpen;
    QToolBar *tbarSave;
    QToolBar *tbarUndoRedo;
    QToolBar *tbarClipboard;
    QToolBar *tbarDocument;
    QToolBar *tbarSearch;
private:
    Q_DISABLE_COPY(MainWindow)
};

#endif // MAINWINDOW_H
