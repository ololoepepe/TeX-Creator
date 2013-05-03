#include "registerdialog.h"
#include "requestprogressdialog.h"
#include "texsamplesettingstab.h"
#include "application.h"
#include "client.h"

#include <TUserInfo>
#include <TeXSample>
#include <TOperationResult>

#include <BPasswordWidget>
#include <BNetworkConnection>
#include <BGenericSocket>
#include <BNetworkOperation>

#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QByteArray>
#include <QVariantMap>
#include <QMessageBox>
#include <QTimer>
#include <QProgressDialog>

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
Q_DECLARE_METATYPE(QUuid)
#endif

/*============================================================================
================================ RegisterDialog ==============================
============================================================================*/

/*============================== Public constructors =======================*/

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Registration", "windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QFormLayout *flt = new QFormLayout;
        mledtInvite = new QLineEdit(this);
          //TODO: Set mask
          connect(mledtInvite, SIGNAL(textChanged(QString)), this, SLOT(checkRegister()));
        flt->addRow(tr("Invite:", "lbl text"), mledtInvite);
        mledtLogin = new QLineEdit(this);
          //TODO: Set max line length
          connect(mledtLogin, SIGNAL(textChanged(QString)), this, SLOT(checkRegister()));
        flt->addRow(tr("Login:", "lbl text"), mledtLogin);
        mpwdwgt1 = new BPasswordWidget(this);
          mpwdwgt1->setSavePasswordVisible(false);
          mpwdwgt1->restoreState(TexsampleSettingsTab::getPasswordState());
          mpwdwgt1->clear();
          connect(mpwdwgt1, SIGNAL(passwordChanged()), this, SLOT(checkRegister()));
        flt->addRow(tr("Password:", "lbl text"), mpwdwgt1);
        mpwdwgt2 = new BPasswordWidget(this);
          mpwdwgt2->setSavePasswordVisible(false);
          mpwdwgt2->restoreState(TexsampleSettingsTab::getPasswordState());
          mpwdwgt2->clear();
          connect(mpwdwgt2, SIGNAL(passwordChanged()), this, SLOT(checkRegister()));
          connect(mpwdwgt1, SIGNAL(showPasswordChanged(bool)), mpwdwgt2, SLOT(setShowPassword(bool)));
          connect(mpwdwgt2, SIGNAL(showPasswordChanged(bool)), mpwdwgt1, SLOT(setShowPassword(bool)));
        flt->addRow(tr("Confirm password:", "lbl text"), mpwdwgt2);
      vlt->addLayout(flt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        connect(dlgbbox->addButton(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        mbtnRegister = dlgbbox->addButton(tr("Register", "btn text"), QDialogButtonBox::AcceptRole);
        mbtnRegister->setEnabled(false);
        connect(mbtnRegister, SIGNAL(clicked()), this, SLOT(registerMe()));
      vlt->addWidget(dlgbbox);
}

/*============================== Private slots =============================*/

void RegisterDialog::checkRegister()
{
    mbtnRegister->setEnabled(!mledtInvite->text().isEmpty() && !mledtLogin->text().isEmpty()
                             && !mpwdwgt1->encryptedPassword().isEmpty()
                             && mpwdwgt1->encryptedPassword() == mpwdwgt2->encryptedPassword());
}

void RegisterDialog::registerMe()
{
    TUserInfo info(TUserInfo::RegisterContext);
    info.setLogin(mledtLogin->text());
    info.setPassword(mpwdwgt1->encryptedPassword());
    TOperationResult r = Client::registerUser(info, mledtInvite->text(), this);
    if (!r)
    {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Registration error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to register due to the following error:", "msgbox text"));
        msg.setInformativeText(r.errorString());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    TexsampleSettingsTab::setLogin(info.login());
    TexsampleSettingsTab::setPasswordSate(info.password());
    sClient->updateSettings();
    sClient->connectToServer();
    accept();
}
