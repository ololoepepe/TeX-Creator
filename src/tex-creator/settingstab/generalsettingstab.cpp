/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
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

#include "generalsettingstab.h"

#include "application.h"
#include "settings.h"
#include "texsample/texsamplecore.h"

#include <BAbstractSettingsTab>
#include <BLocaleComboBox>

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include <QString>

/*============================================================================
================================ GeneralSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

GeneralSettingsTab::GeneralSettingsTab() :
    BAbstractSettingsTab()
{
    QFormLayout *flt = new QFormLayout(this);
    mlcmbox = new BLocaleComboBox(this);
      mlcmbox->setAvailableLocales(Application::availableLocales(true));
      mlcmbox->setCurrentLocale(Application::locale());
    flt->addRow("Language (" + tr("language", "lbl text") + "):", mlcmbox);
    mcboxMultipleWindows = new QCheckBox(this);
      mcboxMultipleWindows->setChecked(Settings::General::multipleWindowsEnabled());
    flt->addRow(tr("Enable multiple windows:", "lbl text"), mcboxMultipleWindows);
    QHBoxLayout *hlt = new QHBoxLayout;
      mcboxNewVersions = new QCheckBox(this);
        mcboxNewVersions->setChecked(Settings::General::checkForNewVersionOnStartup());
      hlt->addWidget(mcboxNewVersions);
      QPushButton *btn = new QPushButton(tr("Check now", "btn text"));
        connect(btn, SIGNAL(clicked()), tSmp, SLOT(checkForNewVersionPersistent()));
      hlt->addWidget(btn);
    flt->addRow(tr("Check for new versions:", "lbl text"), hlt);
}

/*============================== Public methods ============================*/

QIcon GeneralSettingsTab::icon() const
{
    return Application::icon("configure");
}

QString GeneralSettingsTab::id() const
{
    return "general";
}

bool GeneralSettingsTab::hasDefault() const
{
    return true;
}

bool GeneralSettingsTab::restoreDefault()
{
    mcboxMultipleWindows->setChecked(false);
    return true;
}

bool GeneralSettingsTab::saveSettings()
{
    if (Settings::General::multipleWindowsEnabled() && !mcboxMultipleWindows->isChecked() && !bApp->mergeWindows()) {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Failed to change settings", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Information);
        msg.setText(tr("Can't disable multiple windows: documents merging failed", "msgbox text"));
        msg.setInformativeText(tr("Please, close all duplicate documents and try again", "msgbox informativeText"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return false;
    }
    Application::setLocale(mlcmbox->currentLocale());
    Settings::General::setMultipleWindowsEnabled(mcboxMultipleWindows->isChecked());
    Settings::General::setCheckForNewVersionOnStartup(mcboxNewVersions->isChecked());
    return true;
}

QString GeneralSettingsTab::title() const
{
    return tr("General", "title");
}
