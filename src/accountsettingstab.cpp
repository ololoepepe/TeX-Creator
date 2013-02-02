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
#include <QToolButton>
#include <QSize>
#include <QPixmap>
#include <QImage>
#include <QVariant>
#include <QIcon>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>

#include <QDebug>

/*============================================================================
================================ AccountSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

AccountSettingsTab::AccountSettingsTab() :
    BAbstractSettingsTab()
{
    mpwd = TexsampleSettingsTab::getPassword();
    mrealName = sClient->realName();
    mavatar = sClient->avatar();
    //
    QFormLayout *flt = new QFormLayout(this);
    mledtName = new QLineEdit(this);
      mledtName->setText(mrealName);
    flt->addRow(tr("Real name:", "lbl text"), mledtName);
    mpwdwgt = new BPasswordWidget(this);
      mpwdwgt->restoreState( TexsampleSettingsTab::getPasswordState() );
    flt->addRow(tr("Password:", "lbl text"), mpwdwgt);
    mtbtnAvatar = new QToolButton(this);
      mtbtnAvatar->setIconSize( QSize(128, 128) );
      mtbtnAvatar->setToolTip( tr("Click to select a new picture", "tbtn toolTip") );
      QPixmap pm = QPixmap::fromImage(mavatar);
      if ( !pm.isNull() )
      {
          mtbtnAvatar->setIcon( QIcon(pm) );
          mtbtnAvatar->setProperty("has_avatar", true);
      }
      else
      {
          mtbtnAvatar->setIcon( Application::icon("user") );
      }
      connect( mtbtnAvatar, SIGNAL( clicked() ), this, SLOT( tbtntAvatarClicked() ) );
    flt->addRow(tr("Avatar:", "lbl text"), mtbtnAvatar);
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
    QIcon icn = mtbtnAvatar->icon();
    QImage ava = mtbtnAvatar->property("has_image").toBool() ? icn.pixmap( icn.availableSizes().first() ).toImage() :
                                                               mavatar;
    if (pwd == mpwd && name == mrealName && ava == mavatar)
        return true;
    if ( sClient->updateAccount(pwd, name, ava, mavatarFormat, this) )
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

/*============================== Private slots =============================*/

void AccountSettingsTab::tbtntAvatarClicked()
{
    QString caption = tr("Select file", "fdlg caption");
    QString filter = tr("Images", "fdlg filter") + " (*.jpg *.jpeg *.png *.bmp)";
    QString fn = QFileDialog::getOpenFileName(this, caption, QDir::homePath(), filter);
    if ( fn.isEmpty() )
        return;
    QIcon icn(fn);
    if ( icn.isNull() )
        return; //Show message
    mtbtnAvatar->setIcon(icn);
    mtbtnAvatar->setProperty("has_image", true);
    mavatarFormat = QFileInfo(fn).suffix();
}
