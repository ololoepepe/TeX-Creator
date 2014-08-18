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

class BAbstractCodeEditorDocument;
class BCodeEditor;
class BTerminalWidget;

class QAction;
class QSignalMapper;
class QToolBar;

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>

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
        SettingsAction,
        SwitchCompilerAction
    };
private:
    QMap<int, QAction *> mactMap;
    bool malwaysLatin;
    BCodeEditor *mcedtr;
    QString mcommand;
    bool mdvips;
    QString mfileName;
    bool mmakeindex;
     Qt::KeyboardModifiers mmodifiers;
    QSignalMapper *mmprActions;
    bool mopen;
    bool mremote;
    QToolBar *mtbar;
    BTerminalWidget *mtermwgt;
public:
    explicit ConsoleWidget(BCodeEditor *cedtr, QWidget *parent = 0);
public:
    QAction *consoleAction(Action actId) const;
    QList<QAction *> consoleActions(bool withSeparators = false) const;
    bool eventFilter(QObject *object, QEvent *event);
public slots:
    void updateSwitchCompilerAction();
private:
    static QString fileNameNoSuffix(const QString &fileName);
private:
    void compile(bool op = false);
    QAction *createAction(int id, const QString &iconFileName = QString(), const QString &shortcut = QString(),
                          bool enabled = false);
    void noFileNameError();
    void open(bool pdf = true);
    void setUiEnabled(bool b);
    void start(const QString &command, const QStringList &args = QStringList());
    void start(const QString &command, const QString &arg);
private slots:
    void checkActions(BAbstractCodeEditorDocument *doc);
    void finished(int exitCode);
    void inputLatinKey(int key);
    void performAction(int actId);
    void retranslateUi();
private:
    Q_DISABLE_COPY(ConsoleWidget)
};

#endif // CONSOLEWIDGET_H
