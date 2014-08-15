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

#ifndef TEXSAMPLESETTINGSTAB_H
#define TEXSAMPLESETTINGSTAB_H

class BPasswordWidget;

class QIcon;
class QCheckBox;
class QLineEdit;
class QByteArray;
class QComboBox;
class QHBoxLayout;
class QToolButton;

#include <BAbstractSettingsTab>

#include <QObject>
#include <QVariantMap>
#include <QString>
#include <QStringList>

/*============================================================================
================================ TexsampleSettingsTab ========================
============================================================================*/

class TexsampleSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit TexsampleSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool restoreDefault();
    bool saveSettings();
private:
    QStringList updateHostHistory(const QStringList &history = QStringList());
private slots:
    void clearCache();
    void removeCurrentHostFromHistory();
    void cmboxHostCurrentIndexChanged(int index);
private:
    QString autoSelectText;
    QHBoxLayout *mhltHost;
      QComboBox *mcmboxHost;
      QToolButton *mtbtnRemoveFromHistory;
    QLineEdit *mledtLogin;
    BPasswordWidget *mpwdwgt;
    QCheckBox *mcboxAutoconnection;
    QCheckBox *mcboxCaching;
private:
    Q_DISABLE_COPY(TexsampleSettingsTab)
};

#endif // TEXSAMPLESETTINGSTAB_H
