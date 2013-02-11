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
    void checkAddUser();
    void addUser();
    void generateInvite();
private:
    QLineEdit *mledtLogin;
    BPasswordWidget *mpwdwgt;
    QLineEdit *mledtRealName;
    QComboBox *mcmboxAccessLevel;
    QPushButton *mbtnAddUser;
    QDateTimeEdit *mdtedt;
    QLineEdit *mledtInvite;
private:
    Q_DISABLE_COPY(AdministrationDialog)
};

#endif // ADMINISTRATIONDIALOG_H
