#include "texsamplesettingstab.h"
#include "application.h"
#include "client.h"

#include <BAbstractSettingsTab>
#include <BPasswordWidget>
#include <BDirTools>

#include <QObject>
#include <QVariantMap>
#include <QString>
#include <QIcon>
#include <QCheckBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QVariant>
#include <QByteArray>
#include <QSettings>

/*============================================================================
================================ TexsampleSettingsTab ========================
============================================================================*/

/*============================== Public constructors =======================*/

TexsampleSettingsTab::TexsampleSettingsTab() :
    BAbstractSettingsTab()
{
    QFormLayout *flt = new QFormLayout(this);
    mcboxAutoconnection = new QCheckBox(this);
      mcboxAutoconnection->setChecked( getAutoconnection() );
    flt->addRow(tr("Autoconnection:", "lbl text"), mcboxAutoconnection);
    mledtHost = new QLineEdit(this);
      mledtHost->setText( getHost() );
    flt->addRow(tr("Host:", "lbl text"), mledtHost);
    mledtLogin = new QLineEdit(this);
      mledtLogin->setText( getLogin() );
    flt->addRow(tr("Login:", "lbl text"), mledtLogin);
    mpwdwgt = new BPasswordWidget(this);
      mpwdwgt->restoreState( getPasswordState() );
    flt->addRow(tr("Password:", "lbl text"), mpwdwgt);
    //
    setRowVisible(mledtHost, false);
}

/*============================== Static public methods =====================*/

bool TexsampleSettingsTab::getAutoconnection()
{
    return bSettings->value("TeXSample/Client/autoconnection", true).toBool();
}

QString TexsampleSettingsTab::getHost()
{
    return bSettings->value("TeXSample/Client/host", "texsample-server.no-ip.org").toString();
}

QString TexsampleSettingsTab::getLogin()
{
    return bSettings->value("TeXSample/Client/login").toString();
}

QByteArray TexsampleSettingsTab::getPasswordState()
{
    return bSettings->value("TeXSample/Client/password_state").toByteArray();
}

QByteArray TexsampleSettingsTab::getPassword()
{
    return BPasswordWidget::stateToData( getPasswordState() ).encryptedPassword;
}

void TexsampleSettingsTab::setAutoconnection(bool enabled)
{
    bSettings->setValue("TeXSample/Client/autoconnection", enabled);
}

void TexsampleSettingsTab::setHost(const QString &host)
{
    bSettings->setValue("TeXSample/Client/host", host);
}

void TexsampleSettingsTab::setLogin(const QString &login)
{
    bSettings->setValue("TeXSample/Client/login", login);
}

void TexsampleSettingsTab::setPasswordSate(const QByteArray &state)
{
    bSettings->setValue("TeXSample/Client/password_state", state);
}

/*============================== Public methods ============================*/

QString TexsampleSettingsTab::title() const
{
    return tr("TeXSample", "title");
}

QIcon TexsampleSettingsTab::icon() const
{
    return Application::icon("tex");
}

bool TexsampleSettingsTab::hasAdvancedMode() const
{
    return true;
}

void TexsampleSettingsTab::setAdvancedMode(bool enabled)
{
    setRowVisible(mledtHost, enabled);
}

bool TexsampleSettingsTab::restoreDefault()
{
    mledtHost->setText("texsample-server.no-ip.org");
    return true;
}

bool TexsampleSettingsTab::saveSettings()
{
    setAutoconnection( mcboxAutoconnection->isChecked() );
    setHost( mledtHost->text() );
    setLogin( mledtLogin->text() );
    setPasswordSate( mpwdwgt->saveStateEncrypted() );
    sClient->updateSettings();
    return true;
}
