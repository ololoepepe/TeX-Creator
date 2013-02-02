#ifndef ACCOUNTSETTINGSTAB_H
#define ACCOUNTSETTINGSTAB_H

class BPasswordWidget;

class QIcon;
class QLineEdit;
class QToolButton;

#include <BAbstractSettingsTab>

#include <QByteArray>
#include <QString>
#include <QImage>

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
private slots:
    void tbtntAvatarClicked();
private:
    QByteArray mpwd;
    QString mrealName;
    QImage mavatar;
    QString mavatarFormat;
    QLineEdit *mledtName;
    BPasswordWidget *mpwdwgt;
    QToolButton *mtbtnAvatar;
private:
    Q_DISABLE_COPY(AccountSettingsTab)
};

#endif // ACCOUNTSETTINGSTAB_H
