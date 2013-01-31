#ifndef ACCOUNTSETTINGSTAB_H
#define ACCOUNTSETTINGSTAB_H

class BPasswordWidget;

class QString;
class QIcon;
class QLineEdit;

#include <BAbstractSettingsTab>

#include <QByteArray>

/*============================================================================
================================ AccountSettingsTab ==========================
============================================================================*/

class AccountSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit AccountSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool saveSettings();
private:
    QByteArray mpwd;
    QString mrealName;
    QLineEdit *mledtName;
    BPasswordWidget *mpwdwgt;
};

#endif // ACCOUNTSETTINGSTAB_H
