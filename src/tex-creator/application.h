/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
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

class ConsoleWidget;
class MainWindow;
class TexsampleCore;

class BCodeEditor;
class BAbstractSettingsTab;
class BSpellChecker;
class BPluginWrapper;

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
    QFileSystemWatcher *mfsWatcher;

    QMap<QObject *, MainWindow *> mmainWindows;

    BSpellChecker *mspellChecker;
    TexsampleCore *mtexsampleCore;

public:
    explicit Application(int &argc, char **argv, const QString &applicationName, const QString &organizationName);
    ~Application();
public:
    static void resetProxy();
    static void windowAboutToClose(MainWindow *mw);
public:
    QList<BCodeEditor *> codeEditors() const;
    QList<ConsoleWidget *> consoleWidgets() const;
    bool mergeWindows();
    MainWindow *mostSuitableWindow() const;
    bool showSettings(SettingsType type, QWidget *parent = 0);
    BSpellChecker *spellChecker() const;
    TexsampleCore *texsampleCore() const;
    void updateCodeEditorSettings();
    void updateConsoleSettings();
public slots:
    void messageReceived(const QStringList &args);
    bool showConsoleSettings(QWidget *parent = 0);
    void showStatusBarMessage(const QString &message);
protected:
    QList<BAbstractSettingsTab *> createSettingsTabs() const;
signals:
    void reloadAutotexts();
private:
    static bool testAppInit();
private:
    void addMainWindow(const QStringList &fileNames = QStringList());
    void compatibility();
    void createInitialWindow();
    void reloadDictionaries();
private slots:
    void directoryChanged(const QString &path);
    void fileHistoryChanged(const QStringList &history);
    void mainWindowDestroyed(QObject *obj);
    void pluginAboutToBeDeactivatedSlot(BPluginWrapper *pw);
    void pluginActivatedSlot(BPluginWrapper *pw);
private:
    Q_DISABLE_COPY(Application)
};

#endif // APPLICATION_H
