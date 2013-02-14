#include "texsamplesettingstab.h"
#include "application.h"
#include "client.h"
#include "cache.h"

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
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

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
    QHBoxLayout *hlt = new QHBoxLayout;
      mcboxCaching = new QCheckBox(this);
        mcboxCaching->setChecked( getCachingEnabled() );
      hlt->addWidget(mcboxCaching);
      QPushButton *btn = new QPushButton(tr("Clear cache", "btn text"), this);
        connect( btn, SIGNAL( clicked() ), this, SLOT( clearCache() ) );
      hlt->addWidget(btn);
    flt->addRow(tr("Enable caching:", "lbl text"), hlt);
    mcboxRemoteCompiler = new QCheckBox(this);
      mcboxRemoteCompiler->setChecked(getUseRemoteCompiler());
      mcboxRemoteCompiler->setToolTip(tr("If checked and if you are connected to the TeXSample service, "
                                         "remote compilation system will be used", "cbox toolTip"));
    flt->addRow(tr("Remote compilation:", "lbl text"), mcboxRemoteCompiler);
    mcboxFallbackToLocalCompiler = new QCheckBox(this);
      mcboxFallbackToLocalCompiler->setEnabled(mcboxRemoteCompiler->isChecked());
      mcboxFallbackToLocalCompiler->setToolTip(tr("If checked and if the remote compiler is not available, "
                                                  "the local one will be used", "cbox toolTip"));
      mcboxFallbackToLocalCompiler->setChecked(hasFallbackToLocalCompiler() && getFallbackToLocalCompiler());
      connect(mcboxRemoteCompiler, SIGNAL(clicked(bool)), mcboxFallbackToLocalCompiler, SLOT(setEnabled(bool)));
    flt->addRow(tr("Fallback to remote compiler:", "lbl text"), mcboxFallbackToLocalCompiler);
    //
    setRowVisible(mledtHost, false);
}

/*============================== Static public methods =====================*/

bool TexsampleSettingsTab::hasFallbackToLocalCompiler()
{
    return bSettings->contains("TeXSample/RemoteCompiler/fallback_to_local_compiler");
}

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

bool TexsampleSettingsTab::getCachingEnabled()
{
    return bSettings->value("TeXSample/Cache/enabled", true).toBool();
}

bool TexsampleSettingsTab::getUseRemoteCompiler()
{
    return bSettings->value("TeXSample/RemoteCompiler/use_remote_compiler").toBool();
}

bool TexsampleSettingsTab::getFallbackToLocalCompiler()
{
    return bSettings->value("TeXSample/RemoteCompiler/fallback_to_local_compiler").toBool();
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

void TexsampleSettingsTab::setCachingEnabled(bool enabled)
{
    bSettings->setValue("TeXSample/Cache/enabled", enabled);
}

void TexsampleSettingsTab::setUseRemoteCompiler(bool b)
{
    bSettings->setValue("TeXSample/RemoteCompiler/use_remote_compiler", b);
}

void TexsampleSettingsTab::setFallbackToLocalCompiler(bool b)
{
    bSettings->setValue("TeXSample/RemoteCompiler/fallback_to_local_compiler", b);
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
    setCachingEnabled( mcboxCaching->isChecked() );
    setUseRemoteCompiler(mcboxRemoteCompiler->isChecked());
    if (hasFallbackToLocalCompiler() || mcboxFallbackToLocalCompiler->isChecked())
        setFallbackToLocalCompiler(mcboxFallbackToLocalCompiler->isChecked());
    sClient->updateSettings();
    return true;
}

/*============================== Private slots =============================*/

void TexsampleSettingsTab::clearCache()
{
    if ( !Cache::hasCache() )
        return;
    QMessageBox msg(this);
    msg.setWindowTitle( tr("Confirmation", "msgbox windowTitle") );
    msg.setIcon(QMessageBox::Question);
    msg.setText( tr("You are going to delete all cached files. This action is irreversible", "msgbox text") );
    msg.setInformativeText( tr("Are you absolutely sure?", "msgbox informativeText") );
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setDefaultButton(QMessageBox::Yes);
    if (msg.exec() != QMessageBox::Yes)
        return;
    Cache::clearCache();
}
