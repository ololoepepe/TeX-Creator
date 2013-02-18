#include "consolesettingstab.h"
#include "application.h"

#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QStringList>
#include <QString>
#include <QIcon>
#include <QVariant>
#include <QSettings>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>

/*============================================================================
================================ ConsoleSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

ConsoleSettingsTab::ConsoleSettingsTab()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Compiler", "gbox title"), this);
        QFormLayout *flt = new QFormLayout;
          mcmboxName = new QComboBox(gbox);
            QStringList sl;
            sl << "pdflatex";
            sl << "pdftex";
            sl << "latex";
            sl << "tex";
            mcmboxName->addItems(sl);
            mcmboxName->setCurrentIndex( mcmboxName->findText( getCompilerName() ) );
          flt->addRow(tr("Compiler:", "label text"), mcmboxName);
          mledtOptions = new QLineEdit(gbox);
            mledtOptions->setText( getCompilerOptionsString() );
            mledtOptions->setToolTip( tr("Separate options with spaces", "ledt toolTip") );
          flt->addRow(tr("Compiler options:", "lbl text"), mledtOptions);
          mledtCommands = new QLineEdit(this);
            mledtCommands->setText( getCompilerCommandsString() );
            mledtCommands->setToolTip( tr("Use quotes (\") to wrap commands that contain spaces", "ledt toolTip") );
          flt->addRow(tr("Compiler commands:", "lbl text"), mledtCommands);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Remote compiler", "gbox title"), this);
        flt = new QFormLayout;
          mcboxRemoteCompiler = new QCheckBox(gbox);
            mcboxRemoteCompiler->setChecked(getUseRemoteCompiler());
            mcboxRemoteCompiler->setToolTip(tr("If checked and if you are connected to the TeXSample service, "
                                               "remote compilation system will be used", "cbox toolTip"));
          flt->addRow(tr("Remote compilation:", "lbl text"), mcboxRemoteCompiler);
          mcboxFallbackToLocalCompiler = new QCheckBox(gbox);
            mcboxFallbackToLocalCompiler->setEnabled(mcboxRemoteCompiler->isChecked());
            mcboxFallbackToLocalCompiler->setToolTip(tr("If checked and if the remote compiler is not available, "
                                                        "the local one will be used", "cbox toolTip"));
            mcboxFallbackToLocalCompiler->setChecked(hasFallbackToLocalCompiler() && getFallbackToLocalCompiler());
            connect(mcboxRemoteCompiler, SIGNAL(clicked(bool)), mcboxFallbackToLocalCompiler, SLOT(setEnabled(bool)));
          flt->addRow(tr("Fallback to remote compiler:", "lbl text"), mcboxFallbackToLocalCompiler);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Tools", "gbox title"), this);
        flt = new QFormLayout;
          mcboxMakeindex = new QCheckBox(gbox);
            mcboxMakeindex->setToolTip( tr("Run makeindex after compilation", "cbox toolTip") );
            mcboxMakeindex->setWhatsThis( tr("Check this option to run the makeindex utility after compliation",
                                             "cbox whatsThis") );
            mcboxMakeindex->setChecked( getMakeindexEnabled() );
          flt->addRow(tr("Makeindex:", "lbl text"), mcboxMakeindex);
          mcboxDvips = new QCheckBox(gbox);
            mcboxDvips->setToolTip( tr("Run dvips after compilation", "cbox toolTip") );
            mcboxDvips->setWhatsThis( tr("Check this option to run the dvips utility after compilation", "cbox whatsThis") );
            mcboxDvips->setChecked( getDvipsEnabled() );
          flt->addRow(tr("Dvips:", "lbl text"), mcboxDvips);
          mcboxAlwaysLatin = new QCheckBox(gbox);
            mcboxAlwaysLatin->setToolTip(tr("If checked, Latin letters will always be entered, ignoring keyboard layout",
                                            "cbox toolTip"));
            mcboxAlwaysLatin->setWhatsThis(tr("Check this option if you always enter latin only characters into console, "
                                              "so you will not have to switch keyboard layout", "cbox whatsThis"));
            mcboxAlwaysLatin->setChecked(getAlwaysLatinEnabled());
          flt->addRow(tr("Always Latin:", "lbl text"), mcboxAlwaysLatin);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
    //
    setRowVisible(mledtOptions, false);
    setRowVisible(mledtCommands, false);
}

/*============================== Static public methods =====================*/

bool ConsoleSettingsTab::hasFallbackToLocalCompiler()
{
    return bSettings->contains("Console/fallback_to_local_compiler");
}

QString ConsoleSettingsTab::getCompilerName()
{
    return bSettings->value("Console/compiler_name", "pdflatex").toString();
}

QStringList ConsoleSettingsTab::getCompilerOptions()
{
    return bSettings->value("Console/compiler_options").toStringList();
}

QString ConsoleSettingsTab::getCompilerOptionsString()
{
    return joinArguments( getCompilerOptions() );
}

