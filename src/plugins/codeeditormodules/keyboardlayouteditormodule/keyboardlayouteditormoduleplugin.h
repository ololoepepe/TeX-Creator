/****************************************************************************
**
** Copyright (C) 2014 TeXSample Team
**
** This file is part of the Kayboard Layout Editor Module plugin
** of TeX Creator.
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

#ifndef KEYBOARDLAYOUTEDITORMODULEPLUGIN_H
#define KEYBOARDLAYOUTEDITORMODULEPLUGIN_H

class BAboutDialog;
class BAbstractSettingsTab;
class BCodeEditor;
class BLocationProvider;
class BVersion;

class QFileSystemWatcher;
class QMainWindow;
class QPixmap;
class QString;
class QStringList;

#include "modulecomponents.h"

#include <CodeEditorModulePluginInterface>

#include <BGuiPluginInterface>
#include <BPluginInterface>

#include <QMap>
#include <QObject>
#include <QtPlugin>

/*============================================================================
================================ KeyboardLayoutEditorModulePlugin ============
============================================================================*/

class KeyboardLayoutEditorModulePlugin : public QObject, public CodeEditorModulePluginInterface,
        public BPluginInterface, public BGuiPluginInterface
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "TeX-Creator.KeyboardLayoutEditorModulePlugin")
#endif
    Q_OBJECT
    Q_INTERFACES(BPluginInterface)
    Q_INTERFACES(BGuiPluginInterface)
    Q_INTERFACES(CodeEditorModulePluginInterface)
private:
    QFileSystemWatcher *mfsWatcher;
    QMap<BCodeEditor *, ModuleComponents> mmap;
    BLocationProvider *mprovider;
public:
    explicit KeyboardLayoutEditorModulePlugin();
    ~KeyboardLayoutEditorModulePlugin();
public:
    void activate();
    BAboutDialog *createAboutDialog();
    BAbstractSettingsTab *createSettingsTab();
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
private slots:
    void directoryChanged(const QString &path);
};

#endif // KEYBOARDLAYOUTEDITORMODULEPLUGIN_H
