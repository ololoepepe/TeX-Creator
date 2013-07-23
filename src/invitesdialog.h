#ifndef INVITESDIALOG_H
#define INVITESDIALOG_H

class BPasswordWidget;

class QTableWidget;
class QTableWidgetItem;
class QToolBar;
class QAction;

#include <TInviteInfo>
#include <TInviteInfoList>

#include <BDialog>

/*============================================================================
================================ InvitesDialog ===============================
============================================================================*/

class InvitesDialog : public BDialog
{
    Q_OBJECT
public:
    explicit InvitesDialog(QWidget *parent = 0);
private slots:
    void generateInvite();
    void copyInvite(QTableWidgetItem *item = 0);
    void updateInvitesList(TInviteInfoList list = TInviteInfoList());
    void tblwgtCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
private:
    QTableWidget *mtblwgt;
    QToolBar *mtbar;
    QAction *mactCopy;
private:
    Q_DISABLE_COPY(InvitesDialog)
};

#endif // INVITESDIALOG_H