QStringList ConsoleSettingsTab::getCompilerCommands()
{
    return bSettings->value("Console/compiler_commands").toStringList();
}

QString ConsoleSettingsTab::getCompilerCommandsString()
{
    return joinArguments( getCompilerCommands() );
}

bool ConsoleSettingsTab::getMakeindexEnabled()
{
    return bSettings->value("Console/makeindex_enabled", false).toBool();
}

bool ConsoleSettingsTab::getDvipsEnabled()
{
    return bSettings->value("Console/dvips_enabled", false).toBool();
}

bool ConsoleSettingsTab::getUseRemoteCompiler()
{
    return bSettings->value("Console/use_remote_compiler").toBool();
}

bool ConsoleSettingsTab::getFallbackToLocalCompiler()
{
    return bSettings->value("Console/fallback_to_local_compiler").toBool();
}

bool ConsoleSettingsTab::getAlwaysLatinEnabled()
{
    return bSettings->value("Console/always_latin_enabled", false).toBool();
}

void ConsoleSettingsTab::setCompilerName(const QString &command)
{
    if ( command.isEmpty() )
        return;
    bSettings->setValue("Console/compiler_name", command);
}

void ConsoleSettingsTab::setCompilerOptions(const QStringList &list)
{
    bSettings->setValue("Console/compiler_options", list);
}

void ConsoleSettingsTab::setCompilerOptions(const QString &string)
{
    setCompilerOptions( splitArguments(string) );
}

void ConsoleSettingsTab::setCompilerCommands(const QStringList &list)
{
    bSettings->setValue("Console/compiler_commands", list);
}

void ConsoleSettingsTab::setCompilerCommands(const QString &string)
{
    setCompilerCommands( splitArguments(string) );
}

void ConsoleSettingsTab::setMakeindexEnabled(bool enabled)
{
    bSettings->setValue("Console/makeindex_enabled", enabled);
}

void ConsoleSettingsTab::setDvipsEnabled(bool enabled)
{
    bSettings->setValue("Console/dvips_enabled", enabled);
}

void ConsoleSettingsTab::setUseRemoteCompiler(bool b)
{
    bSettings->setValue("Console/use_remote_compiler", b);
}

void ConsoleSettingsTab::setFallbackToLocalCompiler(bool b)
{
    bSettings->setValue("Console/fallback_to_local_compiler", b);
}

void ConsoleSettingsTab::setAlwaysLatinEnabled(bool enabled)
{
    bSettings->setValue("Console/always_latin_enabled", enabled);
}

/*============================== Public methods ============================*/

QString ConsoleSettingsTab::title() const
{
    return tr("Console", "title");
}

QIcon ConsoleSettingsTab::icon() const
{
    return Application::icon("utilities_terminal");
}

bool ConsoleSettingsTab::hasAdvancedMode() const
{
    return true;
}

void ConsoleSettingsTab::setAdvancedMode(bool enabled)
{
    setRowVisible(mledtOptions, enabled);
    setRowVisible(mledtCommands, enabled);
}

bool ConsoleSettingsTab::restoreDefault()
{
    mcmboxName->setCurrentIndex( mcmboxName->findText("pdflatex") );
    return true;
}

bool ConsoleSettingsTab::saveSettings()
{
    setCompilerName(mcmboxName->currentText());
    setCompilerOptions(mledtOptions->text());
    setCompilerCommands(mledtCommands->text());
    setMakeindexEnabled(mcboxMakeindex->isChecked());
    setDvipsEnabled(mcboxDvips->isChecked());
    setUseRemoteCompiler(mcboxRemoteCompiler->isChecked());
    if (hasFallbackToLocalCompiler() || mcboxFallbackToLocalCompiler->isChecked())
        setFallbackToLocalCompiler(mcboxFallbackToLocalCompiler->isChecked());
    setAlwaysLatinEnabled(mcboxAlwaysLatin->isChecked());
    return true;
}

/*============================== Static private methods ====================*/

QStringList ConsoleSettingsTab::splitArguments(const QString &string)
{
    QStringList list = string.split(' ', QString::SkipEmptyParts);
    if ( list.isEmpty() )
        return list;
    int x = -1;
    foreach (int i, bRangeR(list.size() - 1, 0))
    {
        QString &arg = list[i];
        if (x < 0 && arg.at(arg.length() - 1) == '\"')
            x = i;
        if (x >= 0 && arg.at(0) == '\"')
        {
            if (x != i)
                foreach (int j, bRange(x, i + 1))
                    arg.append( " " + list.takeAt(j) );
            x = -1;
        }
    }
    return list;
}

QString ConsoleSettingsTab::joinArguments(const QStringList &list)
{
    QStringList nlist = list;
    if ( !nlist.isEmpty() )
    {
        foreach (int i, bRangeD(0, list.size() - 1))
        {
            QString &arg = nlist[i];
            if ( arg.contains(' ') && (arg.at(0) != '\"' || arg.at(arg.length() - 1) != '\"') )
                arg.prepend('\"').append('\"');
        }
    }
    return nlist.join(' ');
}
