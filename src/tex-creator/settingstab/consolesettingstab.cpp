/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
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

#include <BAbstractSettingsTab>

#include <QCheckBox>
#include <QComboBox>
#include <QIcon>
#include <QLineEdit>
#include <QString>

/*============================================================================
================================ ConsoleSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

ConsoleSettingsTab::ConsoleSettingsTab()
{
    /*QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Compiler", "gbox title"), this);
        QFormLayout *flt = new QFormLayout;
          mcmboxCompiler = new QComboBox(gbox);
            foreach (TCompilerParameters::Compiler c, TCompilerParameters::allCompilers())
                mcmboxCompiler->addItem(TCompilerParameters::compilerToString(c), c);
            mcmboxCompiler->setCurrentIndex(mcmboxCompiler->findData(Global::compiler()));
          flt->addRow(tr("Compiler:", "label text"), mcmboxCompiler);
          mledtOptions = new QLineEdit(gbox);
            mledtOptions->setText(Global::compilerOptionsString());
            mledtOptions->setToolTip(tr("Separate options with spaces", "ledt toolTip"));
          flt->addRow(tr("Compiler options:", "lbl text"), mledtOptions);
          mledtCommands = new QLineEdit(this);
            mledtCommands->setText(Global::compilerCommandsString());
            mledtCommands->setToolTip(tr("Use quotes (\") to wrap commands that contain spaces", "ledt toolTip"));
          flt->addRow(tr("Compiler commands:", "lbl text"), mledtCommands);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Remote compiler", "gbox title"), this);
        flt = new QFormLayout;
          mcboxRemoteCompiler = new QCheckBox(gbox);
            mcboxRemoteCompiler->setChecked(Global::useRemoteCompiler());
            mcboxRemoteCompiler->setToolTip(tr("If checked and if you are connected to the TeXSample service, "
                                               "remote compilation system will be used", "cbox toolTip"));
          flt->addRow(tr("Remote compilation:", "lbl text"), mcboxRemoteCompiler);
          mcboxFallbackToLocalCompiler = new QCheckBox(gbox);
            mcboxFallbackToLocalCompiler->setEnabled(mcboxRemoteCompiler->isChecked());
            mcboxFallbackToLocalCompiler->setToolTip(tr("If checked and if the remote compiler is not available, "
                                                        "the local one will be used", "cbox toolTip"));
            mcboxFallbackToLocalCompiler->setChecked(Global::hasFallbackToLocalCompiler()
                                                     && Global::fallbackToLocalCompiler());
            connect(mcboxRemoteCompiler, SIGNAL(clicked(bool)), mcboxFallbackToLocalCompiler, SLOT(setEnabled(bool)));
          flt->addRow(tr("Fallback to remote compiler:", "lbl text"), mcboxFallbackToLocalCompiler);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Tools", "gbox title"), this);
        flt = new QFormLayout;
          mcboxMakeindex = new QCheckBox(gbox);
            mcboxMakeindex->setToolTip(tr("Run makeindex after compilation", "cbox toolTip"));
            mcboxMakeindex->setWhatsThis(tr("Check this option to run the makeindex utility after compliation",
                                            "cbox whatsThis"));
            mcboxMakeindex->setChecked(Global::makeindexEnabled());
          flt->addRow(tr("Makeindex:", "lbl text"), mcboxMakeindex);
          mcboxDvips = new QCheckBox(gbox);
            mcboxDvips->setToolTip(tr("Run dvips after compilation", "cbox toolTip"));
            mcboxDvips->setWhatsThis(tr("Check this option to run the dvips utility after compilation",
                                        "cbox whatsThis"));
            mcboxDvips->setChecked(Global::dvipsEnabled());
          flt->addRow(tr("Dvips:", "lbl text"), mcboxDvips);
          mcboxAlwaysLatin = new QCheckBox(gbox);
            mcboxAlwaysLatin->setToolTip(tr("If checked, Latin letters will always be entered, "
                                            "ignoring keyboard layout", "cbox toolTip"));
            mcboxAlwaysLatin->setWhatsThis(tr("Check this option if you always enter latin only characters "
                                              "into console, so you will not have to switch keyboard layout",
                                              "cbox whatsThis"));
            mcboxAlwaysLatin->setChecked(Global::alwaysLatinEnabled());
          flt->addRow(tr("Always Latin:", "lbl text"), mcboxAlwaysLatin);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);*/
}

/*============================== Public methods ============================*/

QString ConsoleSettingsTab::id() const
{
    return "console";
}

QString ConsoleSettingsTab::title() const
{
    return tr("Console", "title");
}

QIcon ConsoleSettingsTab::icon() const
{
    return Application::icon("utilities_terminal");
}

bool ConsoleSettingsTab::hasDefault() const
{
    return true;
}

bool ConsoleSettingsTab::restoreDefault()
{
    mcmboxCompiler->setCurrentIndex(mcmboxCompiler->findText("pdflatex"));
    return true;
}

bool ConsoleSettingsTab::saveSettings()
{
    /*TCompilerParameters param;
    param.setCompiler(mcmboxCompiler->itemData(mcmboxCompiler->currentIndex()).toInt());
    param.setOptions(mledtOptions->text());
    param.setCommands(mledtCommands->text());
    param.setMakeindexEnabled(mcboxMakeindex->isChecked());
    param.setDvipsEnabled(mcboxDvips->isChecked());
    Global::setCompilerParameters(param);
    Global::setUseRemoteCompiler(mcboxRemoteCompiler->isChecked());
    Global::setAlwaysLatinEnabled(mcboxAlwaysLatin->isChecked());
    if (Global::hasFallbackToLocalCompiler() || mcboxFallbackToLocalCompiler->isChecked())
        Global::setFallbackToLocalCompiler(mcboxFallbackToLocalCompiler->isChecked());*/
    return true;
}
