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
    static bool hasTexsample();
    static bool getAutoconnection();
    static QString getHost();
    static QString getLogin();
    static QByteArray getPasswordState();
    static QByteArray getPassword();
    static bool getCachingEnabled();
    static void setAutoconnection(bool enabled);
    static void setHost(const QString &host);
    static void setLogin(const QString &login);
    static void setPasswordSate(const QByteArray &state);
    static void setCachingEnabled(bool enabled);
public:
    QString title() const;
    QIcon icon() const;
    bool hasAdvancedMode() const;
    void setAdvancedMode(bool enabled);
    bool restoreDefault();
    bool saveSettings();
private slots:
    void clearCache();
private:
    QCheckBox *mcboxAutoconnection;
    QLineEdit *mledtHost;
    QLineEdit *mledtLogin;
    BPasswordWidget *mpwdwgt;
    QCheckBox *mcboxCaching;
private:
    Q_DISABLE_COPY(TexsampleSettingsTab)
};

#endif // TEXSAMPLESETTINGSTAB_H
