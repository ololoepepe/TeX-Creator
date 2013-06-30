#ifndef RECOVERYDIALOG_H
#define RECOVERYDIALOG_H

class BPasswordWidget;

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
    QPushButton *mbtnGet;
    QLineEdit *mledtCode;
    BPasswordWidget *mpwdwgt1;
    BPasswordWidget *mpwdwgt2;
    QPushButton *mbtnRecover;
};

#endif // RECOVERYDIALOG_H
