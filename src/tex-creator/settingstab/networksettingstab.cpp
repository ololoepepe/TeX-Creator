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

#include "networksettingstab.h"

#include "application.h"
#include "settings.h"

#include <BAbstractSettingsTab>
#include <BLoginWidget>

#include <QButtonGroup>
#include <QRadioButton>
#include <QObject>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QString>
#include <QIcon>

/*============================================================================
================================ NetworkSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

NetworkSettingsTab::NetworkSettingsTab() :
    BAbstractSettingsTab()
{
    btngr = new QButtonGroup(this);
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Proxy mode", "gbox title"));
        QHBoxLayout *hlt = new QHBoxLayout(gbox);
          QRadioButton *rbtn = new QRadioButton(tr("No proxy", "rbtn text"));
          hlt->addWidget(rbtn);
          btngr->addButton(rbtn, Settings::Network::NoProxy);
          rbtn = new QRadioButton(tr("System proxy", "rbtn text"));
          hlt->addWidget(rbtn);
          btngr->addButton(rbtn, Settings::Network::SystemProxy);
          rbtn = new QRadioButton(tr("User proxy", "rbtn text"));
          hlt->addWidget(rbtn);
          btngr->addButton(rbtn, Settings::Network::UserProxy);
      vlt->addWidget(gbox);
      lwgt = new BLoginWidget;
        lwgt->setAddressType(BLoginWidget::SimpleAddress, false);
        lwgt->setPortEnabled(true, false);
        lwgt->setLoginRequired(false);
        lwgt->setPasswordType(BLoginWidget::SimplePassword, false);
        lwgt->setAddress(Settings::Network::proxyHost());
        if (Settings::Network::proxyPort())
            lwgt->setPort(Settings::Network::proxyPort());
        lwgt->setLogin(Settings::Network::proxyLogin());
        lwgt->setPassword(Settings::Network::proxyPassword());
      vlt->addWidget(lwgt);
      vlt->addStretch();
    //
    connect(btngr, SIGNAL(buttonClicked(int)), this, SLOT(btnClicked(int)));
    btngr->button(Settings::Network::proxyMode())->setChecked(true);
    btnClicked(Settings::Network::proxyMode());
}

/*============================== Public methods ============================*/

QString NetworkSettingsTab::id() const
{
    return "network";
}

QString NetworkSettingsTab::title() const
{
    return tr("Network", "title");
}

QIcon NetworkSettingsTab::icon() const
{
    return Application::icon("network");
}

bool NetworkSettingsTab::hasDefault() const
{
    return true;
}

bool NetworkSettingsTab::restoreDefault()
{
    btngr->button(Settings::Network::NoProxy)->setChecked(true);
    btnClicked(Settings::Network::NoProxy);
    return true;
}

bool NetworkSettingsTab::saveSettings()
{
    Settings::Network::ProxyMode ppm = Settings::Network::proxyMode();
    Settings::Network::ProxyMode pm = static_cast<Settings::Network::ProxyMode>(btngr->checkedId());
    Settings::Network::setProxyMode(pm);
    Settings::Network::setProxyHost(lwgt->address());
    Settings::Network::setProxyPort(lwgt->port());
    Settings::Network::setProxyLogin(lwgt->login());
    Settings::Network::setProxyPassword(lwgt->simplePassword());
    if (ppm != pm)
        Application::resetProxy();
    return true;
}

/*============================== Private slots =============================*/

void NetworkSettingsTab::btnClicked(int index)
{
    lwgt->setEnabled(Settings::Network::UserProxy == index);
}
