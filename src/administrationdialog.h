#ifndef ADMINISTRATIONDIALOG_H
#define ADMINISTRATIONDIALOG_H

class BPasswordWidget;

class QLineEdit;
class QComboBox;
class QPushButton;
class QDateTimeEdit;

#include <QDialog>

/*============================================================================
================================ AdministrationDialog ========================
============================================================================*/

class AdministrationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdministrationDialog(QWidget *parent = 0);
private slots:
    void addUser();
    void generateInvite();
    void copyInvite();
    void updateInviteList();
    void cmboxInvitesCurrentIndexChanged(int index);
private:
    QDateTimeEdit *mdtedt;
    QLineEdit *mledtInvite;
    QComboBox *mcmboxInvites;
private:
    Q_DISABLE_COPY(AdministrationDialog)
};

#endif // ADMINISTRATIONDIALOG_H
