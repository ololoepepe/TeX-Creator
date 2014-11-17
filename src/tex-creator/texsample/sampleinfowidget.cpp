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

#include "sampleinfowidget.h"

#include "application.h"
#include "samplemodel.h"
#include "settings.h"
#include "texsamplecore.h"

#include <TAddSampleRequestData>
#include <TAuthorInfo>
#include <TAuthorInfoList>
#include <TAuthorListWidget>
#include <TEditSampleAdminRequestData>
#include <TEditSampleRequestData>
#include <TeXSample>
#include <TFileInfo>
#include <TNetworkClient>
#include <TSampleInfo>
#include <TSampleType>
#include <TTagWidget>
#include <TTexFile>
#include <TTexProject>

#include <BAbstractCodeEditorDocument>
#include <BCodeEditor>
#include <BeQt>
#include <BExtendedFileDialog>
#include <BInputField>

#include <QByteArray>
#include <QChar>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPlainTextEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QToolButton>
#include <QValidator>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>
#include <QWidget>

/*============================================================================
================================ SampleInfoWidget ============================
============================================================================*/

/*============================== Static private constants ==================*/

const QString SampleInfoWidget::DateTimeFormat = "dd MMMM yyyy hh:mm";
const Qt::TextInteractionFlags SampleInfoWidget::TextInteractionFlags = Qt::TextSelectableByKeyboard
        | Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;

/*============================== Public constructors =======================*/

SampleInfoWidget::SampleInfoWidget(Mode m, QWidget *parent) :
    QWidget(parent), mmode(m), meditor(0)
{
    mcache = 0;
    mclient = 0;
    mcodec = 0;
    meditor = 0;
    mid = 0;
    mmodel = 0;
    msenderId = 0;
    mvalid = false;
    //
    mledtTitle = 0;
    minputTitle = 0;
    mtbtnShowPreview = 0;
    mcboxEditSource = 0;
    mledtFileName = 0;
    minputFileName = 0;
    mlblSize = 0;
    mtbtnSetupFromCurrentDocument = 0;
    mtbtnSetupFromExternalFile = 0;
    mtgwgt = 0;
    mlblSender = 0;
    mlblCreationDT = 0;
    mlblUpdateDT = 0;
    msboxRating = 0;
    mcmboxType = 0;
    mlstwgtAuthors = 0;
    mptedtDescription = 0;
    mptedtRemark = 0;
    //
    QVBoxLayout *vlt = new QVBoxLayout(this);
    switch (mmode) {
    case AddMode: {
        createMainGroup(vlt);
        QHBoxLayout *hlt = new QHBoxLayout;
        createAuthorsGroup(hlt);
        vlt->addLayout(hlt);
        hlt = new QHBoxLayout;
        createDescriptionGroup(hlt);
        vlt->addLayout(hlt);
        break;
    }
    case EditAdminMode: {
        createMainGroup(vlt);
        QHBoxLayout *hlt = new QHBoxLayout;
        createExtraGroup(hlt);
        createAuthorsGroup(hlt);
        vlt->addLayout(hlt);
        hlt = new QHBoxLayout;
        createDescriptionGroup(hlt);
        createAdminRemarkGroup(hlt);
        vlt->addLayout(hlt);
        break;
    }
    case EditSelfMode: {
        createMainGroup(vlt);
        QHBoxLayout *hlt = new QHBoxLayout;
        createExtraGroup(hlt, true);
        createAuthorsGroup(hlt);
        vlt->addLayout(hlt);
        hlt = new QHBoxLayout;
        createDescriptionGroup(hlt);
        createAdminRemarkGroup(hlt, true);
        vlt->addLayout(hlt);
        break;
    }
    case ShowMode: {
        createMainGroup(vlt, true);
        QHBoxLayout *hlt = new QHBoxLayout;
        createExtraGroup(hlt, true);
        createAuthorsGroup(hlt, true);
        vlt->addLayout(hlt);
        hlt = new QHBoxLayout;
        createDescriptionGroup(hlt, true);
        createAdminRemarkGroup(hlt, true);
        vlt->addLayout(hlt);
        break;
    }
    default: {
        break;
    }
    }
    //
    resetFile();
    checkInputs();
}

/*============================== Public methods ============================*/

TAbstractCache *SampleInfoWidget::cache() const
{
    return mcache;
}

TNetworkClient *SampleInfoWidget::client() const
{
    return mclient;
}

