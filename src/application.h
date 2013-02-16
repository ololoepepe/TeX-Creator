#ifndef APPLICATION_H
#define APPLICATION_H

class Client;
class MainWindow;

class BCodeEditor;
class BAbstractSettingsTab;

class QWidget;

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
#define bApp ( static_cast<Application *>( BApplication::instance() ) )

#define sClient Application::clientInstance()

/*============================================================================
================================ Application =================================
============================================================================*/

class Application : public BApplication
{
    Q_OBJECT
public:
    explicit Application();
    ~Application();
public:
    static void createInitialWindow(const QStringList &args);
    static Client *clientInstance();
    static QWidget *mostSuitableWindow();
    static QList<BCodeEditor *> codeEditors();
    static bool mergeWindows();
    static void handleExternalRequest(const QStringList &args);
    static bool showPasswordDialog(QWidget *parent = 0);
    static bool showRegisterDialog(QWidget *parent = 0);
protected:
    QList<BAbstractSettingsTab *> createSettingsTabs() const;
private:
    static bool testAppInit();
private:
    void addMainWindow( const QStringList &fileNames = QStringList() );
private slots:
    void mainWindowDestroyed(QObject *obj);
    void fileHistoryChanged(const QStringList &history);
private:
    bool minitialWindowCreated;
    Client *mclient;
    QMap<QObject *, MainWindow *> mmainWindows;
private:
    Q_DISABLE_COPY(Application)
};

#endif // APPLICATION_H
