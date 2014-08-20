/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
**
** This file is part of TeX Creator.
**
** TeX Creator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** TeX Creator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with TeX Creator.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "consolesettingstab.h"

#include "application.h"
#include "settings.h"

#include <TTexCompiler>

#include <BAbstractSettingsTab>
#include <BTextTools>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QIcon>
#include <QLineEdit>
#include <QString>
#include <QVBoxLayout>

/*============================================================================
================================ ConsoleSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

ConsoleSettingsTab::ConsoleSettingsTab()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Compiler", "gbox title"));
        QFormLayout *flt = new QFormLayout;
          cmboxCompiler = new QComboBox;
            foreach (const TTexCompiler &compiler, TTexCompiler::allCompilers())
                cmboxCompiler->addItem(compiler.toString(), int(compiler));
            cmboxCompiler->setCurrentIndex(cmboxCompiler->findData(int(Settings::Compiler::compiler())));
          flt->addRow(tr("Compiler:", "label text"), cmboxCompiler);
          ledtOptions = new QLineEdit;
            ledtOptions->setText(BTextTools::mergeArguments(Settings::Compiler::compilerOptions()));
            ledtOptions->setToolTip(tr("Separate options with spaces", "ledt toolTip"));
          flt->addRow(tr("Compiler options:", "lbl text"), ledtOptions);
          ledtCommands = new QLineEdit;
            ledtCommands->setText(BTextTools::mergeArguments(Settings::Compiler::compilerCommands()));
            ledtCommands->setToolTip(tr("Use quotes (\") to wrap commands that contain spaces", "ledt toolTip"));
          flt->addRow(tr("Compiler commands:", "lbl text"), ledtCommands);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Remote compiler", "gbox title"));
        flt = new QFormLayout;
          cboxRemoteCompiler = new QCheckBox;
            cboxRemoteCompiler->setChecked(Settings::Console::useRemoteCompiler());
            cboxRemoteCompiler->setToolTip(tr("If checked and if you are connected to the TeXSample service, "
                                              "remote compilation system will be used", "cbox toolTip"));
          flt->addRow(tr("Remote compilation:", "lbl text"), cboxRemoteCompiler);
          cboxFallbackToLocalCompiler = new QCheckBox;
            cboxFallbackToLocalCompiler->setEnabled(cboxRemoteCompiler->isChecked());
            cboxFallbackToLocalCompiler->setToolTip(tr("If checked and if the remote compiler is not available, "
                                                       "the local one will be used", "cbox toolTip"));
            cboxFallbackToLocalCompiler->setChecked(Settings::Console::hasFallbackToLocalCompiler()
                                                    && Settings::Console::fallbackToLocalCompiler());
            connect(cboxRemoteCompiler, SIGNAL(clicked(bool)), cboxFallbackToLocalCompiler, SLOT(setEnabled(bool)));
          flt->addRow(tr("Fallback to remote compiler:", "lbl text"), cboxFallbackToLocalCompiler);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Tools", "gbox title"));
        flt = new QFormLayout;
          cboxMakeindex = new QCheckBox;
            cboxMakeindex->setToolTip(tr("Run makeindex after compilation", "cbox toolTip"));
            cboxMakeindex->setWhatsThis(tr("Check this option to run the makeindex utility after compliation",
                                           "cbox whatsThis"));
            cboxMakeindex->setChecked(Settings::Compiler::makeindexEnabled());
          flt->addRow(tr("MakeIndex:", "lbl text"), cboxMakeindex);
          cboxDvips = new QCheckBox;
            cboxDvips->setToolTip(tr("Run dvips after compilation", "cbox toolTip"));
            cboxDvips->setWhatsThis(tr("Check this option to run the dvips utility after compilation",
                                       "cbox whatsThis"));
            cboxDvips->setChecked(Settings::Compiler::dvipsEnabled());
          flt->addRow(tr("dvips:", "lbl text"), cboxDvips);
          cboxAlwaysLatin = new QCheckBox;
            cboxAlwaysLatin->setToolTip(tr("If checked, Latin letters will always be entered, "
                                           "ignoring keyboard layout", "cbox toolTip"));
            cboxAlwaysLatin->setWhatsThis(tr("Check this option if you always enter latin only characters "
                                             "into console, so you will not have to switch keyboard layout",
                                             "cbox whatsThis"));
            cboxAlwaysLatin->setChecked(Settings::Console::alwaysLatinEnabled());
          flt->addRow(tr("Always Latin:", "lbl text"), cboxAlwaysLatin);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
}

/*============================== Public methods ============================*/

bool ConsoleSettingsTab::hasDefault() const
{
    return true;
}

QIcon ConsoleSettingsTab::icon() const
{
    return Application::icon("utilities_terminal");
}

QString ConsoleSettingsTab::id() const
{
    return "console";
}

bool ConsoleSettingsTab::restoreDefault()
{
    cmboxCompiler->setCurrentIndex(cmboxCompiler->findText("pdflatex"));
    return true;
}

bool ConsoleSettingsTab::saveSettings()
{
    Settings::Compiler::setCompiler(cmboxCompiler->itemData(cmboxCompiler->currentIndex()).toInt());
    Settings::Compiler::setCompilerCommands(BTextTools::splitCommand(ledtCommands->text()));
    Settings::Compiler::setCompilerOptions(BTextTools::splitCommand(ledtOptions->text()));
    Settings::Compiler::setMakeindexEnabled(cboxMakeindex->isChecked());
    Settings::Compiler::setDvipsEnabled(cboxDvips->isChecked());
    Settings::Console::setUseRemoteCompiler(cboxRemoteCompiler->isChecked());
    Settings::Console::setAlwaysLatinEnabled(cboxAlwaysLatin->isChecked());
    if (Settings::Console::hasFallbackToLocalCompiler() || cboxFallbackToLocalCompiler->isChecked())
        Settings::Console::setFallbackToLocalCompiler(cboxFallbackToLocalCompiler->isChecked());
    bApp->updateConsoleSettings();
    return true;
}

QString ConsoleSettingsTab::title() const
{
    return tr("Console", "title");
}
