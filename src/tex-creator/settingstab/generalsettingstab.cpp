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

#include "generalsettingstab.h"

#include "application.h"
#include "global.h"

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
    mlcmbox = new BLocaleComboBox(true, this);
      mlcmbox->setCurrentLocale( Application::locale() );
    flt->addRow("Language (" + tr("language", "lbl text") + "):", mlcmbox);
    mcboxMultipleWindows = new QCheckBox(this);
      mcboxMultipleWindows->setChecked(Global::multipleWindowsEnabled());
    flt->addRow(tr("Enable multiple windows:", "lbl text"), mcboxMultipleWindows);
    QHBoxLayout *hlt = new QHBoxLayout;
      mcboxNewVersions = new QCheckBox(this);
        mcboxNewVersions->setChecked(Global::checkForNewVersions());
      hlt->addWidget(mcboxNewVersions);
      QPushButton *btn = new QPushButton(tr("Check now", "btn text"));
        connect(btn, SIGNAL(clicked()), bApp, SLOT(checkForNewVersionsSlot()));
      hlt->addWidget(btn);
    flt->addRow(tr("Check for new versions:", "lbl text"), hlt);
}

/*============================== Public methods ============================*/

QString GeneralSettingsTab::title() const
{
    return tr("General", "title");
}

QIcon GeneralSettingsTab::icon() const
{
    return Application::icon("configure");
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
    if (Global::multipleWindowsEnabled() && !mcboxMultipleWindows->isChecked() && !Application::mergeWindows()) {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Failed to change settings", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Information);
        msg.setText( tr("Can't disable multiple windows: documents merging failed", "msgbox text") );
        msg.setInformativeText( tr("Please, close all duplicate documents and try again", "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return false;
    }
    Application::setLocale(mlcmbox->currentLocale());
    Global::setMultipleWindowsEnabled(mcboxMultipleWindows->isChecked());
    Global::setCheckForNewVersions(mcboxNewVersions->isChecked());
    return true;
}
