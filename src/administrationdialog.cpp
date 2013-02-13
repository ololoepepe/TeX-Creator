#include "administrationdialog.h"
#include "application.h"
#include "client.h"

#include <BPasswordWidget>

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
#include <QDateTimeEdit>
#include <QDateTime>
#include <QClipboard>
#include <QApplication>
#include <QToolButton>
#include <QToolTip>
#include <QPoint>
#include <QUuid>

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
        gbox->setEnabled(sClient->accessLevel() >= Client::AdminLevel);
        QVBoxLayout *vltg = new QVBoxLayout;
          QFormLayout *flt = new QFormLayout;
            mledtLogin = new QLineEdit(gbox);
              connect( mledtLogin, SIGNAL( textChanged(QString) ), this, SLOT( checkAddUser() ) );
            flt->addRow(tr("Login:", "lbl text"), mledtLogin);
            mpwdwgt = new BPasswordWidget(gbox);
              mpwdwgt->setSavePasswordVisible(false);
              connect( mpwdwgt, SIGNAL( passwordChanged() ), this, SLOT( checkAddUser() ) );
            flt->addRow(tr("Password:", "lbl text"), mpwdwgt);
            mledtRealName = new QLineEdit(gbox);
            flt->addRow(tr("Real name:", "lbl text"), mledtRealName);
            mcmboxAccessLevel = new QComboBox(gbox);
              mcmboxAccessLevel->addItem(Client::accessLevelToLocalizedString(Client::NoLevel), Client::NoLevel);
              mcmboxAccessLevel->addItem(Client::accessLevelToLocalizedString(Client::UserLevel), Client::UserLevel);
              mcmboxAccessLevel->addItem(Client::accessLevelToLocalizedString(Client::ModeratorLevel),
                                         Client::ModeratorLevel);
              mcmboxAccessLevel->addItem(Client::accessLevelToLocalizedString(Client::AdminLevel), Client::AdminLevel);
              mcmboxAccessLevel->setCurrentIndex( mcmboxAccessLevel->findData(Client::UserLevel) );
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
      gbox = new QGroupBox( tr("Generating invite code", "gbox title"), this);
        vltg = new QVBoxLayout;
          flt = new QFormLayout;
            mdtedt = new QDateTimeEdit(gbox);
              mdtedt->setMinimumDateTime( QDateTime::currentDateTime().addDays(1) );
              mdtedt->setDateTime( QDateTime::currentDateTime().addDays(3) );
              mdtedt->setMaximumDateTime( QDateTime::currentDateTime().addMonths(1) );
              mdtedt->setDisplayFormat("dd MMMM yyyy hh:mm:ss");
              mdtedt->setCalendarPopup(true);
            flt->addRow(tr("Expiration date:", "lbl text"), mdtedt);
          vltg->addLayout(flt);
          hlt = new QHBoxLayout;
            QPushButton *btn = new QPushButton(tr("Generate", "btn txt"), gbox);
              connect(btn, SIGNAL(clicked()), this, SLOT(generateInvite()));
            hlt->addWidget(btn);
            mledtInvite = new QLineEdit(gbox);
              mledtInvite->setReadOnly(true);
            hlt->addWidget(mledtInvite);
            QToolButton *tbtn = new QToolButton(gbox);
              tbtn->setIcon(Application::icon("editcopy"));
              tbtn->setToolTip(tr("Copy to clipboard", "tbtn toolTip"));
              connect(tbtn, SIGNAL(clicked()), this, SLOT(copyInvite()));
            hlt->addWidget(tbtn);
            tbtn = new QToolButton(gbox);
              tbtn->setIcon(Application::icon("editclear"));
              tbtn->setToolTip(tr("Clear", "tbtn toolTip"));
              connect(tbtn, SIGNAL(clicked()), mledtInvite, SLOT(clear()));
            hlt->addWidget(tbtn);
          vltg->addLayout(hlt);
          hlt = new QHBoxLayout;
            btn = new QPushButton(tr("Update list", "btn text"), gbox);
              connect(btn, SIGNAL(clicked()), this, SLOT(updateInviteList()));
            hlt->addWidget(btn);
            mcmboxInvites = new QComboBox(gbox);
              mcmboxInvites->setSizePolicy(QSizePolicy::Expanding, mcmboxInvites->sizePolicy().verticalPolicy());
              connect(mcmboxInvites, SIGNAL(currentIndexChanged(int)),
                      this, SLOT(cmboxInvitesCurrentIndexChanged(int)));
            hlt->addWidget(mcmboxInvites);
          vltg->addLayout(hlt);
        gbox->setLayout(vltg);
      vlt->addWidget(gbox);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        connect( dlgbbox->addButton(QDialogButtonBox::Close), SIGNAL( clicked() ), this, SLOT( close() ) );
      vlt->addWidget(dlgbbox);
    resize(450, height());
}

/*============================== Private slots =============================*/

void AdministrationDialog::checkAddUser()
{
    mbtnAddUser->setEnabled( !mledtLogin->text().isEmpty() && !mpwdwgt->encryptedPassword().isEmpty() );
}

void AdministrationDialog::addUser()
{
    QString login = mledtLogin->text();
    QByteArray pwd = mpwdwgt->encryptedPassword();
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

void AdministrationDialog::generateInvite()
{
    QUuid invite;
    if (!sClient->generateInvite(invite, mdtedt->dateTime().toUTC(), this) || invite.isNull())
    {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Generating invite failed", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to generate invite", "msgbox text") );
        msg.setInformativeText( tr("This may be due to a connection error", "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    updateInviteList();
}

void AdministrationDialog::copyInvite()
{
    if (mledtInvite->text().isEmpty())
        return;
    QApplication::clipboard()->setText(mledtInvite->text());
    QToolTip::showText(findChildren<QGroupBox *>().last()->mapToGlobal(mledtInvite->pos()),
                       tr("Invite was copied to clipboard", "toolTip"), mledtInvite);
}

void AdministrationDialog::updateInviteList()
{
    QList<Client::Invite> list;
    if ( !sClient->getInvitesList(list, this) )
    {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Updating invites list failed", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to update invites list", "msgbox text") );
        msg.setInformativeText( tr("This may be due to a connection error", "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
    }
    mcmboxInvites->clear();
    foreach (const Client::Invite &inv, list)
        mcmboxInvites->addItem(tr("Expires:", "cmbox item text") + " " + inv.expires.toString("dd MMMM yyyy hh:mm:ss")
                               + " " + BeQt::pureUuidText(inv.uuid), inv.uuid);
    if (mcmboxInvites->count())
        mcmboxInvites->setCurrentIndex(mcmboxInvites->count() - 1);
}

void AdministrationDialog::cmboxInvitesCurrentIndexChanged(int index)
{
    mledtInvite->setText(index >= 0 ? BeQt::pureUuidText(mcmboxInvites->itemData(index).toUuid()) : QString());
    if (mledtInvite->text().isEmpty())
        return;
    mledtInvite->selectAll();
    mledtInvite->setFocus();
    copyInvite();
}
