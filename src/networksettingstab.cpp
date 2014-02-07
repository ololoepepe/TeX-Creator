#include "networksettingstab.h"
#include "application.h"
#include "global.h"

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
          btngr->addButton(rbtn, Global::NoProxy);
          rbtn = new QRadioButton(tr("System proxy", "rbtn text"));
          hlt->addWidget(rbtn);
          btngr->addButton(rbtn, Global::SystemProxy);
          rbtn = new QRadioButton(tr("User proxy", "rbtn text"));
          hlt->addWidget(rbtn);
          btngr->addButton(rbtn, Global::UserProxy);
      vlt->addWidget(gbox);
      lwgt = new BLoginWidget;
        lwgt->setAddressType(BLoginWidget::SimpleAddress, false);
        lwgt->setPortEnabled(true, false);
        lwgt->setLoginRequired(false);
        lwgt->setPasswordType(BLoginWidget::SimplePassword, false);
        lwgt->setAddress(Global::proxyHost());
        if (Global::proxyPort())
            lwgt->setPort(Global::proxyPort());
        lwgt->setLogin(Global::proxyLogin());
        lwgt->setPassword(Global::proxyPassword());
      vlt->addWidget(lwgt);
      vlt->addStretch();
    //
    connect(btngr, SIGNAL(buttonClicked(int)), this, SLOT(btnClicked(int)));
    btngr->button(Global::proxyMode())->setChecked(true);
    btnClicked(Global::proxyMode());
}

/*============================== Public methods ============================*/

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
    btngr->button(Global::NoProxy)->setChecked(true);
    btnClicked(Global::NoProxy);
    return true;
}

bool NetworkSettingsTab::saveSettings()
{
    Global::ProxyMode ppm = Global::proxyMode();
    Global::ProxyMode pm = static_cast<Global::ProxyMode>(btngr->checkedId());
    Global::setProxyMode(pm);
    Global::setProxyHost(lwgt->address());
    Global::setProxyPort(lwgt->port());
    Global::setProxyLogin(lwgt->login());
    Global::setProxyPassword(lwgt->simplePassword());
    if (ppm != pm)
        Application::resetProxy();
    return true;
}

/*============================== Private slots =============================*/

void NetworkSettingsTab::btnClicked(int index)
{
    lwgt->setEnabled(Global::UserProxy == index);
}
