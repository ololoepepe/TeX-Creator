#include "codeeditorsettingstab.h"
#include "application.h"

#include <BAbstractSettingsTab>
#include <BCodeEditor>
#include <BCodeEdit>

#include <QObject>
#include <QVariant>
#include <QString>
#include <QIcon>
#include <QFormLayout>
#include <QFontComboBox>
#include <QSpinBox>
#include <QComboBox>
#include <QFont>
#include <QSettings>
#include <QStringList>
#include <QByteArray>

/*============================================================================
================================ CodeEditorSettingsTab =======================
============================================================================*/

/*============================== Public constructors =======================*/

CodeEditorSettingsTab::CodeEditorSettingsTab()
{
    QFormLayout *flt = new QFormLayout(this);
    mfntcmbox = new QFontComboBox(this);
      mfntcmbox->setFontFilters(QFontComboBox::MonospacedFonts);
      mfntcmbox->setCurrentFont( getEditFont() );
    flt->addRow(tr("Font:", "lbl text"), mfntcmbox);
    msboxFontPointSize = new QSpinBox(this);
      msboxFontPointSize->setMinimum(1);
      msboxFontPointSize->setMaximum(100);
      msboxFontPointSize->setValue( getEditFontPointSize() );
    flt->addRow(tr("Font size:", "lbl text"), msboxFontPointSize);
    mcmboxEncoding = new QComboBox(this);
      foreach ( QTextCodec *c, BCodeEditor::supportedCodecs() )
      {
          QString fcn = BCodeEditor::fullCodecName(c);
          QString cn = BCodeEditor::codecName(c);
          mcmboxEncoding->addItem(!fcn.isEmpty() ? fcn : cn, cn);
      }
      mcmboxEncoding->setCurrentIndex( mcmboxEncoding->findData( getDefaultCodecName() ) );
    flt->addRow(tr("Default encoding:", "lbl text"), mcmboxEncoding);
    msboxLineLength = new QSpinBox(this);
      msboxLineLength->setMinimum(10);
      msboxLineLength->setMaximum(1000);
      msboxLineLength->setSingleStep(10);
      msboxLineLength->setValue( getEditLineLength() );
    flt->addRow(tr("Line length:", "lbl text"), msboxLineLength);
    mcmboxTabWidth = new QComboBox(this);
      mcmboxTabWidth->addItem(QString::number(BCodeEdit::TabWidth2), BCodeEdit::TabWidth2);
      mcmboxTabWidth->addItem(QString::number(BCodeEdit::TabWidth4), BCodeEdit::TabWidth4);
      mcmboxTabWidth->addItem(QString::number(BCodeEdit::TabWidth8), BCodeEdit::TabWidth8);
      mcmboxTabWidth->setCurrentIndex( mcmboxTabWidth->findData( getEditTabWidth() ) );
    flt->addRow(tr("Tab width:", "lbl text"), mcmboxTabWidth);
    setRowVisible(msboxLineLength, false);
    setRowVisible(mcmboxTabWidth, false);
}

CodeEditorSettingsTab::~CodeEditorSettingsTab()
{
    //
}

/*============================== Static public methods =====================*/

QFont CodeEditorSettingsTab::getEditFont()
{
    QFont fnt = Application::createMonospaceFont();
    fnt.setFamily( bSettings->value( "CodeEditor/edit_font_family", fnt.family() ).toString() );
    fnt.setPointSize( bSettings->value( "CodeEditor/edit_font_point_size", fnt.pointSize() ).toInt() );
    return fnt;
}

QString CodeEditorSettingsTab::getEditFontFamily()
{
    return getEditFont().family();
}

int CodeEditorSettingsTab::getEditFontPointSize()
{
    return getEditFont().pointSize();
}

QTextCodec *CodeEditorSettingsTab::getDefaultCodec()
{
    return QTextCodec::codecForName( bSettings->value( "CodeEditor/default_codec",
                                                       QTextCodec::codecForLocale()->name() ).toByteArray() );
}

QString CodeEditorSettingsTab::getDefaultCodecName()
{
    return BCodeEditor::codecName( getDefaultCodec() );
}

int CodeEditorSettingsTab::getEditLineLength()
{
    return bSettings->value("CodeEditor/edit_line_length", 120).toInt();
}

BCodeEdit::TabWidth CodeEditorSettingsTab::getEditTabWidth()
{
    return static_cast<BCodeEdit::TabWidth>( bSettings->value("CodeEditor/edit_tab_width", BCodeEdit::TabWidth4).toInt() );
}

QStringList CodeEditorSettingsTab::getFileHistory()
{
    return bSettings->value("CodeEditor/file_history").toStringList();
}

QByteArray CodeEditorSettingsTab::getDocumentDriverState()
{
    return bSettings->value("CodeEditor/document_driver_state").toByteArray();
}