QVariant SampleInfoWidget::createRequestData() const
{
    if (!hasValidInput())
        return QVariant();
    switch (mmode) {
    case AddMode: {
        TAddSampleRequestData data;
        data.setAuthors(mlstwgtAuthors->authors());
        data.setDescritpion(mptedtDescription->toPlainText().replace(QChar::ParagraphSeparator, '\n'));
        data.setProject(msource);
        data.setTags(mtgwgt->tags());
        data.setTitle(mledtTitle->text());
        return data;
    }
    case EditAdminMode: {
        TEditSampleAdminRequestData data;
        data.setAdminRemark(mptedtRemark->toPlainText().replace(QChar::ParagraphSeparator, '\n'));
        data.setAuthors(mlstwgtAuthors->authors());
        data.setDescritpion(mptedtDescription->toPlainText().replace(QChar::ParagraphSeparator, '\n'));
        data.setEditProject(mcboxEditSource->isChecked());
        data.setId(mid);
        if (mcboxEditSource->isChecked())
            data.setProject(msource);
        data.setRating(quint8(msboxRating->value()));
        data.setTags(mtgwgt->tags());
        data.setTitle(mledtTitle->text());
        data.setType(mcmboxType->itemData(mcmboxType->currentIndex()).toInt());
        return data;
    }
    case EditSelfMode: {
        TEditSampleRequestData data;
        data.setAuthors(mlstwgtAuthors->authors());
        data.setDescritpion(mptedtDescription->toPlainText().replace(QChar::ParagraphSeparator, '\n'));
        data.setEditProject(mcboxEditSource->isChecked());
        data.setId(mid);
        if (mcboxEditSource->isChecked())
            data.setProject(msource);
        data.setTags(mtgwgt->tags());
        data.setTitle(mledtTitle->text());
        return data;
    }
    case ShowMode:
    default: {
        break;
    }
    }
    return QVariant();
}

BCodeEditor *SampleInfoWidget::editor() const
{
    return meditor;
}

bool SampleInfoWidget::hasValidInput() const
{
    return mvalid;
}

SampleInfoWidget::Mode SampleInfoWidget::mode() const
{
    return mmode;
}

SampleModel *SampleInfoWidget::model() const
{
    return mmodel;
}

void SampleInfoWidget::restoreState(const QByteArray &state)
{
    QVariantMap m = BeQt::deserialize(state).toMap();
    mlstwgtAuthors->setAvailableAuthors(m.value("authors").value<TAuthorInfoList>());
    mtgwgt->setAvailableTags(m.value("tags").toStringList());
    mledtTitle->setText(m.value("title").toString());
    mcodec = BeQt::codec(m.value("codec_name").toString());
    QString fn = m.value("file_name").toString();
    if (msource.load(fn, mcodec)) {
        resetFile(fn, msource.size());
    } else {
        resetFile();
        mcodec = 0;
    }
}

QByteArray SampleInfoWidget::saveState() const
{
    QVariantMap m;
    TAuthorInfoList list = mlstwgtAuthors->availableAuthors();
    list << mlstwgtAuthors->authors();
    bRemoveDuplicates(list);
    m.insert("authors", list);
    m.insert("tags", mtgwgt->availableTags());
    m.insert("title", mledtTitle->text());
    m.insert("file_name", mledtFileName->toolTip());
    m.insert("codec_name", BeQt::codecName(mcodec));
    return BeQt::serialize(m);
}

void SampleInfoWidget::setCache(TAbstractCache *cache)
{
    mcache = cache;
}

void SampleInfoWidget::setClient(TNetworkClient *client)
{
    mclient = client;
}

void SampleInfoWidget::setEditor(BCodeEditor *editor)
{
    if (meditor)
        disconnect(meditor, SIGNAL(documentAvailableChanged(bool)), this, SLOT(checkInputs()));
    meditor = editor;
    if (meditor)
        connect(meditor, SIGNAL(documentAvailableChanged(bool)), this, SLOT(checkInputs()));
    checkInputs();
}

void SampleInfoWidget::setModel(SampleModel *model)
{
    mmodel = model;
}

bool SampleInfoWidget::setSample(quint64 sampleId)
{
    if (AddMode == mmode || !mmodel)
        return false;
    TSampleInfo info = mmodel->sampleInfo(sampleId);
    mid = info.id();
    msenderId = info.senderId();
    resetFile(info.mainSourceFile().fileName(), info.sourceSize());
    mtgwgt->setTags(info.tags());
    mledtTitle->setText(info.title());
    mlblSender->setText("<a href=dummy>" + info.senderLogin() + "</a>");
    mlblSender->setToolTip(tr("Click the link to see info about the sender", "lbl toolTip"));
    msboxRating->setValue(info.rating());
    mcmboxType->setCurrentIndex(mcmboxType->findData(int(info.type())));
    mlblCreationDT->setText(info.creationDateTime().toTimeSpec(Qt::LocalTime).toString(DateTimeFormat));
    mlblUpdateDT->setText(info.lastModificationDateTime().toTimeSpec(Qt::LocalTime).toString(DateTimeFormat));
    mlstwgtAuthors->setAuthors(info.authors());
    mptedtDescription->setPlainText(info.description());
    mptedtRemark->setPlainText(info.adminRemark());
    QString s = BeQt::fileSizeToString(info.previewSize(), BeQt::KilobytesFormat);
    if(mtbtnShowPreview)
        mtbtnShowPreview->setToolTip(tr("Show sample preview", "tbtn toolTip") + " (" + s + ")");
    checkInputs();
    return info.isValid();
}

