/****************************************************************************
**
** Copyright (C) 2014 Andrey Bogdanov
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
class BProperties;
class BVersion;

class QByteArray;
class QMainWindow;
class QPixmap;
class QStringList;

#include "modulecomponents.h"

#include <CodeEditorModulePluginInterface>

#include <BGuiPluginInterface>
#include <BPluginInterface>

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QtPlugin>

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
    static void clearExecutionStack();
    static void clearExecutionStack(PretexEditorModule *module);
    static QByteArray executionStackState(PretexEditorModule *module = 0);
    static BProperties externalTools();
    static PretexEditorModulePlugin *instance();
    static QByteArray moduleState(PretexEditorModule *module = 0);
    static bool saveExecutionStack();
    static void setExecutionStackState(const QByteArray &state, PretexEditorModule *module = 0);
    static void setExternalTools(const BProperties &map);
    static void setModuleState(const QByteArray &state, PretexEditorModule *module = 0);
    static void setSaveExecutionStack(bool b);
public:
    void activate();
    BAboutDialog *createAboutDialog();
    QList<BAbstractSettingsTab *> createSettingsTabs();
    void deactivate();
    QString helpIndex() const;
    QStringList helpSearchPaths() const;
    QString id() const;
    PluginInfo info() const;
    bool installModule(BCodeEditor *cedtr, QMainWindow *mw);
    QPixmap pixmap() const;
    bool prefereStaticInfo() const;
    void processStandardAboutDialog(BAboutDialog *dlg) const;
    StaticPluginInfo staticInfo() const;
    QString title() const;
    QString type() const;
    bool uninstallModule(BCodeEditor *cedtr, QMainWindow *mw);
    BVersion version() const;
private:
    static QString path(const QString &key = QString(), const QString &section = QString(),
                        PretexEditorModule *module = 0);
private slots:
    void retranslateUi();
};

#endif // PRETEXEDITORMODULEPLUGIN_H
