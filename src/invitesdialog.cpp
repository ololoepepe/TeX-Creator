#include "invitesdialog.h"
#include "application.h"
#include "client.h"

#include <TeXSample>
#include <TInviteInfo>
#include <TOperationResult>
#include <TService>
#include <TServiceList>

#include <BeQt>
#include <BDialog>

#include <QVBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>
#include <QAction>
#include <QDialogButtonBox>
#include <QPushButton>
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
#include <QCheckBox>
#include <QHeaderView>
#include <QMap>

/*============================================================================
================================ InvitesDialog ===============================
============================================================================*/

/*============================== Public constructors =======================*/

InvitesDialog::InvitesDialog(QWidget *parent) :
    BDialog(parent)
{
    setWindowTitle(tr("Invites management", "windowTitle"));
    QWidget *wgt = new QWidget;
      QVBoxLayout *vlt = new QVBoxLayout(wgt);
        mtblwgt = new QTableWidget(0, 2);
        QStringList headers;
        headers << tr("Expiration date", "tblwgt header");
        headers << tr("Available services", "tblwgt header");
        mtblwgt->setHorizontalHeaderLabels(headers);
        mtblwgt->setAlternatingRowColors(true);
        mtblwgt->setEditTriggers(QTableView::NoEditTriggers);
        mtblwgt->setSelectionBehavior(QTableWidget::SelectRows);
        mtblwgt->setSelectionMode(QTableView::SingleSelection);
        mtblwgt->horizontalHeader()->setStretchLastSection(true);
        mtblwgt->verticalHeader()->setVisible(false);
        connect(mtblwgt, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(copyInvite(QTableWidgetItem *)));
        connect(mtblwgt, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)),
                this, SLOT(tblwgtCurrentItemChanged(QTableWidgetItem *, QTableWidgetItem *)));
        vlt->addWidget(mtblwgt);
        mtbar = new QToolBar;
          mtbar->addAction(Application::icon("edit_add"), tr("Generate invites...", "act text"),
                           this, SLOT(generateInvite()));
          mactCopy = mtbar->addAction(Application::icon("editcopy"), tr("Copy selected invite code to clipboard",
                                                                        "act text"), this, SLOT(copyInvite()));
            mactCopy->setEnabled(false);
        vlt->addWidget(mtbar);
      setWidget(wgt);
    addButton(QDialogButtonBox::Close, SLOT(close()));
    resize(450, height());
    //
    updateInvitesList();
}

/*============================== Private slots =============================*/

void InvitesDialog::generateInvite()
{
    BDialog dlg(this);
    dlg.setWindowTitle(tr("Generating invites", "dlg windowTitle"));
      QWidget *wgt = new QWidget;
        QFormLayout *flt = new QFormLayout(wgt);
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
          QMap<TService, QCheckBox *> cboxMap;
          foreach (const TService &s, TServiceList::allServices())
          {
              QCheckBox *cbox = new QCheckBox;
                cbox->setEnabled(sClient->hasAccessToService(s));
              flt->addRow(tr("Access to", "lbl text") + " " + s.toString() + ":", cbox);
              cboxMap.insert(s, cbox);
          }
      dlg.setWidget(wgt);
      dlg.addButton(QDialogButtonBox::Ok, SLOT(accept()));
      dlg.addButton(QDialogButtonBox::Cancel, SLOT(reject()));
    if (dlg.exec() != QDialog::Accepted)
        return;
    TServiceList services;
    foreach (const TService &s, cboxMap.keys())
        if (cboxMap.value(s)->isChecked())
            services << s;
    TInviteInfoList invites;
    TOperationResult r = sClient->generateInvites(invites, dtedt->dateTime().toUTC(), sbox->value(), services, this);
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

void InvitesDialog::copyInvite(QTableWidgetItem *item)
{
    if (!item)
        item = mtblwgt->currentItem();
    if (!item)
        return;
    QApplication::clipboard()->setText(item->toolTip());
    BeQt::waitNonBlocking(100);
    QToolTip::showText(mtblwgt->mapToGlobal(mtblwgt->visualItemRect(item).topLeft()),
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
        mtblwgt->clearContents();
        mtblwgt->setRowCount(0);
    }
    foreach (const TInviteInfo &inv, list)
    {
        mtblwgt->insertRow(mtblwgt->rowCount());
        QTableWidgetItem *twi = new QTableWidgetItem;
        twi->setText(inv.expirationDateTime(Qt::LocalTime).toString("dd MMMM yyyy hh:mm:ss"));
        twi->setToolTip(inv.codeString());
        mtblwgt->setItem(mtblwgt->rowCount() - 1, 0, twi);
        twi = new QTableWidgetItem;
        twi->setText(inv.services().toString());
        twi->setToolTip(inv.codeString());
        mtblwgt->setItem(mtblwgt->rowCount() - 1, 1, twi);
    }
    if (mtblwgt->rowCount())
        mtblwgt->setCurrentCell(mtblwgt->rowCount() - 1, 0);
}

void InvitesDialog::tblwgtCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *)
{
    mactCopy->setEnabled(current);
    copyInvite();
}
