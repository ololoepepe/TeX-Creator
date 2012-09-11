#ifndef GENERALSETTINGSTAB_H
#define GENERALSETTINGSTAB_H

class QCheckBox;

#include <bgeneralsettingstab.h>

#include <QVariantMap>
#include <QString>

class GeneralSettingsTab : public BGeneralSettingsTab
{
    Q_OBJECT
public:
    static const QString IdMultipleInstancesEnabled;
    //
    GeneralSettingsTab();
    //
    void setMultipleInstancesEnabled(bool enabled);
protected:
    QVariantMap extraValueMap() const;
private:
    //_m_flt
      QCheckBox *mCboxMultipleInstances;
};

#endif // GENERALSETTINGSTAB_H
