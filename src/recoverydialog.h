#ifndef RECOVERYDIALOG_H
#define RECOVERYDIALOG_H

class BPasswordWidget;
class BInputField;

class QLineEdit;
class QPushButton;
class QString;

#include <QDialog>

/*============================================================================
================================ RecoveryDialog ==============================
============================================================================*/

class RecoveryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RecoveryDialog(QWidget *parent = 0);
private slots:
    void checkInputs();
    void getCode();
    void recoverAccount();
private:
    QLineEdit *mledtEmail;
    BInputField *minputEmail;
    QPushButton *mbtnGet;
    QLineEdit *mledtCode;
    BInputField *minputCode;
    BPasswordWidget *mpwdwgt1;
    BInputField *minputPwd1;
    BPasswordWidget *mpwdwgt2;
    BInputField *minputPwd2;
    QPushButton *mbtnRecover;
};

#endif // RECOVERYDIALOG_H