QString SampleInfoWidget::title() const
{
    return mledtTitle->text();
}

/*============================== Private methods ===========================*/

void SampleInfoWidget::createAdminRemarkGroup(QHBoxLayout *hlt, bool readOnly)
{
    QGroupBox *gbox = new QGroupBox(tr("Admin remark:", "gbox title"));
      QVBoxLayout *vlt = new QVBoxLayout(gbox);
        mptedtRemark = new QPlainTextEdit;
          mptedtRemark->setReadOnly(readOnly);
          mptedtRemark->setMaximumHeight(120);
        vlt->addWidget(mptedtRemark);
    hlt->addWidget(gbox);
}

void SampleInfoWidget::createAuthorsGroup(QHBoxLayout *hlt, bool readOnly)
{
    QGroupBox *gbox = new QGroupBox(tr("Authors:", "gbox title"));
      QVBoxLayout *vlt = new QVBoxLayout(gbox);
        mlstwgtAuthors = new TAuthorListWidget;
          mlstwgtAuthors->setReadOnly(readOnly);
          mlstwgtAuthors->setButtonsVisible(!readOnly);
        vlt->addWidget(mlstwgtAuthors);
    hlt->addWidget(gbox);
}

void SampleInfoWidget::createDescriptionGroup(QHBoxLayout *hlt, bool readOnly)
{
    QGroupBox *gbox = new QGroupBox(tr("Description:", "gbox title"));
      QVBoxLayout *vlt = new QVBoxLayout(gbox);
        mptedtDescription = new QPlainTextEdit;
          mptedtDescription->setReadOnly(readOnly);
          mptedtDescription->setMaximumHeight(120);
        vlt->addWidget(mptedtDescription);
    hlt->addWidget(gbox);
}

void SampleInfoWidget::createEditSourceField(QFormLayout *flt)
{
    mcboxEditSource = new QCheckBox;
      connect(mcboxEditSource, SIGNAL(toggled(bool)), this, SLOT(checkInputs()));
    flt->addRow(tr("Edit source:", "lbl text"), mcboxEditSource);
}

void SampleInfoWidget::createExtraGroup(QHBoxLayout *hlt, bool readOnly)
{
    QFormLayout *flt = new QFormLayout;
      mlblSender = new QLabel;
        mlblSender->setTextInteractionFlags(TextInteractionFlags);
        connect(mlblSender, SIGNAL(linkActivated(QString)), this, SLOT(showSenderInfo()));
      flt->addRow(tr("Sender:", "lbl text"), mlblSender);
      msboxRating = new QSpinBox;
        msboxRating->setMinimum(0);
        msboxRating->setMaximum(100);
        msboxRating->setValue(0);
        msboxRating->setEnabled(!readOnly);
      flt->addRow(tr("Rating:", "lbl text"), msboxRating);
      mcmboxType = new QComboBox;
        foreach (const TSampleType &t, TSampleType::allTypes())
            mcmboxType->addItem(TSampleType::sampleTypeToString(t, true), int(t));
        mcmboxType->setEnabled(!readOnly);
      flt->addRow(tr("Type:", "lbl text"), mcmboxType);
      mlblCreationDT = new QLabel;
        mlblCreationDT->setTextInteractionFlags(TextInteractionFlags);
      flt->addRow(tr("Creation date:", "lbl text"), mlblCreationDT);
      mlblUpdateDT = new QLabel;
        mlblUpdateDT->setTextInteractionFlags(TextInteractionFlags);
      flt->addRow(tr("Modified:", "lbl text"), mlblUpdateDT);
    hlt->addLayout(flt);
}

