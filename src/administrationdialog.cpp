#include "administrationdialog.h"
#include "application.h"
#include "client.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include <QHBoxLayout>
#include <QByteArray>
#include <QCryptographicHash>
#include <QVariant>
#include <QMessageBox>

/*============================================================================
================================ AdministrationDialog ========================
============================================================================*/

/*============================== Public constructors =======================*/

AdministrationDialog::AdministrationDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle( tr("Administration", "windowTitle") );
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox( tr("Adding user", "gbox title"), this);
        QVBoxLayout *vltg = new QVBoxLayout;
          QFormLayout *flt = new QFormLayout;
            mledtLogin = new QLineEdit(gbox);
              connect( mledtLogin, SIGNAL( textChanged(QString) ), this, SLOT( checkAddUser() ) );
            flt->addRow(tr("Login:", "lbl text"), mledtLogin);
            mledtPassword = new QLineEdit(gbox);
              connect( mledtPassword, SIGNAL( textChanged(QString) ), this, SLOT( checkAddUser() ) );
            flt->addRow(tr("Password:", "lbl text"), mledtPassword);
            mledtRealName = new QLineEdit(gbox);
            flt->addRow(tr("Real name:", "lbl text"), mledtRealName);
            mcmboxAccessLevel = new QComboBox(gbox);
              mcmboxAccessLevel->addItem(Client::accessLevelToLocalizedString(Client::UserLevel), Client::UserLevel);
              mcmboxAccessLevel->addItem(Client::accessLevelToLocalizedString(Client::ModeratorLevel),
                                         Client::ModeratorLevel);
              mcmboxAccessLevel->addItem(Client::accessLevelToLocalizedString(Client::AdminLevel), Client::AdminLevel);
              mcmboxAccessLevel->setCurrentIndex(0);
            flt->addRow(tr("Access level:", "lbl text"), mcmboxAccessLevel);
          vltg->addLayout(flt);
          QHBoxLayout *hlt = new QHBoxLayout;
            hlt->addStretch();
            mbtnAddUser = new QPushButton(tr("Add user", "btn text"), gbox);
              checkAddUser();
              connect( mbtnAddUser, SIGNAL( clicked() ), this, SLOT( addUser() ) );
            hlt->addWidget(mbtnAddUser);
          vltg->addLayout(hlt);
        gbox->setLayout(vltg);
      vlt->addWidget(gbox);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        connect( dlgbbox->addButton(QDialogButtonBox::Close), SIGNAL( clicked() ), this, SLOT( close() ) );
      vlt->addWidget(dlgbbox);
}

/*============================== Private slots =============================*/

void AdministrationDialog::checkAddUser()
{
    mbtnAddUser->setEnabled( !mledtLogin->text().isEmpty() && !mledtPassword->text().isEmpty() );
}

void AdministrationDialog::addUser()
{
    QString login = mledtLogin->text();
    QByteArray pwd = QCryptographicHash::hash(mledtPassword->text().toUtf8(), QCryptographicHash::Sha1);
    QString name = mledtRealName->text();
    int lvl = mcmboxAccessLevel->itemData( mcmboxAccessLevel->currentIndex() ).toInt();
    if ( !sClient->addUser(login, pwd, name, lvl) )
    {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Adding user failed", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to add user", "msgbox text") );
        msg.setInformativeText( tr("Maybe the user already exists, or a connection error occured",
                                   "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
    }
}