QByteArray CodeEditorSettingsTab::getSearchModuleState()
{
    return bSettings->value("CodeEditor/search_moudle_state").toByteArray();
}

void CodeEditorSettingsTab::setEditFont(const QFont &font)
{
    foreach ( BCodeEditor *edr, Application::codeEditors() )
        edr->setEditFont(font);
    setEditFontFamily( font.family() );
    setEditFontPointSize( font.pointSize() );
}

void CodeEditorSettingsTab::setEditFontFamily(const QString &family)
{
    if ( family.isEmpty() )
        return;
    foreach ( BCodeEditor *edr, Application::codeEditors() )
        edr->setEditFontFamily(family);
    bSettings->setValue("CodeEditor/edit_font_family", family);
}

void CodeEditorSettingsTab::setEditFontPointSize(int pointSize)
{
    if (pointSize < 1)
        return;
    foreach ( BCodeEditor *edr, Application::codeEditors() )
        edr->setEditFontPointSize(pointSize);
    bSettings->setValue("CodeEditor/edit_font_point_size", pointSize);
}

void CodeEditorSettingsTab::setDefaultCodec(QTextCodec *codec)
{
    if (!codec)
        return;
    foreach ( BCodeEditor *edr, Application::codeEditors() )
        edr->setDefaultCodec(codec);
    bSettings->setValue( "CodeEditor/default_codec", BCodeEditor::codecName(codec) );
}

void CodeEditorSettingsTab::setDefaultCodec(const QByteArray &codecName)
{
    setDefaultCodec( QTextCodec::codecForName(codecName) );
}

void CodeEditorSettingsTab::setDefaultCodec(const QString &codecName)
{
    setDefaultCodec( codecName.toLatin1() );
}

void CodeEditorSettingsTab::setEditLineLength(int lineLength)
{
    if (lineLength < 10 || lineLength > 1000)
        return;
    foreach ( BCodeEditor *edr, Application::codeEditors() )
        edr->setEditLineLength(lineLength);
    bSettings->setValue("CodeEditor/edit_line_length", lineLength);
}

void CodeEditorSettingsTab::setEditTabWidth(int tabWidth)
{
    switch (tabWidth)
    {
    case BCodeEdit::TabWidth2:
    case BCodeEdit::TabWidth4:
    case BCodeEdit::TabWidth8:
        break;
    default:
        return;
    }
    foreach ( BCodeEditor *edr, Application::codeEditors() )
        edr->setEditTabWidth( static_cast<BCodeEdit::TabWidth>(tabWidth) );
    bSettings->setValue("CodeEditor/edit_tab_width", tabWidth);
}

void CodeEditorSettingsTab::setFileHistory(const QStringList &history)
{
    foreach ( BCodeEditor *edr, Application::codeEditors() )
        edr->setFileHistory(history);
    bSettings->setValue("CodeEditor/file_history", history);
}

void CodeEditorSettingsTab::setDocumentDriverState(const QByteArray &state)
{
    bSettings->setValue("CodeEditor/document_driver_state", state);
}

void CodeEditorSettingsTab::setSearchModuleState(const QByteArray &state)
{
    bSettings->setValue("CodeEditor/search_moudle_state", state);
}

/*============================== Public methods ============================*/

QString CodeEditorSettingsTab::title() const
{
    return tr("Text editor", "title");
}

QIcon CodeEditorSettingsTab::icon() const
{
    return Application::icon("edit");
}

bool CodeEditorSettingsTab::hasAdvancedMode() const
{
    return true;
}

void CodeEditorSettingsTab::setAdvancedMode(bool enabled)
{
    setRowVisible(msboxLineLength, enabled);
    setRowVisible(mcmboxTabWidth, enabled);
}

bool CodeEditorSettingsTab::restoreDefault()
{
    QFont fnt = Application::createMonospaceFont();
    mfntcmbox->setCurrentFont(fnt);
    msboxFontPointSize->setValue( fnt.pointSize() );
    QString cn = BCodeEditor::codecName( QTextCodec::codecForLocale() );
    mcmboxEncoding->setCurrentIndex( mcmboxEncoding->findData(cn) );
    msboxLineLength->setValue(120);
    mcmboxTabWidth->setCurrentIndex( mcmboxTabWidth->findData(BCodeEdit::TabWidth4) );
    return true;
}

bool CodeEditorSettingsTab::saveSettings()
{
    setEditFontFamily( mfntcmbox->currentFont().family() );
    setEditFontPointSize( msboxFontPointSize->value() );
    setDefaultCodec( mcmboxEncoding->itemData( mcmboxEncoding->currentIndex() ).toString() );
    setEditLineLength( msboxLineLength->value() );
    setEditTabWidth( mcmboxTabWidth->itemData( mcmboxTabWidth->currentIndex() ).toInt() );
    return true;
}
