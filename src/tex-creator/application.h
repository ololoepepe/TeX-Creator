/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
**
** This file is part of TeX Creator.
**
** TeX Creator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** TeX Creator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with TeX Creator.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef APPLICATION_H
#define APPLICATION_H

class Client;
class ConsoleWidget;
class MainWindow;

class BCodeEditor;
class BAbstractSettingsTab;
class BSpellChecker;
class BPluginWrapper;
class BNetworkConnection;
class BNetworkOperation;

class QFileSystemWatcher;
class QString;
class QWidget;

#include <TApplication>

#include <QList>
#include <QMap>
#include <QObject>
#include <QStringList>

#if defined(bApp)
#   undef bApp
#endif
#define bApp (static_cast<Application *>(BApplication::instance()))

/*============================================================================
================================ Application =================================
============================================================================*/

class Application : public TApplication
{
    Q_OBJECT
public:
    enum SettingsType
    {
        ConsoleSettings,
        TexsampleSettings
    };
private:
    Client *mclient;
    QFileSystemWatcher *mfsWatcher;
    QList<QObject *> mfutureWatchers;
    QMap<QObject *, MainWindow *> mmainWindows;
    BSpellChecker *mspellChecker;
public:
    explicit Application(int &argc, char **argv, const QString &applicationName, const QString &organizationName);
    ~Application();
public:
    static void resetProxy();
    static void windowAboutToClose(MainWindow *mw);
public:
    Client *client() const;
    QList<BCodeEditor *> codeEditors() const;
    QList<ConsoleWidget *> consoleWidgets() const;
    void handleExternalRequest(const QStringList &args);
    bool mergeWindows();
    MainWindow *mostSuitableWindow() const;
    bool showSettings(SettingsType type, QWidget *parent = 0);
    BSpellChecker *spellChecker() const;
    void updateClientSettings();
    void updateCodeEditorSettings();
    void updateConsoleSettings();
public slots:
    bool checkForNewVersion(bool persistent = false);
    bool checkForNewVersionPersistent();
    bool showConsoleSettings(QWidget *parent = 0);
    bool showRecoverDialog(QWidget *parent = 0);
    bool showRegisterDialog(QWidget *parent = 0);
    bool showTexsampleSettings(QWidget *parent = 0);
protected:
    QList<BAbstractSettingsTab *> createSettingsTabs() const;
signals:
    void reloadAutotexts();
private:
    static void showMessageFunction(const QString &text, const QString &informativeText, bool error,
                                    QWidget *parentWidget);
    static bool testAppInit();
    static bool waitForConnectedFunction(BNetworkConnection *connection, int timeout, bool gui, QWidget *parentWidget,
                                         QString *msg);
    static bool waitForFinishedFunction(BNetworkOperation *op, int timeout, bool gui, QWidget *parentWidget,
                                        QString *msg);
private:
    void addMainWindow(const QStringList &fileNames = QStringList());
    void compatibility();
    void createInitialWindow();
    void reloadDictionaries();
    void showStatusBarMessage(const QString &message);
    void texsample();
private slots:
    void checkingForNewVersionFinished();
    void directoryChanged(const QString &path);
    void fileHistoryChanged(const QStringList &history);
    void mainWindowDestroyed(QObject *obj);
    void pluginAboutToBeDeactivatedSlot(BPluginWrapper *pw);
    void pluginActivatedSlot(BPluginWrapper *pw);
private:
    Q_DISABLE_COPY(Application)
};

#endif // APPLICATION_H
