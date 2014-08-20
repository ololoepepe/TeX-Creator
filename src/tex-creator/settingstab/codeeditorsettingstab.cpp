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

#include "codeeditorsettingstab.h"

#include "application.h"
#include "settings.h"

#include <BAbstractSettingsTab>
#include <BCodeEditor>
#include <BeQt>
#include <BGuiTools>
#include <BTextCodecComboBox>

#include <QCheckBox>
#include <QComboBox>
#include <QFont>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QIcon>
#include <QSpinBox>
#include <QString>
#include <QTextCodec>
#include <QVBoxLayout>

#include <climits>

/*============================================================================
================================ CodeEditorSettingsTab =======================
============================================================================*/

/*============================== Public constructors =======================*/

CodeEditorSettingsTab::CodeEditorSettingsTab()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Document type", "gbox title"), this);
        QFormLayout *flt = new QFormLayout;
          mcboxSimple = new QCheckBox;
            mcboxSimple->setChecked(Settings::CodeEditor::documentType() == BCodeEditor::SimpleDocument);
          flt->addRow(tr("Classic documents:", "lbl text"), mcboxSimple);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Font", "gbox title"), this);
        flt = new QFormLayout;
          mfntcmbox = new QFontComboBox(gbox);
            mfntcmbox->setFontFilters(QFontComboBox::MonospacedFonts);
            mfntcmbox->setCurrentFont(Settings::CodeEditor::editFont());
          flt->addRow(tr("Font:", "lbl text"), mfntcmbox);
          msboxFontPointSize = new QSpinBox(gbox);
            msboxFontPointSize->setMinimum(1);
            msboxFontPointSize->setMaximum(100);
            msboxFontPointSize->setValue(Settings::CodeEditor::editFontPointSize());
          flt->addRow(tr("Font size:", "lbl text"), msboxFontPointSize);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Tabulation and lines", "gbox title"), this);
        flt = new QFormLayout;
          msboxLineLength = new QSpinBox(gbox);
            msboxLineLength->setMinimum(10);
            msboxLineLength->setMaximum(1000);
            msboxLineLength->setSingleStep(10);
            msboxLineLength->setValue(Settings::CodeEditor::editLineLength());
            msboxLineLength->setEnabled(!mcboxSimple->isChecked());
            connect(mcboxSimple, SIGNAL(toggled(bool)), msboxLineLength, SLOT(setDisabled(bool)));
          flt->addRow(tr("Line length:", "lbl text"), msboxLineLength);
          mcmboxTabWidth = new QComboBox(gbox);
            mcmboxTabWidth->addItem(QString::number(BeQt::TabWidth2), int(BeQt::TabWidth2));
            mcmboxTabWidth->addItem(QString::number(BeQt::TabWidth4), int(BeQt::TabWidth4));
            mcmboxTabWidth->addItem(QString::number(BeQt::TabWidth8), int(BeQt::TabWidth8));
            mcmboxTabWidth->setCurrentIndex(mcmboxTabWidth->findData(int(Settings::CodeEditor::editTabWidth())));
          flt->addRow(tr("Tab width:", "lbl text"), mcmboxTabWidth);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Files", "gbox title"), this);
        flt = new QFormLayout;
          mcboxAutoCodecDetection = new QCheckBox;
            mcboxAutoCodecDetection->setChecked(Settings::CodeEditor::autoCodecDetectionEnabled());
          flt->addRow(tr("Enable automatic encoding detection:", "lbl text"), mcboxAutoCodecDetection);
          mcmboxEncoding = new BTextCodecComboBox;
            mcmboxEncoding->selectCodec(Settings::CodeEditor::defaultCodec());
          flt->addRow(tr("Default encoding:", "lbl text"), mcmboxEncoding);
          msboxMaxFileSize = new QSpinBox;
            msboxMaxFileSize->setMinimum(0);
            msboxMaxFileSize->setSingleStep(100);
            msboxMaxFileSize->setMaximum(INT_MAX / BeQt::Kilobyte);
            msboxMaxFileSize->setValue(Settings::CodeEditor::maximumFileSize() / BeQt::Kilobyte);
            msboxMaxFileSize->setToolTip(tr("0 means no limit", "sbox toolTip"));
          flt->addRow(tr("Maximum file size (KB):", "lbl text"), msboxMaxFileSize);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
}

/*============================== Public methods ============================*/

QString CodeEditorSettingsTab::id() const
{
    return "code_editor";
}

QString CodeEditorSettingsTab::title() const
{
    return tr("Text editor", "title");
}

QIcon CodeEditorSettingsTab::icon() const
{
    return Application::icon("edit");
}

bool CodeEditorSettingsTab::hasDefault() const
{
    return true;
}

bool CodeEditorSettingsTab::restoreDefault()
{
    QFont fnt = BGuiTools::createMonospaceFont();
    mcboxSimple->setChecked(false);
    mfntcmbox->setCurrentFont(fnt);
    msboxFontPointSize->setValue( fnt.pointSize() );
    mcmboxEncoding->selectCodec(QTextCodec::codecForLocale());
    msboxLineLength->setValue(120);
    mcmboxTabWidth->setCurrentIndex(mcmboxTabWidth->findData(int(BeQt::TabWidth4)));
    return true;
}

bool CodeEditorSettingsTab::saveSettings()
{
    Settings::CodeEditor::setAutoCodecDetectionEnabled(mcboxAutoCodecDetection->isChecked());
    Settings::CodeEditor::setDocumentType(mcboxSimple->isChecked() ? BCodeEditor::SimpleDocument :
                                                                     BCodeEditor::StandardDocument);
    Settings::CodeEditor::setEditFontFamily(mfntcmbox->currentFont().family());
    Settings::CodeEditor::setEditFontPointSize(msboxFontPointSize->value());
    Settings::CodeEditor::setDefaultCodec(mcmboxEncoding->selectedCodec());
    Settings::CodeEditor::setEditLineLength(msboxLineLength->value());
    QVariant tw = mcmboxTabWidth->itemData(mcmboxTabWidth->currentIndex());
    Settings::CodeEditor::setEditTabWidth(enum_cast<BeQt::TabWidth>(tw, BeQt::allTabWidths(), BeQt::TabWidth4));
    Settings::CodeEditor::setMaximumFileSize(msboxMaxFileSize->value() * BeQt::Kilobyte);
    bApp->updateCodeEditorSettings();
    return true;
}
