#ifndef TEXSAMPLESETTINGSTAB_H
#define TEXSAMPLESETTINGSTAB_H

class BPasswordWidget;

class QString;
class QIcon;
class QCheckBox;
class QLineEdit;
class QByteArray;
class QComboBox;
class QHBoxLayout;
class QToolButton;

#include <BAbstractSettingsTab>

#include <QObject>
#include <QVariantMap>
#include <QStringList>

/*============================================================================
================================ TexsampleSettingsTab ========================
============================================================================*/

class TexsampleSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit TexsampleSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool hasAdvancedMode() const;
    bool isInAdvancedMode() const;
    void setAdvancedMode(bool enabled);
    bool restoreDefault();
    bool saveSettings();
private:
    static QString autoSelectText();
private:
    QStringList updateHostHistory(const QStringList &history = QStringList());
private slots:
    void clearCache();
    void removeCurrentHostFromHistory();
    void cmboxHostCurrentIndexChanged(int index);
private:
    bool am;
    QHBoxLayout *mhltHost;
      QComboBox *mcmboxHost;
      QToolButton *mtbtnRemoveFromHistory;
    QLineEdit *mledtLogin;
    BPasswordWidget *mpwdwgt;
    QCheckBox *mcboxAutoconnection;
    QCheckBox *mcboxCaching;
private:
    Q_DISABLE_COPY(TexsampleSettingsTab)
};

#endif // TEXSAMPLESETTINGSTAB_H
