#include "accountsettingstab.h"
#include "texsamplesettingstab.h"
#include "application.h"
#include "client.h"

#include <BAbstractSettingsTab>
#include <BPasswordWidget>
#include <BDirTools>

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
    QPixmap pm;
    mhasAvatar = pm.loadFromData(mavatar) && !pm.isNull();
    //
    QFormLayout *flt = new QFormLayout(this);
    mledtName = new QLineEdit(this);
      mledtName->setMaxLength(128);
      mledtName->setText(mrealName);
    flt->addRow(tr("Real name:", "lbl text"), mledtName);
    mpwdwgt = new BPasswordWidget(this);
      mpwdwgt->restoreState( TexsampleSettingsTab::getPasswordState() );
    flt->addRow(tr("Password:", "lbl text"), mpwdwgt);
    mtbtnAvatar = new QToolButton(this);
      mtbtnAvatar->setIconSize( QSize(128, 128) );
      mtbtnAvatar->setToolTip( tr("Click to select a new picture", "tbtn toolTip") );
      mtbtnAvatar->setIcon( mhasAvatar ? QIcon(pm) : Application::icon("user") );
      QVBoxLayout *vlt = new QVBoxLayout(mtbtnAvatar);
        vlt->addStretch();
        QToolButton *tbtn = new QToolButton(mtbtnAvatar);
          tbtn->setIconSize(QSize(16, 16));
          tbtn->setIcon(Application::icon("editdelete"));
          tbtn->setToolTip(tr("Remove avatar", "tbtn toolTip"));
          connect(tbtn, SIGNAL(clicked()), this, SLOT(clearAvatar()));
        vlt->addWidget(tbtn);
      connect( mtbtnAvatar, SIGNAL( clicked() ), this, SLOT( selectAvatar() ) );
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
    if (pwd == mpwd && name == mrealName && sClient->avatar() == mavatar)
        return true;
    if ( sClient->updateAccount(pwd, name, mavatar, this) )
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

void AccountSettingsTab::selectAvatar()
{
    QString caption = tr("Select file", "fdlg caption");
    QString filter = tr("Images", "fdlg filter") + " (*.jpg *.jpeg *.png *.bmp)";
    QString fn = QFileDialog::getOpenFileName(this, caption, QDir::homePath(), filter);
    int code = 0;
    if ( fn.isEmpty() )
        return;
    if (QFileInfo(fn).size() > MaxAvatarFileSize)
        code = 2; //File is too big
    if (!code)
    {
        bool ok = false;
        mavatar = BDirTools::readFile(fn, -1, &ok);
        QPixmap pm;
        if ( !ok || !pm.loadFromData(mavatar) || pm.isNull() )
            code = 3;
        else if (qMax( pm.height(), pm.width() ) > MaxAvatarSize)
            code = 4;
    }
    if (code)
    {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Failed to change avatar", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to change account avatar", "msgbox text") );
        switch (code)
        {
        case 2:
        {
            QString szs = QString::number(MaxAvatarFileSize / BeQt::Kilobyte);
            msg.setInformativeText( tr("The file is too big. Maximum allowed size is", "msgbox informativeText") + " "
                                    + szs + " " + tr("KB", "msgbox informativeText") );
            break;
        }
        case 3:
            msg.setInformativeText( tr("Invalid file is selected", "msgbox informativeText") );
            break;
        case 4:
        {
            QString szs = QString::number(MaxAvatarSize);
            msg.setInformativeText(tr("The image is too big. Maximum allowed size is", "msgbox informativeText") + " "
                                   + szs + "x" + szs);
            break;
        }
        default:
            break;
        }
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    mtbtnAvatar->setIcon( QIcon(fn) );
    mhasAvatar = true;
}

void AccountSettingsTab::clearAvatar()
{
    if (!mhasAvatar)
        return;
    mavatar.clear();
    mtbtnAvatar->setIcon(Application::icon("user"));
    mhasAvatar = true;
}

/*============================== Static private constants ==================*/

const int AccountSettingsTab::MaxAvatarFileSize = BeQt::Megabyte;
const int AccountSettingsTab::MaxAvatarSize = 2048;
