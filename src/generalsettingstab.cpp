#include "generalsettingstab.h"
#include "application.h"

#include <BAbstractSettingsTab>
#include <BLocaleComboBox>

#include <QString>
#include <QVariantMap>
#include <QIcon>
#include <QFormLayout>
#include <QCheckBox>
#include <QLocale>
#include <QSettings>
#include <QMessageBox>

#include <QDebug>

/*============================================================================
================================ GeneralSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

GeneralSettingsTab::GeneralSettingsTab() :
    BAbstractSettingsTab()
{
    QFormLayout *flt = new QFormLayout(this);
    mlcmbox = new BLocaleComboBox(true, this);
      mlcmbox->setCurrentLocale( Application::locale() );
    flt->addRow("Language:", mlcmbox); //Yep, this label should never be translated
    mcboxMultipleWindows = new QCheckBox(this);
      mcboxMultipleWindows->setChecked( getMultipleWindowsEnabled() );
    flt->addRow(tr("Enable multiple windows:", "lbl text"), mcboxMultipleWindows);
}

GeneralSettingsTab::~GeneralSettingsTab()
{
    //
}

/*============================== Static public methods =====================*/

bool GeneralSettingsTab::getMultipleWindowsEnabled()
{
    return bSettings->value("Core/multiple_windows_enabled", false).toBool();
}

void GeneralSettingsTab::setMultipleWindowsEnabled(bool enabled)
{
    bSettings->setValue("Core/multiple_windows_enabled", enabled);
}

/*============================== Public methods ============================*/

QString GeneralSettingsTab::title() const
{
    return tr("General", "title");
}

QIcon GeneralSettingsTab::icon() const
{
    return Application::icon("configure");
}

bool GeneralSettingsTab::restoreDefault()
{
    mcboxMultipleWindows->setChecked(false);
    return true;
}

bool GeneralSettingsTab::saveSettings()
{
    if ( getMultipleWindowsEnabled() && !mcboxMultipleWindows->isChecked() && !Application::mergeWindows() )
    {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Failed to change settings", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Information);
        msg.setText( tr("Can't disable multiple windows: documents merging failed", "msgbox text") );
        msg.setInformativeText( tr("Please, close all duplicate documents and try again", "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return false;
    }
    Application::setLocale( mlcmbox->currentLocale() );
    setMultipleWindowsEnabled( mcboxMultipleWindows->isChecked() );
    return true;
}
