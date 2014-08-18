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

#ifndef PRETEXSETTINGSTAB_H
#define PRETEXSETTINGSTAB_H

class QString;
class QIcon;
class QFormLayout;
class QLineEdit;
class QHBoxLayout;
class QCheckBox;

#include <BAbstractSettingsTab>

#include <QObject>
#include <QString>
#include <QMap>

/*============================================================================
================================ PretexSettingsTab ===========================
============================================================================*/

class PretexSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit PretexSettingsTab();
public:
    QString id() const;
    QString title() const;
    QIcon icon() const;
    bool saveSettings();
private slots:
    void addRow(const QString &name = QString(), const QString &path = QString());
    void removeRow();
    void search();
    void clearStack();
private:
    QCheckBox *cboxSaveStack;
    QFormLayout *flt;
    QMap<QObject *, QLineEdit *> pathMap;
    QMap<QObject *, QHBoxLayout *> layoutMap;
private:
    Q_DISABLE_COPY(PretexSettingsTab)
};

#endif // PRETEXSETTINGSTAB_H
