#include "userwidget.h"
#include "client.h"
#include "application.h"

#include <TUserInfo>
#include <TOperationResult>
#include <TAccessLevel>
#include <TeXSample>

#include <BDirTools>
#include <BPasswordWidget>

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QFont>
#include <QList>
#include <QComboBox>
#include <QVariant>
#include <QToolButton>
#include <QSize>
#include <QByteArray>
#include <QIcon>
#include <QPixmap>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QScrollArea>

#include <QDebug>

/*============================================================================
================================ UserWidget ==================================
============================================================================*/

/*============================== Public constructors =======================*/

UserWidget::UserWidget(Mode m, QWidget *parent) :
    QWidget(parent), mmode(m)
{
    mvalid = false;
    mid = 0;
    QHBoxLayout *hlt = new QHBoxLayout(this);
      QFormLayout *flt = new QFormLayout;
      if (RegisterMode == m)
      {
          mledtInvite = new QLineEdit;
            mledtInvite->setFont(Application::createMonospaceFont());
            mledtInvite->setInputMask("HHHHHHHH-HHHH-HHHH-HHHH-HHHHHHHHHHHH;_");
            connect(mledtInvite, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
          flt->addRow(tr("Invite:", "lbl text"), mledtInvite);
      }
      if (AddMode == m || RegisterMode == m)
      {
          mledtEmail = new QLineEdit;
            //TODO: Validate
            connect(mledtEmail, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
          flt->addRow(tr("E-mail:", "lbl text"), mledtEmail);
          mledtLogin = new QLineEdit;
            //TODO: Input limits
            mledtLogin->setMaxLength(20);
            connect(mledtLogin, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
          flt->addRow(tr("Login:", "lbl text"), mledtLogin);
      }
      else
      {
          mlblLogin = new QLabel;
          flt->addRow(tr("Login:", "lbl text"), mlblLogin);
      }
      if (ShowMode != m)
      {
          mpwdwgt1 = new BPasswordWidget;
            mpwdwgt1->setSavePasswordVisible(false);
            mpwdwgt1->setShowPasswordVisible(false);
            mpwdwgt1->setGeneratePasswordVisible(true);
            connect(mpwdwgt1, SIGNAL(passwordChanged()), this, SLOT(checkInputs()));
          mpwdwgt2 = new BPasswordWidget;
            mpwdwgt2->setSavePasswordVisible(false);
            connect(mpwdwgt1, SIGNAL(showPasswordChanged(bool)), mpwdwgt2, SLOT(setShowPassword(bool)));
            connect(mpwdwgt2, SIGNAL(showPasswordChanged(bool)), mpwdwgt1, SLOT(setShowPassword(bool)));
            connect(mpwdwgt2, SIGNAL(passwordChanged()), this, SLOT(checkInputs()));
          flt->addRow(tr("Password:", "lbl text"), mpwdwgt1);
          flt->addRow(tr("Confirm password:", "lbl text"), mpwdwgt2);
      }
      if (AddMode == m || EditMode == m)
      {
          mcmboxAccessLevel = new QComboBox;
            foreach (const TAccessLevel &lvl, QList<TAccessLevel>() << TAccessLevel::UserLevel
                     << TAccessLevel::ModeratorLevel << TAccessLevel::AdminLevel)
                mcmboxAccessLevel->addItem(lvl.string(), (int) lvl);
          flt->addRow(tr("Access level:", "lbl text"), mcmboxAccessLevel);
      }
      else if (RegisterMode != m)
      {
          mlblAccessLevel = new QLabel;
          flt->addRow(tr("Access level:", "lbl text"), mlblAccessLevel);
      }
      if (ShowMode != m)
      {
          mledtRealName = new QLineEdit;
          flt->addRow(tr("Real name:", "lbl text"), mledtRealName);
      }
      else
      {
          mlblRealName = new QLabel;
          flt->addRow(tr("Real name:", "lbl text"), mlblRealName);
          BApplication::setRowVisible(mlblRealName, false);
      }
      hlt->addLayout(flt);
      mtbtnAvatar = new QToolButton;
        mtbtnAvatar->setIconSize(QSize(128, 128));
        if (ShowMode != m)
        {
            mtbtnAvatar->setToolTip(tr("Click to select a new picture", "tbtn toolTip"));
            QVBoxLayout *vlt = new QVBoxLayout(mtbtnAvatar);
              vlt->addStretch();
              QToolButton *tbtn = new QToolButton;
                tbtn->setIconSize(QSize(16, 16));
                tbtn->setIcon(Application::icon("editdelete"));
                tbtn->setToolTip(tr("Clear avatar", "tbtn toolTip"));
                connect(tbtn, SIGNAL(clicked()), this, SLOT(resetAvatar()));
              vlt->addWidget(tbtn);
        }
        connect(mtbtnAvatar, SIGNAL(clicked()), this, SLOT(tbtnAvatarClicked()));
        resetAvatar();
      hlt->addWidget(mtbtnAvatar);
    //
    checkInputs();
}

/*============================== Public methods ============================*/

void UserWidget::setInfo(const TUserInfo &info)
{

    if (info.isValid())
    {
        mid = info.id();
        if (AddMode == mmode || RegisterMode == mmode)
        {
            mledtEmail->setText(info.email());
            mledtLogin->setText(info.login());
        }
        else
        {
            mlblLogin->setText(info.login());
        }
        if (ShowMode != mmode)
        {
            mpwdwgt1->setEncryptedPassword(info.password());
            mpwdwgt2->setEncryptedPassword(info.password());
        }
        if (AddMode == mmode || EditMode == mmode)
        {
            mcmboxAccessLevel->setCurrentIndex(mcmboxAccessLevel->findData((int) info.accessLevel()));
            mcmboxAccessLevel->setEnabled(info.accessLevel() < TAccessLevel::AdminLevel);
        }
        else if (RegisterMode != mmode)
        {
            mlblAccessLevel->setText(info.accessLevelString());
            mlblAccessLevel->setProperty("access_level", info.accessLevel());
        }
        if (ShowMode != mmode)
        {
            mledtRealName->setText(info.realName());
        }
        else
        {
            mlblRealName->setText(info.realName());
            BApplication::setRowVisible(mlblRealName, !info.realName().isEmpty());
        }
        resetAvatar(info.avatar());
    }
    else
    {
        mid = 0;
        if (AddMode == mmode || RegisterMode == mmode)
        {
            mledtEmail->clear();
            mledtLogin->clear();
        }
        else
        {
            mlblLogin->clear();
        }
        if (ShowMode != mmode)
            mpwdwgt1->clear();
        if (AddMode == mmode || EditMode == mmode)
        {
            mcmboxAccessLevel->setCurrentIndex(0);
            mcmboxAccessLevel->setEnabled(false);
        }
        else if (RegisterMode != mmode)
            mlblAccessLevel->setText(TAccessLevel::accessLevelToString(TAccessLevel::NoLevel));
        if (ShowMode != mmode)
        {
            mledtRealName->clear();
        }
        else
        {
            mlblRealName->clear();
            BApplication::setRowVisible(mlblRealName, false);
        }
        resetAvatar();
    }
    if (RegisterMode == mmode)
        mledtInvite->clear();
    checkInputs();
}

void UserWidget::setPasswordState(const QByteArray &state)
{
    if (ShowMode == mmode)
        return;
    mpwdwgt1->restoreState(state);
    mpwdwgt2->restoreState(state);
}

UserWidget::Mode UserWidget::mode() const
{
    return mmode;
}

TUserInfo UserWidget::info() const
{
    TUserInfo info;
    info.setId(mid);
    QByteArray pwd;
    if (ShowMode != mmode)
        pwd= (mpwdwgt1->encryptedPassword() == mpwdwgt2->encryptedPassword()) ? mpwdwgt1->encryptedPassword() :
                                                                                QByteArray();
    switch (mmode)
    {
    case AddMode:
        info.setContext(TUserInfo::AddContext);
        info.setEmail(mledtEmail->text());
        info.setLogin(mledtLogin->text());
        info.setPassword(pwd);
        info.setAccessLevel(mcmboxAccessLevel->itemData(mcmboxAccessLevel->currentIndex()).value<TAccessLevel>());
        info.setRealName(mledtRealName->text());
        info.setAvatar(mavatar);
        break;
    case RegisterMode:
        info.setContext(TUserInfo::RegisterContext);
        info.setEmail(mledtEmail->text());
        info.setLogin(mledtLogin->text());
        info.setPassword(pwd);
        info.setRealName(mledtRealName->text());
        info.setAvatar(mavatar);
        break;
    case EditMode:
        info.setContext(TUserInfo::EditContext);
        info.setPassword(pwd);
        info.setAccessLevel(mcmboxAccessLevel->itemData(mcmboxAccessLevel->currentIndex()).value<TAccessLevel>());
        info.setRealName(mledtRealName->text());
        info.setAvatar(mavatar);
        break;
    case UpdateMode:
        info.setContext(TUserInfo::UpdateContext);
        info.setPassword(pwd);
        info.setRealName(mledtRealName->text());
        info.setAvatar(mavatar);
        break;
    case ShowMode:
        info.setContext(TUserInfo::GeneralContext);
        info.setLogin(mlblLogin->text());
        info.setAccessLevel(mlblAccessLevel->property("access_level").value<TAccessLevel>());
        info.setRealName(mlblRealName->text());
        info.setAvatar(mavatar);
        break;
    default:
        break;
    }
    return info;
}

QByteArray UserWidget::passwordState() const
{
    if (ShowMode == mmode)
        return QByteArray();
    QByteArray ba1 = mpwdwgt1->saveStateEncrypted();
    QByteArray ba2 = mpwdwgt2->saveStateEncrypted();
    return (ba1 == ba2) ? ba1 : QByteArray();
}

QString UserWidget::invite() const
{
    return (RegisterMode == mmode) ? mledtInvite->text() : QString();
}

bool UserWidget::isValid() const
{
    return mvalid;
}

bool UserWidget::passwordsMatch() const
{
    return ShowMode != mmode && mpwdwgt1->encryptedPassword() == mpwdwgt2->encryptedPassword();
}

/*============================== Private methods ===========================*/

void UserWidget::resetAvatar(const QString &fileName)
{
    if (fileName.isEmpty())
        return;
    if (QFileInfo(fileName).size() > Texsample::MaximumAvatarSize)
    {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Failed to change avatar", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to change account avatar", "msgbox text"));
        msg.setInformativeText(tr("The file is too big. Maximum allowed size is", "msgbox informativeText") + " "
                               + QString::number(Texsample::MaximumAvatarSize / BeQt::Kilobyte) + " "
                               + tr("KB", "msgbox informativeText"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        return;
    }
    bool ok = false;
    QByteArray data = BDirTools::readFile(fileName, -1, &ok);
    if (!ok || data.isEmpty())
    {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Failed to change avatar", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to change account avatar", "msgbox text"));
        msg.setInformativeText(tr("Some filesystem error occured", "msgbox informativeText"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
        return;
    }
    resetAvatar(data);
}

/*============================== Private slots =============================*/

void UserWidget::resetAvatar(const QByteArray &data)
{
    mavatar = data;
    if (!mavatar.isEmpty())
    {
        QPixmap pm;
        pm.loadFromData(data);
        mtbtnAvatar->setIcon(QIcon(pm));
    }
    else
    {
        mtbtnAvatar->setIcon(Application::icon("user"));
    }
    if (ShowMode == mmode)
    {
        mtbtnAvatar->setToolTip(!mavatar.isEmpty() ? tr("Click to show the picture in full size", "tbtn toolTip") :
                                                     QString());
        mtbtnAvatar->setEnabled(!mavatar.isEmpty());
    }
}

void UserWidget::checkInputs()
{
    bool v = info().isValid() && (ShowMode == mmode || mpwdwgt1->encryptedPassword() == mpwdwgt2->encryptedPassword());
    if (RegisterMode == mmode)
        v = v && !BeQt::uuidFromText(mledtInvite->text()).isNull();
    if (v == mvalid)
        return;
    mvalid = v;
    emit validityChanged(v);
}

void UserWidget::tbtnAvatarClicked()
{
    if (ShowMode == mmode)
    {
        if (mavatar.isEmpty())
            return;
        QPixmap pm;
        pm.loadFromData(mavatar);
        QDialog dlg(this);
        dlg.setWindowTitle(tr("Avatar:", "dlg windowTitle") + " " + mlblLogin->text());
        QVBoxLayout *vlt = new QVBoxLayout(&dlg);
          QScrollArea *sa = new QScrollArea;
            QLabel *lbl = new QLabel;
              lbl->setPixmap(pm);
              lbl->setToolTip(QString::number(pm.width()) + "x" + QString::number(pm.height()));
            sa->setWidget(lbl);
          vlt->addWidget(sa);
          QDialogButtonBox *dlgbbox = new QDialogButtonBox;
            dlgbbox->addButton(QDialogButtonBox::Close);
            connect(dlgbbox->button(QDialogButtonBox::Close), SIGNAL(clicked()), &dlg, SLOT(close()));
          vlt->addWidget(dlgbbox);
          dlg.setMinimumSize(600, 600);
          dlg.resize(600, 600);
        dlg.exec();
    }
    else
    {
        QString caption = tr("Select file", "fdlg caption");
        QString filter = tr("Images", "fdlg filter") + " (*.jpg *.jpeg *.png *.bmp)";
        if (mavatarFileName.isEmpty())
            mavatarFileName = QDir::homePath();
        mavatarFileName = QFileDialog::getOpenFileName(this, caption, mavatarFileName, filter);
        resetAvatar(mavatarFileName);
    }
}