void SampleInfoWidget::createFileField(QFormLayout *flt, bool readOnly)
{
    QHBoxLayout *hlt = new QHBoxLayout;
      mledtFileName = new QLineEdit;
        mledtFileName->setReadOnly(readOnly);
        mledtFileName->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9\\-]+(\\.tex)?")));
        connect(mledtFileName, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
        connect(mledtFileName, SIGNAL(textChanged(QString)), this, SLOT(setFileName(QString)));
        minputFileName = new BInputField(readOnly ? BInputField::ShowNever : BInputField::ShowAlways);
        minputFileName->addWidget(mledtFileName);
      hlt->addWidget(minputFileName);
      mlblSize = new QLabel;
        mlblSize->setTextInteractionFlags(TextInteractionFlags);
      hlt->addWidget(mlblSize);
      QSize sz(mledtFileName->sizeHint().height(), mledtFileName->sizeHint().height());
      mtbtnSetupFromCurrentDocument = new QToolButton;
        mtbtnSetupFromCurrentDocument->setIconSize(sz - (mtbtnSetupFromCurrentDocument ->sizeHint()
                                                         - mtbtnSetupFromCurrentDocument->iconSize()));
        mtbtnSetupFromCurrentDocument->setIcon(Application::icon("tex"));
        mtbtnSetupFromCurrentDocument->setFixedSize(sz);
        mtbtnSetupFromCurrentDocument->setToolTip(tr("Use current document", "tbtn toolTip"));
        connect(mtbtnSetupFromCurrentDocument, SIGNAL(clicked()), this, SLOT(setupFromCurrentDocument()));
      hlt->addWidget(mtbtnSetupFromCurrentDocument);
      mtbtnSetupFromSelectedText = new QToolButton;
        mtbtnSetupFromSelectedText->setIconSize(sz - (mtbtnSetupFromSelectedText ->sizeHint()
                                                      - mtbtnSetupFromSelectedText->iconSize()));
        mtbtnSetupFromSelectedText->setIcon(Application::icon("selection"));
        mtbtnSetupFromSelectedText->setFixedSize(sz);
        mtbtnSetupFromSelectedText->setToolTip(tr("Use current document selected text", "tbtn toolTip"));
        connect(mtbtnSetupFromSelectedText, SIGNAL(clicked()), this, SLOT(setupFromSelectedText()));
      hlt->addWidget(mtbtnSetupFromSelectedText);
      mtbtnSetupFromExternalFile = new QToolButton;
        mtbtnSetupFromExternalFile->setIconSize(sz - (mtbtnSetupFromExternalFile ->sizeHint()
                                                      - mtbtnSetupFromExternalFile->iconSize()));
        mtbtnSetupFromExternalFile->setIcon(Application::icon("fileopen"));
        mtbtnSetupFromExternalFile->setFixedSize(sz);
        mtbtnSetupFromExternalFile->setToolTip(tr("Use external file...", "tbtn toolTip"));
        mtbtnSetupFromExternalFile->setEnabled(!readOnly);
        connect(mtbtnSetupFromExternalFile, SIGNAL(clicked()), this, SLOT(setupFromExternalFile()));
      hlt->addWidget(mtbtnSetupFromExternalFile);
    flt->addRow(tr("File:", "lbl text"), hlt);
}

void SampleInfoWidget::createMainGroup(QVBoxLayout *vlt, bool readOnly)
{
    QFormLayout *flt = new QFormLayout;
      createTitleField(flt, readOnly);
      if (EditAdminMode == mmode || EditSelfMode == mmode)
          createEditSourceField(flt);
      createFileField(flt, readOnly);
      createTagsField(flt, readOnly);
    vlt->addLayout(flt);
}

void SampleInfoWidget::createTagsField(QFormLayout *flt, bool readOnly)
{
    mtgwgt = new TTagWidget;
      mtgwgt->setReadOnly(readOnly);
      mtgwgt->setButtonsVisible(!readOnly);
    flt->addRow(tr("Tags:", "lbl text"), mtgwgt);
}

