/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
**
** This file is part of the MacrosEditorModule plugin of TeX Creator.
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

class MacrosEditorModule;

class BAbstractSettingsTab;
class BCodeEditor;

class QString;
class QPixmap;
class QMainWindow;
class QByteArray;
class QMenu;
class QDockWidget;

#include "modulecomponents.h"

#include <CodeEditorModulePluginInterface>

#include <BPluginInterface>
#include <BGuiPluginInterface>

#include <QObject>
#include <QtPlugin>
#include <QVariantMap>
#include <QMap>

/*============================================================================
================================ PretexEditorModule ==========================
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
public:
    static void setMacrosModuleState(const QByteArray &state);
    static void setSaveMacroStack(bool b);
    static void setExternalTools(const QMap<QString, QString> &map);
    static QByteArray macrosModuleState();
    static bool saveMacroStack();
    static QMap<QString, QString> externalTools();
public:
    explicit PretexEditorModulePlugin();
    ~PretexEditorModulePlugin();
public:
    QString type() const;
    QString name() const;
    bool prefereStaticInfo() const;
    PluginInfoStatic staticInfo() const;
    PluginInfo info() const;
    void activate();
    void deactivate();
    QPixmap pixmap() const;
    BAbstractSettingsTab *settingsTab() const;
    void handleSettings(const QVariantMap &s);
    bool installModule(BCodeEditor *cedtr, QMainWindow *mw);
    bool uninstallModule(BCodeEditor *cedtr, QMainWindow *mw);
private slots:
    void retranslateUi();
private:
    QMap<BCodeEditor *, ModuleComponents> mmap;
};

#endif // PRETEXEDITORMODULEPLUGIN_H
