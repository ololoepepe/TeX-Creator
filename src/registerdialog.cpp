#include "registerdialog.h"
#include "requestprogressdialog.h"
#include "texsamplesettingstab.h"
#include "application.h"
#include "client.h"

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
    BNetworkConnection c(BGenericSocket::TcpSocket);
    QString host = TexsampleSettingsTab::getHost();
    c.connectToHost(host.compare("auto_select") ? host : QString("texsample-server.no-ip.org"), 9041);
    if (!c.isConnected() && !c.waitForConnected(BeQt::Second / 2))
    {
        QProgressDialog pd(this);
        pd.setWindowTitle(tr("Connecting to server", "pdlg windowTitle"));
        pd.setLabelText(tr("Connecting to server, please, wait...", "pdlg labelText"));
        pd.setMinimum(0);
        pd.setMaximum(0);
        QTimer::singleShot(10 * BeQt::Second, &pd, SLOT(close()));
        if (pd.exec() == QProgressDialog::Rejected)
            return c.close();
    }
    if (!c.isConnected())
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
    out.insert("invite", BeQt::uuidFromText(mledtInvite->text()));
    out.insert("login", mledtLogin->text());
    out.insert("password", mpwdwgt1->encryptedPassword());
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
    TexsampleSettingsTab::setPasswordSate(mpwdwgt1->saveStateEncrypted());
    sClient->updateSettings();
    sClient->connectToServer();
    accept();
}
