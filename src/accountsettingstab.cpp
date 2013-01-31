#include "accountsettingstab.h"
#include "texsamplesettingstab.h"
#include "application.h"
#include "client.h"

#include <BAbstractSettingsTab>
#include <BPasswordWidget>

#include <QString>
#include <QIcon>
#include <QFormLayout>
#include <QByteArray>
#include <QMessageBox>
#include <QLineEdit>

/*============================================================================
================================ AccountSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

AccountSettingsTab::AccountSettingsTab() :
    BAbstractSettingsTab()
{
    mpwd = TexsampleSettingsTab::getPassword();
    mrealName = sClient->realName();
    //
    QFormLayout *flt = new QFormLayout(this);
    mledtName = new QLineEdit(this);
      mledtName->setText(mrealName);
    flt->addRow(tr("Real name:", "lbl text"), mledtName);
    mpwdwgt = new BPasswordWidget(this);
      mpwdwgt->restoreState( TexsampleSettingsTab::getPasswordState() );
    flt->addRow(tr("Password:", "lbl text"), mpwdwgt);
}

/*============================== Public methods ============================*/

QString AccountSettingsTab::title() const
{
    return tr("Account", "title");
}

QIcon AccountSettingsTab::icon() const
{
    return Application::icon("user");
}

bool AccountSettingsTab::saveSettings()
{
    QByteArray pwd = mpwdwgt->encryptedPassword();
    QString name = mledtName->text();
    if (pwd == mpwd && name == mrealName)
        return true;
    if ( sClient->updateAccount(pwd, name, this) )
    {
        TexsampleSettingsTab::setPasswordSate( mpwdwgt->saveStateEncrypted() );
        if ( !sClient->updateSettings() )
            sClient->reconnect();
        return true;
    }
    else
    {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Changing account failed", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to change account settings", "msgbox text") );
        msg.setInformativeText( tr("This may be due to connection error", "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return false;
    }
}
