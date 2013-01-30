#ifndef TEXSAMPLESETTINGSTAB_H
#define TEXSAMPLESETTINGSTAB_H

class BPasswordWidget;

class QString;
class QIcon;
class QCheckBox;
class QLineEdit;
class QByteArray;

#include <BAbstractSettingsTab>

#include <QObject>
#include <QVariantMap>

/*============================================================================
================================ TexsampleSettingsTab ========================
============================================================================*/

class TexsampleSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit TexsampleSettingsTab();
public:
    static bool getAutoconnection();
    static QString getHost();
    static QString getLogin();
    static QByteArray getPasswordState();
    static QByteArray getPassword();
    static void setAutoconnection(bool enabled);
    static void setHost(const QString &host);
    static void setLogin(const QString &login);
    static void setPasswordSate(const QByteArray &state);
public:
    QString title() const;
    QIcon icon() const;
    bool hasAdvancedMode() const;
    void setAdvancedMode(bool enabled);
    bool restoreDefault();
    bool saveSettings();
private:
    QCheckBox *mcboxAutoconnection;
    QLineEdit *mledtHost;
    QLineEdit *mledtLogin;
    BPasswordWidget *mpwdwgt;
private:
    Q_DISABLE_COPY(TexsampleSettingsTab)
};

#endif // TEXSAMPLESETTINGSTAB_H
