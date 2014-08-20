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

class QCheckBox;
class QFormLayout;
class QHBoxLayout;
class QIcon;
class QLineEdit;
class QString;

#include <BAbstractSettingsTab>

#include <QMap>
#include <QObject>
#include <QString>

/*============================================================================
================================ PretexSettingsTab ===========================
============================================================================*/

class PretexSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
private:
    QCheckBox *cboxSaveStack;
    QFormLayout *flt;
    QMap<QObject *, QHBoxLayout *> layoutMap;
    QMap<QObject *, QLineEdit *> pathMap;
public:
    explicit PretexSettingsTab();
public:
    QIcon icon() const;
    QString id() const;
    bool saveSettings();
    QString title() const;
private slots:
    void addRow(const QString &name = QString(), const QString &path = QString());
    void clearStack();
    void removeRow();
    void search();
private:
    Q_DISABLE_COPY(PretexSettingsTab)
};

#endif // PRETEXSETTINGSTAB_H
