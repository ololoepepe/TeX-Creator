#ifndef APPLICATION_H
#define APPLICATION_H

class Client;
class MainWindow;

class BCodeEditor;
class BAbstractSettingsTab;
class BSpellChecker;
class BPluginWrapper;

class QWidget;
class QFileSystemWatcher;

#include <BApplication>
#include <BSettingsDialog>
#include <BCodeEdit>

#include <QObject>
#include <QMap>
#include <QFont>
#include <QTextCodec>
#include <QStringList>
#include <QByteArray>
#include <QLocale>
#include <QList>

#if defined(bApp)
#undef bApp
#endif
#define bApp (static_cast<Application *>(BApplication::instance()))

/*============================================================================
================================ Application =================================
============================================================================*/

class Application : public BApplication
{
    Q_OBJECT
public:
    enum Settings
    {
        AccountSettings,
        ConsoleSettings
    };
public:
    explicit Application();
    ~Application();
public:
    static void createInitialWindow(const QStringList &args);
    static QWidget *mostSuitableWindow();
    static QList<BCodeEditor *> codeEditors();
    static bool mergeWindows();
    static void handleExternalRequest(const QStringList &args);
    static bool showLoginDialog(QWidget *parent = 0);
    static bool showRegisterDialog(QWidget *parent = 0);
    static bool showSettings(Settings type, QWidget *parent = 0);
    static void emitUseRemoteCompilerChanged();
    static void updateDocumentType();
    static void updateMaxDocumentSize();
    static void checkForNewVersions(bool persistent = false);
    static BSpellChecker *spellChecker();
    static void resetProxy();
    static void windowAboutToClose(MainWindow *mw);
public slots:
    void checkForNewVersionsSlot();
protected:
    QList<BAbstractSettingsTab *> createSettingsTabs() const;
signals:
    void useRemoteCompilerChanged();
    void reloadAutotexts();
    void reloadKlms();
private:
    static bool testAppInit();
private:
    void addMainWindow(const QStringList &fileNames = QStringList());
    void reloadDictionaries();
private slots:
    void mainWindowDestroyed(QObject *obj);
    void fileHistoryChanged(const QStringList &history);
    void directoryChanged(const QString &path);
    void checkingForNewVersionsFinished();
    void pluginActivatedSlot(BPluginWrapper *pw);
    void pluginAboutToBeDeactivatedSlot(BPluginWrapper *pw);
private:
    bool minitialWindowCreated;
    QMap<QObject *, MainWindow *> mmainWindows;
    BSpellChecker *msc;
    QFileSystemWatcher *watcher;
    QList<QObject *> futureWatchers;
private:
    Q_DISABLE_COPY(Application)
};

#endif // APPLICATION_H
