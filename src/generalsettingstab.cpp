#include "src/generalsettingstab.h"

#include <QVariantMap>
#include <QString>
#include <QCheckBox>
#include <QFormLayout>
#include <QWidget>
#include <QLayout>

const QString GeneralSettingsTab::IdMultipleInstancesEnabled = "multiple_instances_enabled";

//

GeneralSettingsTab::GeneralSettingsTab()
{
    QFormLayout *flt = qobject_cast<QFormLayout*>( layout() );
      mCboxMultipleInstances = new QCheckBox(this);
      flt->insertRow(1, tr("Enable multiple instances:", "label text"), mCboxMultipleInstances);
}

//

void GeneralSettingsTab::setMultipleInstancesEnabled(bool enabled)
{
    mCboxMultipleInstances->setChecked(enabled);
}

//

QVariantMap GeneralSettingsTab::extraValueMap() const
{
    QVariantMap m;
    m.insert( IdMultipleInstancesEnabled, mCboxMultipleInstances->isChecked() );
    return m;
}
