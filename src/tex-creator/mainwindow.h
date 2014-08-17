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
public slots:
    void showStatusBarMessage(const QString &message);
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
    QMenu *mmnuTools;
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
private:
    Q_DISABLE_COPY(MainWindow)
};

#endif // MAINWINDOW_H
