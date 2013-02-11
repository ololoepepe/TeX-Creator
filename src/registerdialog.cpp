#include "registerdialog.h"
#include "requestprogressdialog.h"
#include "texsamplesettingstab.h"

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
          connect(mledtInvite, SIGNAL(textChanged(QString)), this, SLOT(checkRegister()));
        flt->addRow(tr("Invite:", "lbl text"), mledtInvite);
        mledtLogin = new QLineEdit(this);
          connect(mledtLogin, SIGNAL(textChanged(QString)), this, SLOT(checkRegister()));
        flt->addRow(tr("Login:", "lbl text"), mledtLogin);
        mpwdwgt = new BPasswordWidget(this);
          mpwdwgt->setSavePasswordVisible(false);
          connect(mpwdwgt, SIGNAL(passwordChanged()), this, SLOT(checkRegister()));
        flt->addRow(tr("Password:", "lbl text"), mpwdwgt);
      vlt->addLayout(flt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        connect(dlgbbox->addButton(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        mbtnRegister = dlgbbox->addButton(tr("Register", "btn text"), QDialogButtonBox::AcceptRole);
        connect(mbtnRegister, SIGNAL(clicked()), this, SLOT(registerMe()));
      vlt->addWidget(dlgbbox);
}

/*============================== Private slots =============================*/

void RegisterDialog::checkRegister()
{
    mbtnRegister->setEnabled(!mledtInvite->text().isEmpty() && !mledtLogin->text().isEmpty()
                             && !mpwdwgt->encryptedPassword().isEmpty());
}

void RegisterDialog::registerMe()
{
    BNetworkConnection c(BGenericSocket::TcpSocket);
    if (!c.connectToHostBlocking(TexsampleSettingsTab::getHost(), 9041, 5 * BeQt::Second) || !c.isConnected())
    {
        c.close();
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Connection error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to connect to server", "msgbox text"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    QVariantMap out;
    out.insert("invite", mledtInvite->text());
    out.insert("login", mledtLogin->text());
    out.insert("password", mpwdwgt->encryptedPassword());
    BNetworkOperation *op = c.sendRequest("register", out);
    if ( !op->waitForFinished(BeQt::Second / 2) )
        RequestProgressDialog(op, this).exec();
    QVariantMap in = op->variantData().toMap();
    op->deleteLater();
    c.close();
    if (op->isError() || !in.value("ok").toBool())
    {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Registration error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to register", "msgbox text"));
        msg.setInformativeText(tr("This may be due to a connection error", "msgbox informativeText"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    TexsampleSettingsTab::setLogin(mledtLogin->text());
    BPasswordWidget::PasswordWidgetData pd = BPasswordWidget::stateToData(TexsampleSettingsTab::getPasswordState());
    pd.password = mpwdwgt->password();
    pd.encryptedPassword = mpwdwgt->encryptedPassword();
    pd.charCount = pd.password.length();
    TexsampleSettingsTab::setPasswordSate( BPasswordWidget::dataToState(pd) );
    accept();
}
