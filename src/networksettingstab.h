#ifndef NETWORKSETTINGSTAB_H
#define NETWORKSETTINGSTAB_H

class BLoginWidget;

class QString;
class QIcon;
class QButtonGroup;

#include <BAbstractSettingsTab>

#include <QObject>

/*============================================================================
================================ NetworkSettingsTab ==========================
============================================================================*/

class NetworkSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit NetworkSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool hasDefault() const;
    bool restoreDefault();
    bool saveSettings();
private slots:
    void btnClicked(int index);
private:
    QButtonGroup *btngr;
    BLoginWidget *lwgt;
private:
    Q_DISABLE_COPY(NetworkSettingsTab)
};

#endif // NETWORKSETTINGSTAB_H
