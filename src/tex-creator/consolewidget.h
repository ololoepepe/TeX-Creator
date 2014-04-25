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

#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

class BTerminalWidget;
class BCodeEditor;
class BAbstractCodeEditorDocument;

class QToolBar;
class QAction;
class QEvent;
class QSignalMapper;

#include <QWidget>
#include <QString>
#include <QTextCharFormat>
#include <QMap>
#include <QList>
#include <QStringList>

/*============================================================================
================================ ConsoleWidget ===============================
============================================================================*/

class ConsoleWidget : public QWidget
{
    Q_OBJECT
public:
    enum Action
    {
        ClearAction,
        CompileAction,
        CompileAndOpenAction,
        OpenPdfAction,
        OpenPsAction,
        SwitchCompilerAction,
        SettingsAction
    };
public:
    explicit ConsoleWidget(BCodeEditor *cedtr, QWidget *parent = 0);
public:
    bool eventFilter(QObject *object, QEvent *event);
    QAction *consoleAction(Action actId) const;
    QList<QAction *> consoleActions(bool withSeparators = false) const;
private:
    static QString fileNameNoSuffix(const QString &fileName);
private:
    void initKeyMap();
    void initGui();
    QAction *createAction(int id, const QString &iconFileName = QString(), const QString &shortcut = QString(),
                          bool enabled = false);
    void compile(bool op = false);
    void open(bool pdf = true);
    void start( const QString &command, const QStringList &args = QStringList() );
    void start(const QString &command, const QString &arg);
    void noFileNameError();
    void showSettings();
    void setUiEnabled(bool b);
private slots:
    void retranslateUi();
    void performAction(int actId);
    void checkActions(BAbstractCodeEditorDocument *doc);
    void finished(int exitCode);
    void updateSwitchCompilerAction();
private:
    QMap<int, int> mkeyMap;
    QMap<int, QAction *> mactMap;
    QSignalMapper *mmprActions;
    BCodeEditor *mcedtr;
    QString mfileName;
    QString mcommand;
    bool mmakeindex;
    bool mdvips;
    bool mopen;
    bool mremote;
    //
    //vlt
      QToolBar *mtbar;
        //actions
      BTerminalWidget *mtermwgt;
private:
    Q_DISABLE_COPY(ConsoleWidget)
};

#endif // CONSOLEWIDGET_H
