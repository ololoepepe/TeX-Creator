#include "administrationdialog.h"
#include "application.h"
#include "client.h"
#include "userwidget.h"

#include <TInviteInfo>
#include <TOperationResult>
#include <TUserInfo>
#include <TAccessLevel>

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
#include <QToolBar>
#include <QAction>

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
Q_DECLARE_METATYPE(QUuid)
#endif

/*============================================================================
================================ AdministrationDialog ========================
============================================================================*/

/*============================== Public constructors =======================*/

AdministrationDialog::AdministrationDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle( tr("Administration", "windowTitle") );
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QToolBar *tbar = new QToolBar;
        tbar->addAction(Application::icon("add_user"), tr("Add user", "act text"), this, SLOT(addUser()));
      vlt->addWidget(tbar);
      QGroupBox *gbox = new QGroupBox( tr("Generating invite code", "gbox title"), this);
        QVBoxLayout *vltg = new QVBoxLayout;
          QFormLayout *flt = new QFormLayout;
            mdtedt = new QDateTimeEdit(gbox);
              mdtedt->setMinimumDateTime( QDateTime::currentDateTime().addDays(1) );
              mdtedt->setDateTime( QDateTime::currentDateTime().addDays(3) );
              mdtedt->setMaximumDateTime( QDateTime::currentDateTime().addMonths(1) );
              mdtedt->setDisplayFormat("dd MMMM yyyy hh:mm:ss");
              mdtedt->setCalendarPopup(true);
            flt->addRow(tr("Expiration date:", "lbl text"), mdtedt);
          vltg->addLayout(flt);
          QHBoxLayout *hlt = new QHBoxLayout;
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

void AdministrationDialog::addUser()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Adding user", "dlg windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      UserWidget *uwgt = new UserWidget(UserWidget::AddMode);
      vlt->addWidget(uwgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Ok);
        dlgbbox->button(QDialogButtonBox::Ok)->setEnabled(false);
        connect(uwgt, SIGNAL(validityChanged(bool)), dlgbbox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
        connect(dlgbbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dlg, SLOT(accept()));
        dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(dlgbbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dlg, SLOT(reject()));
      vlt->addWidget(dlgbbox);
      dlg.setFixedSize(640, 240);
    while (dlg.exec() == QDialog::Accepted)
    {
        TUserInfo info = uwgt->info();
        TOperationResult r = sClient->addUser(info, this);
        if (r)
        {
            return;
        }
        else
        {
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Adding user error", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to add user due to the following error:", "msgbox text"));
            msg.setInformativeText(r.errorString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
        }
    }
}

void AdministrationDialog::generateInvite()
{
    TInviteInfo::InvitesList invites;
    if (!sClient->generateInvites(invites, mdtedt->dateTime().toUTC(), 1, this))
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
    //TODO: Just append the invites
    //updateInviteList();
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
    TInviteInfo::InvitesList list;
    if (!sClient->getInvitesList(list, this))
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
    foreach (const TInviteInfo &inv, list)
        mcmboxInvites->addItem(tr("Expires:", "cmbox item text") + " " +
                               inv.expirationDateTime(Qt::LocalTime).toString("dd MMMM yyyy hh:mm:ss")
                               + " " + inv.uuidString(), QVariant::fromValue<QUuid>(inv.uuid()));
    if (mcmboxInvites->count())
        mcmboxInvites->setCurrentIndex(mcmboxInvites->count() - 1);
}

void AdministrationDialog::cmboxInvitesCurrentIndexChanged(int index)
{
    mledtInvite->setText(index >= 0 ? BeQt::pureUuidText(mcmboxInvites->itemData(index).value<QUuid>()) : QString());
    if (mledtInvite->text().isEmpty())
        return;
    mledtInvite->selectAll();
    mledtInvite->setFocus();
    copyInvite();
}
