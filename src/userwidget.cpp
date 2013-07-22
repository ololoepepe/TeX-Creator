#include "userwidget.h"
#include "client.h"
#include "application.h"
#include "global.h"

#include <TUserInfo>
#include <TOperationResult>
#include <TAccessLevel>
#include <TeXSample>

#include <BDirTools>
#include <BPasswordWidget>
#include <BeQt>
#include <BDialog>

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
#include <QVariantMap>
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
#include <QValidator>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSettings>
#include <BDialog>

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
        mledtInvite = new QLineEdit;
          mledtInvite->setFont(Application::createMonospaceFont());
          mledtInvite->setInputMask("HHHHHHHH-HHHH-HHHH-HHHH-HHHHHHHHHHHH;_");
          connect(mledtInvite, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
        flt->addRow(tr("Invite:", "lbl text"), mledtInvite);
        mledtEmail = new QLineEdit;
          mledtEmail->setValidator(new QRegExpValidator(QRegExp(BeQt::standardRegExpPattern(BeQt::EmailPattern)),
                                                        this));
          connect(mledtEmail, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
        flt->addRow(tr("E-mail:", "lbl text"), mledtEmail);
        mledtLogin = new QLineEdit;
          mledtLogin->setMaxLength(20);
          mledtLogin->setReadOnly(AddMode != mmode && RegisterMode != mmode);
          connect(mledtLogin, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
        flt->addRow(tr("Login:", "lbl text"), mledtLogin);
        mpwdwgt1 = new BPasswordWidget;
          mpwdwgt1->restoreWidgetState(Global::passwordWidgetState());
          mpwdwgt1->setMode(BPassword::AlwaysEncryptedMode);
          mpwdwgt1->setSavePasswordVisible(false);
          mpwdwgt1->setShowPasswordVisible(false);
          mpwdwgt1->setGeneratePasswordVisible(true);
          connect(mpwdwgt1, SIGNAL(passwordChanged()), this, SLOT(checkInputs()));
        flt->addRow(tr("Password:", "lbl text"), mpwdwgt1);
        mpwdwgt2 = new BPasswordWidget;
          mpwdwgt2->restoreWidgetState(Global::passwordWidgetState());
          mpwdwgt2->setMode(BPassword::AlwaysEncryptedMode);
          mpwdwgt2->setSavePasswordVisible(false);
          connect(mpwdwgt1, SIGNAL(showPasswordChanged(bool)), mpwdwgt2, SLOT(setShowPassword(bool)));
          connect(mpwdwgt2, SIGNAL(showPasswordChanged(bool)), mpwdwgt1, SLOT(setShowPassword(bool)));
          connect(mpwdwgt2, SIGNAL(passwordChanged()), this, SLOT(checkInputs()));
        flt->addRow(tr("Confirm password:", "lbl text"), mpwdwgt2);
        mcmboxAccessLevel = new QComboBox;
          mcmboxAccessLevel->setEnabled(AddMode == mmode || EditMode == mmode);
          foreach (const TAccessLevel &lvl, TAccessLevel::allAccessLevels())
              mcmboxAccessLevel->addItem(lvl.toString(), (int) lvl);
          mcmboxAccessLevel->setCurrentIndex(0);
        flt->addRow(tr("Access level:", "lbl text"), mcmboxAccessLevel);
        mledtRealName = new QLineEdit;
          mledtRealName->setReadOnly(ShowMode == mmode);
          mledtRealName->setMaxLength(50);
        flt->addRow(tr("Real name:", "lbl text"), mledtRealName);
      hlt->addLayout(flt);
      mtbtnAvatar = new QToolButton;
        mtbtnAvatar->setIconSize(QSize(128, 128));
        if (ShowMode != m)
        {
            mtbtnAvatar->setToolTip(tr("Click to select a new picture", "tbtn toolTip"));
            QVBoxLayout *vlt = new QVBoxLayout(mtbtnAvatar);
              vlt->addStretch();
              mtbtnClearAvatar = new QToolButton;
                mtbtnClearAvatar->setIconSize(QSize(16, 16));
                mtbtnClearAvatar->setIcon(Application::icon("editdelete"));
                mtbtnClearAvatar->setToolTip(tr("Clear avatar", "tbtn toolTip"));
                connect(mtbtnClearAvatar, SIGNAL(clicked()), this, SLOT(resetAvatar()));
              vlt->addWidget(mtbtnClearAvatar);
        }
        connect(mtbtnAvatar, SIGNAL(clicked()), this, SLOT(tbtnAvatarClicked()));
        resetAvatar();
      hlt->addWidget(mtbtnAvatar);
    //
    Application::setRowVisible(mledtInvite, RegisterMode == mmode);
    Application::setRowVisible(mledtEmail, AddMode == mmode || RegisterMode == mmode);
    Application::setRowVisible(mpwdwgt1, EditMode != mmode && ShowMode != mmode);
    Application::setRowVisible(mpwdwgt2, EditMode != mmode && ShowMode != mmode);
    Application::setRowVisible(mcmboxAccessLevel, RegisterMode != mmode);
    checkInputs();
}

UserWidget::~UserWidget()
{
    Global::setPasswordWidgetSate(mpwdwgt1->saveWidgetState());
}

/*============================== Public methods ============================*/

void UserWidget::setInfo(const TUserInfo &info)
{
    mid = info.id();
    mledtInvite->setText(info.inviteCode());
    mledtEmail->setText(info.email());
    mledtLogin->setText(info.login());
    mpwdwgt1->setPassword(QCryptographicHash::Sha1, info.password());
    mpwdwgt2->setPassword(QCryptographicHash::Sha1, info.password());
    mcmboxAccessLevel->setCurrentIndex(mcmboxAccessLevel->findData((int) info.accessLevel()));
    mledtRealName->setText(info.realName());
    resetAvatar(info.avatar());
    checkInputs();
}

void UserWidget::setPassword(const BPassword &pwd)
{
    mpwdwgt1->setPassword(pwd);
    mpwdwgt2->setPassword(pwd);
}

void UserWidget::restoreState(const QByteArray &state)
{
    QVariantMap m = BeQt::deserialize(state).toMap();
    mavatarFileName = m.value("avatar_file_name").toString();
}

UserWidget::Mode UserWidget::mode() const
{
    return mmode;
}

TUserInfo UserWidget::info() const
{
    TUserInfo info;
    switch (mmode)
    {
    case AddMode:
        info.setContext(TUserInfo::AddContext);
        break;
    case RegisterMode:
        info.setContext(TUserInfo::RegisterContext);
        break;
    case EditMode:
        info.setContext(TUserInfo::EditContext);
        break;
    case UpdateMode:
        info.setContext(TUserInfo::UpdateContext);
        break;
    case ShowMode:
        info.setContext(TUserInfo::GeneralContext);
        break;
    default:
        break;
    }
    info.setId(mid);
    if (mledtInvite->hasAcceptableInput())
        info.setInviteCode(mledtInvite->text());
    if (mledtEmail->hasAcceptableInput())
        info.setEmail(mledtEmail->text());
    info.setLogin(mledtLogin->text());
    if (mpwdwgt1->encryptedPassword() == mpwdwgt2->encryptedPassword())
        info.setPassword(mpwdwgt1->encryptedPassword());
    info.setAccessLevel(mcmboxAccessLevel->itemData(mcmboxAccessLevel->currentIndex()).value<TAccessLevel>());
    info.setRealName(mledtRealName->text());
    info.setAvatar(mavatar);
    return info;
}

BPassword UserWidget::password() const
{
    return mpwdwgt1->password();
}

QByteArray UserWidget::saveState() const
{
    QVariantMap m;
    m.insert("avatar_file_name", mavatarFileName);
    return BeQt::serialize(m);
}

bool UserWidget::isValid() const
{
    return mvalid;
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
        if (ShowMode != mmode)
            mtbtnClearAvatar->setEnabled(true);
    }
    else
    {
        mtbtnAvatar->setIcon(Application::icon("user"));
        if (ShowMode != mmode)
            mtbtnClearAvatar->setEnabled(false);
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
        BDialog dlg(this);
          dlg.setWindowTitle(tr("Avatar:", "dlg windowTitle") + " " + mledtLogin->text());
          dlg.addButton(QDialogButtonBox::Close, SLOT(close()));
          QScrollArea *sa = new QScrollArea;
            QLabel *lbl = new QLabel;
              QPixmap pm;
              pm.loadFromData(mavatar);
              lbl->setPixmap(pm);
              lbl->setToolTip(QString::number(pm.width()) + "x" + QString::number(pm.height()));
            sa->setWidget(lbl);
          dlg.setWidget(sa);
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
        QString fn = QFileDialog::getOpenFileName(this, caption, mavatarFileName, filter);
        if (fn.isEmpty())
            return;
        mavatarFileName = fn;
        if (QFileInfo(fn).size() > Texsample::MaximumAvatarSize)
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
        QByteArray data = BDirTools::readFile(fn, -1, &ok);
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
}
