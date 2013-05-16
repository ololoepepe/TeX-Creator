#ifndef INVITESDIALOG_H
#define INVITESDIALOG_H

class BPasswordWidget;

class QListWidget;
class QListWidgetItem;
class QToolBar;
class QAction;

#include <TInviteInfo>

#include <QDialog>

/*============================================================================
================================ InvitesDialog ===============================
============================================================================*/

class InvitesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InvitesDialog(QWidget *parent = 0);
private slots:
    void generateInvite();
    void copyInvite(QListWidgetItem *item = 0);
    void updateInvitesList(TInviteInfo::InvitesList list = TInviteInfo::InvitesList());
    void lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
private:
    QListWidget *mlstwgt;
    QToolBar *mtbar;
    QAction *mactCopy;
private:
    Q_DISABLE_COPY(InvitesDialog)
};

#endif // INVITESDIALOG_H
