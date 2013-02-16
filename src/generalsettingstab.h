#ifndef GENERALSETTINGSTAB_H
#define GENERALSETTINGSTAB_H

class BLocaleComboBox;

class QString;
class QIcon;
class QCheckBox;

#include <BAbstractSettingsTab>

#include <QObject>
#include <QVariantMap>

/*============================================================================
================================ GeneralSettingsTab ==========================
============================================================================*/

class GeneralSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit GeneralSettingsTab();
    ~GeneralSettingsTab();
public:
    static bool getMultipleWindowsEnabled();
    static void setMultipleWindowsEnabled(bool enabled);
public:
    QString title() const;
    QIcon icon() const;
    bool restoreDefault();
    bool saveSettings();
private:
    BLocaleComboBox *mlcmbox;
    QCheckBox *mcboxMultipleWindows;
private:
    Q_DISABLE_COPY(GeneralSettingsTab)
};

#endif // GENERALSETTINGSTAB_H
