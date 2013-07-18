#include "invitesdialog.h"
#include "application.h"
#include "client.h"

#include <TeXSample>
#include <TInviteInfo>
#include <TOperationResult>

#include <BeQt>

#include <QDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolBar>
#include <QAction>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QListWidgetItem>
#include <QFormLayout>
#include <QVariant>
#include <QMessageBox>
#include <QDateTimeEdit>
#include <QDateTime>
#include <QString>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>
#include <QPoint>
#include <QSpinBox>

/*============================================================================
================================ InvitesDialog ===============================
============================================================================*/

/*============================== Public constructors =======================*/

InvitesDialog::InvitesDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Invites management", "windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(this);
      mlstwgt = new QListWidget;
      connect(mlstwgt, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
              this, SLOT(lstwgtCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));
      connect(mlstwgt, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(copyInvite(QListWidgetItem *)));
      vlt->addWidget(mlstwgt);
      mtbar = new QToolBar;
        mtbar->addAction(Application::icon("edit_add"), tr("Generate invites...", "act text"),
                        this, SLOT(generateInvite()));
        mactCopy = mtbar->addAction(Application::icon("editcopy"), tr("Copy selected invite to clipboard", "act text"),
                                    this, SLOT(copyInvite()));
          mactCopy->setEnabled(false);
      vlt->addWidget(mtbar);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        connect(dlgbbox->addButton(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
      vlt->addWidget(dlgbbox);
    resize(450, height());
    //
    updateInvitesList();
}

/*============================== Private slots =============================*/

void InvitesDialog::generateInvite()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Generating invites", "dlg windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      QFormLayout *flt = new QFormLayout;
        QDateTimeEdit *dtedt = new QDateTimeEdit;
          dtedt->setMinimumDateTime(QDateTime::currentDateTime().addDays(1));
          dtedt->setMaximumDateTime(QDateTime::currentDateTime().addMonths(1));
          dtedt->setDateTime(QDateTime::currentDateTime().addDays(3));
          dtedt->setDisplayFormat("dd MMMM yyyy hh:mm:ss");
          dtedt->setCalendarPopup(true);
        flt->addRow(tr("Expiration date:", "lbl text"), dtedt);
        QSpinBox *sbox = new QSpinBox;
          sbox->setMinimum(1);
          sbox->setMaximum(Texsample::MaximumInvitesCount);
          sbox->setValue(1);
        flt->addRow(tr("Count:", "lbl text"), sbox);
      vlt->addLayout(flt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Ok);
        connect(dlgbbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dlg, SLOT(accept()));
        dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(dlgbbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dlg, SLOT(reject()));
      vlt->addWidget(dlgbbox);
    if (dlg.exec() != QDialog::Accepted)
        return;
    TInviteInfoList invites;
    TOperationResult r = sClient->generateInvites(invites, dtedt->dateTime().toUTC(), sbox->value(), this);
    if (!r)
    {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Generating invites failed", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to generate invites. The following error occured:", "msgbox text"));
        msg.setInformativeText(r.messageString());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    updateInvitesList(invites);
}

void InvitesDialog::copyInvite(QListWidgetItem *item)
{
    if (!item)
        item = mlstwgt->currentItem();
    if (!item)
        return;
    QApplication::clipboard()->setText(item->toolTip());
    BeQt::waitNonBlocking(100);
    QToolTip::showText(mlstwgt->mapToGlobal(mlstwgt->visualItemRect(item).topLeft()),
                       tr("Invite was copied to clipboard", "toolTip"), mtbar);
}

void InvitesDialog::updateInvitesList(TInviteInfoList list)
{
    if (list.isEmpty())
    {
        if (!sClient->getInvitesList(list, this))
        {
            QMessageBox msg(this);
            msg.setWindowTitle(tr("Updating invites list failed", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to update invites list", "msgbox text"));
            msg.setInformativeText(tr("This may be due to a connection error", "msgbox informativeText"));
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
        }
        mlstwgt->clear();
    }
    foreach (const TInviteInfo &inv, list)
    {
        QListWidgetItem *lwi = new QListWidgetItem;
        lwi->setText(inv.expirationDateTime(Qt::LocalTime).toString("dd MMMM yyyy hh:mm:ss"));
        lwi->setToolTip(inv.codeString());
        mlstwgt->addItem(lwi);
    }
    if (mlstwgt->count())
        mlstwgt->setCurrentItem(mlstwgt->item(mlstwgt->count() - 1));
}

void InvitesDialog::lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *)
{
    mactCopy->setEnabled(current);
    copyInvite();
}
