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
class MainWindow;

class BCodeEditor;
class BAbstractSettingsTab;
class BSpellChecker;
class BPluginWrapper;

class QWidget;
class QFileSystemWatcher;

#include <TApplication>
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
    enum Settings
    {
        AccountSettings,
        ConsoleSettings
    };
private:
    QFileSystemWatcher *mfsWatcher;
    QList<QObject *> mfutureWatchers;
    QMap<QObject *, MainWindow *> mmainWindows;
    BSpellChecker *mspellChecker;
public:
    explicit Application(int &argc, char **argv, const QString &applicationName, const QString &organizationName);
    ~Application();
public:
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
private:
    static bool testAppInit();
private:
    void addMainWindow(const QStringList &fileNames = QStringList());
    void compatibility();
    void createInitialWindow();
    void reloadDictionaries();
private slots:
    void checkingForNewVersionsFinished();
    void directoryChanged(const QString &path);
    void fileHistoryChanged(const QStringList &history);
    void mainWindowDestroyed(QObject *obj);
    void pluginAboutToBeDeactivatedSlot(BPluginWrapper *pw);
    void pluginActivatedSlot(BPluginWrapper *pw);
private:
    Q_DISABLE_COPY(Application)
};

#endif // APPLICATION_H
