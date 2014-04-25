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

#ifndef KEYBOARDLAYOUTEDITORMODULE_H
#define KEYBOARDLAYOUTEDITORMODULE_H

class BCodeEditor;
class BAbstractCodeEditorDocument;

class QString;

#include <BAbstractEditorModule>

#include <QObject>
#include <QList>
#include <QPointer>
#include <QAction>
#include <QMap>
#include <QChar>

/*============================================================================
================================ KeyboardLayoutMap ===========================
============================================================================*/

class KeyboardLayoutMap
{
public:
    explicit KeyboardLayoutMap();
    KeyboardLayoutMap(const KeyboardLayoutMap &other);
    explicit KeyboardLayoutMap(const QString &fileName);
public:
    bool load(const QString &fileName);
    bool isValid() const;
    bool switchLayout(QString &text) const;
public:
    KeyboardLayoutMap &operator=(const KeyboardLayoutMap &other);
private:
    QMap<QChar, QChar> mdirect;
    QMap<QChar, QChar> mreverse;
    QList<QChar> mdirectUnique;
    QList<QChar> mreverseUnique;
};

/*============================================================================
================================ KeyboardLayoutEditorModule ==================
============================================================================*/

class KeyboardLayoutEditorModule : public BAbstractEditorModule
{
    Q_OBJECT
public:
    enum Action
    {
        SwitchSelectedTextLayoutAction,
        OpenUserKLMDirAction
    };
public:
    explicit KeyboardLayoutEditorModule(QObject *parent = 0);
public:
    QString id() const;
    QAction *action(int type);
    QList<QAction *> actions(bool extended = false);
public slots:
    void switchLayout();
    void reloadMap();
    void openUserDir();
protected:
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void documentHasSelectionChanged(bool hasSelection);
private:
    void checkSwitchAction();
private slots:
    void retranslateUi();
private:
    QPointer<QAction> mactSwitch;
    QPointer<QAction> mactOpenDir;
    KeyboardLayoutMap mmap;
private:
    Q_DISABLE_COPY(KeyboardLayoutEditorModule)
};

#endif // KEYBOARDLAYOUTEDITORMODULE_H