void SampleInfoWidget::createTitleField(QFormLayout *flt, bool readOnly)
{
    QHBoxLayout *hlt = new QHBoxLayout;
      mledtTitle = new QLineEdit;
        mledtTitle->setReadOnly(readOnly);
        QRegExp rx(".{1," + QString::number(Texsample::MaximumSampleTitleLength) + "}");
        mledtTitle->setValidator(new QRegExpValidator(rx, this));
        connect(mledtTitle, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
        minputTitle = new BInputField(readOnly ? BInputField::ShowNever : BInputField::ShowAlways);
        minputTitle->addWidget(mledtTitle);
      hlt->addWidget(minputTitle);
      if (readOnly) {
          QSize sz(mledtTitle->sizeHint().height(), mledtTitle->sizeHint().height());
          mtbtnShowPreview = new QToolButton;
            mtbtnShowPreview ->setIconSize(sz - (mtbtnShowPreview ->sizeHint() - mtbtnShowPreview->iconSize()));
            mtbtnShowPreview->setIcon(Application::icon("pdf"));
            mtbtnShowPreview->setFixedSize(sz);
            mtbtnShowPreview->setToolTip(tr("Show sample preview", "tbtn toolTip"));
            connect(mtbtnShowPreview, SIGNAL(clicked()), this, SLOT(showPreview()));
          hlt->addWidget(mtbtnShowPreview);
      }
    flt->addRow(tr("Title:", "lbl text"), hlt);
}

void SampleInfoWidget::resetFile(const QString &fileName, int size)
{
    QString fn = QFileInfo(fileName).fileName().replace(QRegExp("\\s+"), "-");
    mledtFileName->setText(QRegExp("[a-zA-Z0-9\\-]+(\\.tex)?").exactMatch(fn) ? fn : "RENAME-ME.tex");
    msource.rootFile().setFileName("RENAME-ME.tex");
    mledtFileName->setToolTip(fileName);
    if (size < 0)
        size = 0;
    QString s = tr("Size:", "lbl text") + " ";
    s += (size ? BeQt::fileSizeToString(size, BeQt::KilobytesFormat, 1) : tr("Unknown", "lbl text"));
    mlblSize->setText(s);
}

/*============================== Private slots =============================*/

void SampleInfoWidget::checkInputs()
{
    bool idValid = (AddMode == mmode) || mid;
    bool titleValid = mledtTitle->hasAcceptableInput();
    bool sourceValid = (ShowMode == mmode)
            || (((mcboxEditSource && !mcboxEditSource->isChecked()) || msource.isValid())
                && mledtFileName->hasAcceptableInput());
    minputTitle->setValid(titleValid);
    minputFileName->setValid(sourceValid);
    mtbtnSetupFromCurrentDocument->setEnabled(meditor && meditor->documentAvailable());
    mtbtnSetupFromSelectedText->setEnabled(meditor && meditor->documentAvailable());
    bool v = idValid && titleValid && sourceValid;
    if (v == mvalid)
        return;
    mvalid = v;
    emit inputValidityChanged(mvalid);
}

void SampleInfoWidget::setFileName(const QString &fileName)
{
    msource.rootFile().setFileName(fileName);
}

void SampleInfoWidget::setupFromCurrentDocument()
{
    BAbstractCodeEditorDocument *doc = meditor ? meditor->currentDocument() : 0;
    if (!doc)
        return;
    if (msource.load(doc->fileName(), doc->text(), doc->codec())) {
        resetFile(doc->fileName(), msource.size());
        mcodec = doc->codec();
    } else {
        resetFile();
        mcodec = 0;
    }
    checkInputs();
}

void SampleInfoWidget::setupFromSelectedText()
{
    BAbstractCodeEditorDocument *doc = meditor ? meditor->currentDocument() : 0;
    if (!doc)
        return;
    if (msource.load(doc->fileName(), doc->selectedText(), doc->codec())) {
        resetFile(doc->fileName(), msource.size());
        mcodec = doc->codec();
    } else {
        resetFile();
        mcodec = 0;
    }
    checkInputs();
}

void SampleInfoWidget::setupFromExternalFile()
{
    BExtendedFileDialog dlg(this);
    dlg.setNameFilter(tr("LaTeX files") + " (*.tex)");
    if (!dlg.restoreGeometry(Settings::SampleInfoWidget::selectExternalFileDialogGeometry()))
        dlg.resize(700, 400);
    QByteArray state = Settings::SampleInfoWidget::selectExternalFileDialogState();
    if (!state.isEmpty())
        dlg.restoreState(state);
    else
        dlg.setDirectory(QDir::homePath());
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFile);
    if (dlg.exec() != BExtendedFileDialog::Accepted)
        return;
    QStringList files = dlg.selectedFiles();
    if (files.size() != 1)
        return;
    Settings::SampleInfoWidget::setSelectExternalFileDialogGeometry(dlg.saveGeometry());
    Settings::SampleInfoWidget::setSelectExternalFileDialogState(dlg.saveState());
    if (msource.load(files.first(), dlg.selectedCodec())) {
        resetFile(files.first(), msource.size());
        mcodec = dlg.selectedCodec();
    } else {
        resetFile();
        mcodec = 0;
    }
    checkInputs();
}

void SampleInfoWidget::showPreview()
{
    if (!mid)
        return;
    tSmp->showSamplePreview(mid);
}

void SampleInfoWidget::showSenderInfo()
{
    if (!msenderId)
        return;
    tSmp->showUserInfo(msenderId);
}
