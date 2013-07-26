#include "userwidget.h"
#include "client.h"
#include "application.h"
#include "global.h"

#include <TUserInfo>
#include <TOperationResult>
#include <TAccessLevel>
#include <TeXSample>
#include <TService>
#include <TServiceList>

#include <BDirTools>
#include <BPasswordWidget>
#include <BeQt>
#include <BDialog>
#include <BTextTools>
#include <BInputField>

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
#include <QCheckBox>
#include <QMap>

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
          minputInvite = new BInputField((ShowMode == mmode) ? BInputField::ShowNever : BInputField::ShowAlways);
          minputInvite->addWidget(mledtInvite);
        flt->addRow(tr("Invite:", "lbl text"), minputInvite);
        mledtEmail = new QLineEdit;
          mledtEmail->setValidator(new QRegExpValidator(BTextTools::standardRegExp(BTextTools::EmailPattern), this));
          connect(mledtEmail, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
          minputEmail = new BInputField((ShowMode == mmode) ? BInputField::ShowNever : BInputField::ShowAlways);
          minputEmail->addWidget(mledtEmail);
        flt->addRow(tr("E-mail:", "lbl text"), minputEmail);
        mledtLogin = new QLineEdit;
          mledtLogin->setMaxLength(20);
          mledtLogin->setReadOnly(AddMode != mmode && RegisterMode != mmode);
          connect(mledtLogin, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
          minputLogin = new BInputField((ShowMode == mmode || UpdateMode == mmode) ? BInputField::ShowNever :
                                                                                     BInputField::ShowAlways);
          minputLogin->addWidget(mledtLogin);
        flt->addRow(tr("Login:", "lbl text"), minputLogin);
        mpwdwgt1 = new BPasswordWidget;
          mpwdwgt1->restoreWidgetState(Global::passwordWidgetState());
          mpwdwgt1->setMode(BPassword::AlwaysEncryptedMode);
          mpwdwgt1->setSavePasswordVisible(false);
          mpwdwgt1->setShowPasswordVisible(false);
          mpwdwgt1->setGeneratePasswordVisible(true);
          connect(mpwdwgt1, SIGNAL(passwordChanged()), this, SLOT(checkInputs()));
          minputPwd1 = new BInputField((ShowMode == mmode) ? BInputField::ShowNever : BInputField::ShowAlways);
          minputPwd1->addWidget(mpwdwgt1);
        flt->addRow(tr("Password:", "lbl text"), minputPwd1);
        mpwdwgt2 = new BPasswordWidget;
          mpwdwgt2->restoreWidgetState(Global::passwordWidgetState());
          mpwdwgt2->setMode(BPassword::AlwaysEncryptedMode);
          mpwdwgt2->setSavePasswordVisible(false);
          connect(mpwdwgt1, SIGNAL(showPasswordChanged(bool)), mpwdwgt2, SLOT(setShowPassword(bool)));
          connect(mpwdwgt2, SIGNAL(showPasswordChanged(bool)), mpwdwgt1, SLOT(setShowPassword(bool)));
          connect(mpwdwgt2, SIGNAL(passwordChanged()), this, SLOT(checkInputs()));
          minputPwd2 = new BInputField((ShowMode == mmode) ? BInputField::ShowNever : BInputField::ShowAlways);
          minputPwd2->addWidget(mpwdwgt2);
        flt->addRow(tr("Confirm password:", "lbl text"), minputPwd2);
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
      QVBoxLayout *vlt = new QVBoxLayout;
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
        vlt->addWidget(mtbtnAvatar);
        flt = new QFormLayout;
          foreach (const TService &s, TServiceList::allServices())
          {
              QCheckBox *cbox = new QCheckBox;
                cbox->setEnabled(false);
              flt->addRow(tr("Access to", "lbl text") + " " + s.toString() + ":", cbox);
              mcboxMap.insert(s, cbox);
          }
        vlt->addLayout(flt);
      hlt->addLayout(vlt);
    //
    Application::setRowVisible(minputInvite, RegisterMode == mmode);
    Application::setRowVisible(minputEmail, AddMode == mmode || RegisterMode == mmode);
    Application::setRowVisible(minputPwd1, EditMode != mmode && ShowMode != mmode);
    Application::setRowVisible(minputPwd2, EditMode != mmode && ShowMode != mmode);
    Application::setRowVisible(mcmboxAccessLevel, RegisterMode != mmode);
    foreach (QCheckBox *cbox, mcboxMap)
        Application::setRowVisible(cbox, RegisterMode != mmode);
    checkInputs();
}

UserWidget::~UserWidget()
{
    Global::setPasswordWidgetSate(mpwdwgt1->saveWidgetState());
}

/*============================== Public methods ============================*/

void UserWidget::setAvailableServices(const TServiceList &list)
{
    if (list == mservices)
        return;
    mservices = list;
    foreach (const TService &s, mcboxMap.keys())
        mcboxMap.value(s)->setEnabled((AddMode == mmode || EditMode == mmode) && mservices.contains(s));
}

void UserWidget::setInfo(const TUserInfo &info)
{
    mid = info.id();
    mledtInvite->setText(info.inviteCode());
    mledtEmail->setText(info.email());
    mledtLogin->setText(info.login());
    mpwdwgt1->setPassword(QCryptographicHash::Sha1, info.password());
    mpwdwgt2->setPassword(QCryptographicHash::Sha1, info.password());
    mcmboxAccessLevel->setCurrentIndex(mcmboxAccessLevel->findData((int) info.accessLevel()));
    foreach (const TService &s, mcboxMap.keys())
        mcboxMap.value(s)->setChecked(info.hasAccessToService(s));
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

TServiceList UserWidget::availableServices() const
{
    return mservices;
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
    info.setAccessLevel(mcmboxAccessLevel->itemData(mcmboxAccessLevel->currentIndex()).toInt());
    TServiceList list;
    foreach (const TService &s, mcboxMap.keys())
        if (mcboxMap.value(s)->isChecked())
            list << s;
    info.setServices(list);
    info.setRealName(mledtRealName->text());
    if (!mavatarFileName.isEmpty())
        info.setAvatar(mavatarFileName);
    else
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
    minputInvite->setValid(!mledtInvite->text().isEmpty() && mledtInvite->hasAcceptableInput());
    minputEmail->setValid(!mledtEmail->text().isEmpty() && mledtEmail->hasAcceptableInput());
    minputLogin->setValid(!mledtLogin->text().isEmpty() && mledtLogin->hasAcceptableInput());
    minputPwd1->setValid(!mpwdwgt1->encryptedPassword().isEmpty());
    bool pwdm = mpwdwgt1->encryptedPassword() == mpwdwgt2->encryptedPassword();
    minputPwd2->setValid(minputPwd1->isValid() && pwdm);
    bool v = info().isValid() && (ShowMode == mmode || pwdm);
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
        QString dir = !mavatarFileName.isEmpty() ? mavatarFileName : QDir::homePath();
        QString fn = QFileDialog::getOpenFileName(this, caption, dir, filter);
        if (fn.isEmpty())
            return;
        if (!TUserInfo::testAvatar(fn))
        {
            QMessageBox msg(this);
            msg.setWindowTitle(tr("Failed to change avatar", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to change account avatar. The file is too big", "msgbox text"));
            QString size = BeQt::fileSizeToString(Texsample::MaximumAvatarSize, BeQt::MegabytesFormat);
            QString extent = QString::number(Texsample::MaximumAvatarExtent);
            extent.append("x" + extent + ")").prepend('(');
            msg.setInformativeText(tr("Maximum size:", "msgbox informativeText") + " " + size + " " + extent);
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
        mavatarFileName = fn;
        resetAvatar(data);
    }
}
