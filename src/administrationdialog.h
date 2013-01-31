#ifndef ADMINISTRATIONDIALOG_H
#define ADMINISTRATIONDIALOG_H

class QLineEdit;
class QComboBox;
class QPushButton;

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
private:
    QLineEdit *mledtLogin;
    QLineEdit *mledtPassword;
    QLineEdit *mledtRealName;
    QComboBox *mcmboxAccessLevel;
    QPushButton *mbtnAddUser;
};

#endif // ADMINISTRATIONDIALOG_H
