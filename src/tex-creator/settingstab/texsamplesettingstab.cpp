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

#include "texsamplesettingstab.h"

#include "application.h"
#include "settings.h"
#include "texsample/cache.h"
#include "texsample/texsamplecore.h"

#include <BAbstractSettingsTab>
#include <BDirTools>
#include <BGuiTools>
#include <BLoginWidget>
#include <BPassword>
#include <BTranslation>

#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <QToolButton>
#include <QVariant>
#include <QVariantMap>
#include <QVBoxLayout>

/*============================================================================
================================ TexsampleSettingsTab ========================
============================================================================*/

/*============================== Public constructors =======================*/

TexsampleSettingsTab::TexsampleSettingsTab() :
    BAbstractSettingsTab()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Connection", "gbox title"), this);
        QVBoxLayout *vltw = new QVBoxLayout(gbox);
          lgnwgt = new BLoginWidget;
            lgnwgt->setAddressType(BLoginWidget::EditableComboAddress);
            QStringList addresses = Settings::Texsample::hostHistory();
            addresses.prepend(Settings::Texsample::UsueTexsampleServerHost);
            addresses.removeDuplicates();
            lgnwgt->setAvailableAddresses(addresses);
            lgnwgt->setPersistentAddress(Settings::Texsample::UsueTexsampleServerHost);
            lgnwgt->setAddress(Settings::Texsample::host());
            lgnwgt->setLogin(Settings::Texsample::login());
            lgnwgt->setPasswordType(BLoginWidget::SecurePassword);
            lgnwgt->restorePasswordWidgetState(Settings::Texsample::passwordWidgetState());
            lgnwgt->setPassword(Settings::Texsample::password());
            lgnwgt->setLoginLabel(BTranslation::translate("TexsampleSettingsTab", "Login/E-mail:", "lbl text"));
          vltw->addWidget(lgnwgt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Other", "gbox title"), this);
        QFormLayout *flt = new QFormLayout;
          cboxConnectOnStartup = new QCheckBox(gbox);
            cboxConnectOnStartup->setChecked(Settings::Texsample::connectOnStartup());
          flt->addRow(tr("Connect on startup:", "lbl text"), cboxConnectOnStartup);
          QHBoxLayout *hlt = new QHBoxLayout;
            cboxCaching = new QCheckBox(gbox);
              cboxCaching->setChecked(Settings::Texsample::cachingEnabled());
            hlt->addWidget(cboxCaching);
            QPushButton *btn = new QPushButton(tr("Clear cache", "btn text"), gbox);
              connect(btn, SIGNAL(clicked()), this, SLOT(clearCache()));
            hlt->addWidget(btn);
          flt->addRow(tr("Enable caching:", "lbl text"), hlt);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
}

/*============================== Public methods ============================*/

QIcon TexsampleSettingsTab::icon() const
{
    return Application::icon("tex");
}

QString TexsampleSettingsTab::id() const
{
    return "texsample";
}

bool TexsampleSettingsTab::restoreDefault()
{
    lgnwgt->setAddress(Settings::Texsample::UsueTexsampleServerHost);
    return true;
}

bool TexsampleSettingsTab::saveSettings()
{
    QString nhost = lgnwgt->address();
    if (Settings::Texsample::hasTexsample() && Settings::Texsample::host() != nhost) {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Confirmation", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Question);
        msg.setText(tr("You are going to change server address. All cached files will be removed. "
                       "Are you absolutely sure?", "msgbox text"));
        msg.setInformativeText(tr("This action is irreversible!", "msgbox informativeText"));
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg.setDefaultButton(QMessageBox::Yes);
        if (msg.exec() != QMessageBox::Yes)
            return false;
    }
    Settings::Texsample::setConnectOnStartup(cboxConnectOnStartup->isChecked());
    Settings::Texsample::setHost(nhost);
    Settings::Texsample::setHostHistory(lgnwgt->availableAddresses());
    Settings::Texsample::setLogin(lgnwgt->login());
    Settings::Texsample::setPassword(lgnwgt->securePassword());
    Settings::Texsample::setPasswordWidgetState(lgnwgt->savePasswordWidgetState());
    Settings::Texsample::setCachingEnabled(cboxCaching->isChecked());
    tSmp->updateCacheSettings();
    tSmp->updateClientSettings();
    return true;
}

QString TexsampleSettingsTab::title() const
{
    return tr("TeXSample", "title");
}

/*============================== Private slots =============================*/

void TexsampleSettingsTab::clearCache()
{
    QMessageBox msg(this);
    msg.setWindowTitle( tr("Confirmation", "msgbox windowTitle") );
    msg.setIcon(QMessageBox::Question);
    msg.setText(tr("You are going to delete all cached files. Are you absolutely sure?", "msgbox text"));
    msg.setInformativeText(tr("This action is irreversible!", "msgbox informativeText"));
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setDefaultButton(QMessageBox::Yes);
    if (msg.exec() != QMessageBox::Yes)
        return;
    tSmp->cache()->clear();
}
