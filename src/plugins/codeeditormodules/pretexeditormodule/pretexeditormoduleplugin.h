/****************************************************************************
**
** Copyright (C) 2014 TeXSample Team
**
** This file is part of the PreTeX Editor Module plugin of TeX Creator.
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

#ifndef PRETEXEDITORMODULEPLUGIN_H
#define PRETEXEDITORMODULEPLUGIN_H

class PretexEditorModule;

class BAboutDialog;
class BAbstractSettingsTab;
class BCodeEditor;
class BLocationProvider;
class BVersion;

class QPixmap;
class QMainWindow;
class QByteArray;
class QStringList;

#include "modulecomponents.h"

#include <CodeEditorModulePluginInterface>

#include <BPluginInterface>
#include <BGuiPluginInterface>

#include <QList>
#include <QObject>
#include <QtPlugin>
#include <QMap>
#include <QString>

/*============================================================================
================================ PretexEditorModulePlugin ====================
============================================================================*/

class PretexEditorModulePlugin : public QObject, public CodeEditorModulePluginInterface, public BPluginInterface,
        public BGuiPluginInterface
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "TeX-Creator.PretexEditorModulePlugin")
#endif
    Q_OBJECT
    Q_INTERFACES(BPluginInterface)
    Q_INTERFACES(BGuiPluginInterface)
    Q_INTERFACES(CodeEditorModulePluginInterface)
private:
    static PretexEditorModulePlugin *minstance;
private:
    QMap<BCodeEditor *, ModuleComponents> mmap;
    BLocationProvider *mprovider;
public:
    explicit PretexEditorModulePlugin();
    ~PretexEditorModulePlugin();
public:
    static PretexEditorModulePlugin *instance();
    static void setExecutionStackState(const QByteArray &state, PretexEditorModule *module = 0);
    static void setModuleState(const QByteArray &state, PretexEditorModule *module = 0);
    static void setSaveExecutionStack(bool b);
    static void setExternalTools(const QMap<QString, QString> &map);
    static QByteArray executionStackState(PretexEditorModule *module = 0);
    static QByteArray moduleState(PretexEditorModule *module = 0);
    static bool saveExecutionStack();
    static QMap<QString, QString> externalTools();
    static void clearExecutionStack();
    static void clearExecutionStack(PretexEditorModule *module);
public:
    QString type() const;
    QString id() const;
    QString title() const;
    bool prefereStaticInfo() const;
    StaticPluginInfo staticInfo() const;
    PluginInfo info() const;
    void activate();
    void deactivate();
    QPixmap pixmap() const;
    QList<BAbstractSettingsTab *> createSettingsTabs();
    QStringList helpSearchPaths() const;
    QString helpIndex() const;
    BAboutDialog *createAboutDialog();
    void processStandardAboutDialog(BAboutDialog *dlg) const;
    bool installModule(BCodeEditor *cedtr, QMainWindow *mw);
    bool uninstallModule(BCodeEditor *cedtr, QMainWindow *mw);
    BVersion version() const;
private slots:
    void retranslateUi();
};

#endif // PRETEXEDITORMODULEPLUGIN_H
