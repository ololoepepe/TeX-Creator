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

#include "samplewidget.h"
#include "client.h"
#include "application.h"
#include "cache.h"

#include <TSampleInfo>
#include <TCompilationResult>
#include <TUserInfo>
#include <TTexProject>
#include <TUserWidget>
#include <TTagsWidget>
#include <TListWidget>

#include <BFlowLayout>
#include <BAbstractCodeEditorDocument>
#include <BCodeEditor>
#include <BeQt>
#include <BExtendedFileDialog>
#include <BDialog>
#include <BInputField>

#include <QWidget>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QLineEdit>
#include <QLabel>
#include <QList>
#include <QComboBox>
#include <QVariant>
#include <QToolButton>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSpinBox>
#include <QRegExp>
#include <QFileInfo>
#include <QGroupBox>
#include <QSettings>
#include <QSplitter>
#include <QToolButton>
#include <QChar>
#include <QByteArray>
#include <QVariantMap>
#include <QTimer>
#include <QMetaObject>
#include <QRegExp>
#include <QRegExpValidator>
#include <QValidator>

#include <QDebug>

/*============================================================================
================================ SampleWidget ================================
============================================================================*/

/*============================== Static public methods =====================*/

bool SampleWidget::showSelectSampleDialog(QString &fileName, QTextCodec *&codec, QWidget *parent)
{
    BExtendedFileDialog dlg(parent);
    dlg.setNameFilter(tr("LaTeX files") + " (*.tex)");
    if (!dlg.restoreGeometry(bSettings->value("SampleWidget/select_sample_dialog_geometry").toByteArray()))
        dlg.resize(700, 400);
    QByteArray state = bSettings->value("SampleWidget/select_sample_dialog_state").toByteArray();
    if (!state.isEmpty())
        dlg.restoreState(state);
    else
        dlg.setDirectory(QDir::homePath());
    if (codec)
        dlg.selectCodec(codec);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFile);
    bool b = dlg.exec() == BExtendedFileDialog::Accepted;
    QStringList files = dlg.selectedFiles();
    b = b && !files.isEmpty();
    bSettings->setValue("SampleWidget/select_sample_dialog_geometry", dlg.saveGeometry());
    bSettings->setValue("SampleWidget/select_sample_dialog_state", dlg.saveState());
    if (!b)
        return false;
    fileName = files.first();
    codec = dlg.selectedCodec();
    return true;
}

/*============================== Public constructors =======================*/

SampleWidget::SampleWidget(Mode m, QWidget *parent) :
    QWidget(parent), mmode(m), meditor(0)
{
    init();
}

SampleWidget::SampleWidget(Mode m, BCodeEditor *editor, QWidget *parent) :
    QWidget(parent), mmode(m), meditor(editor)
{
    init();
}

/*============================== Public methods ============================*/

void SampleWidget::setInfo(const TSampleInfo &info)
{
    mid = info.id();
    msenderId = info.sender().id();
    msenderLogin = info.sender().login();
    msenderRealName = info.sender().realName();
    mledtTitle->setText(info.title());
    if (!mledtTitle->hasAcceptableInput())
        mledtTitle->clear();
    mledtFileName->setText(info.fileName());
    if (!mledtFileName->hasAcceptableInput())
        mledtFileName->clear();
    mtgswgt->setTags(info.tags());
    msboxRating->setValue(info.rating());
    setProjectSize(info.projectSize());
    mcmboxType->setCurrentIndex(mcmboxType->findData(info.type()));
    if (!msenderLogin.isEmpty())
    {
        QString s = "<a href=x>" + msenderLogin + "</a>";
        s += !msenderRealName.isEmpty() ? (" (" + msenderRealName + ")") : QString();
        mlblSender->setText(s);
        mlblSender->setToolTip(tr("Click the link to see info about the sender", "lbl toolTip"));
    }
    else
    {
        mlblSender->clear();
        mlblSender->setToolTip("");
    }
    if (info.creationDateTime().isValid())
        mlblCreationDT->setText(info.creationDateTime(Qt::LocalTime).toString(DateTimeFormat));
    else
        mlblCreationDT->clear();
    if (info.updateDateTime().isValid())
        mlblUpdateDT->setText(info.updateDateTime(Qt::LocalTime).toString(DateTimeFormat));
    else
        mlblUpdateDT->clear();
    mlstwgt->setItems(info.authors());
    mptedtComment->setPlainText(info.comment());
    mptedtRemark->setPlainText(info.adminRemark());
    setFocus();
    checkInputs();
}

void SampleWidget::setCheckSourceValidity(bool b)
{
    if (b == mcheckSource)
        return;
    mcheckSource = b;
    checkInputs();
}

void SampleWidget::restoreState(const QByteArray &state)
{
    QVariantMap m = BeQt::deserialize(state).toMap();
    mtgswgt->setAvailableTags(m.value("tags").toStringList());
    mlstwgt->setAvailableItems(m.value("authors").toStringList());
}

void SampleWidget::restoreSourceState(const QByteArray &state)
{
    QVariantMap m = BeQt::deserialize(state).toMap();
    mactualFileName = m.value("file_name").toString();
    mcodec = BeQt::codec(m.value("codec_name").toString());
    QFileInfo fi(mactualFileName);
    if (fi.isAbsolute() && fi.isFile())
        setProjectSize(TTexProject::size(mactualFileName, mcodec));
    else
        setProjectSize();
}

SampleWidget::Mode SampleWidget::mode() const
{
    return mmode;
}

TSampleInfo SampleWidget::info() const
{
    TSampleInfo info;
    switch (mmode)
    {
    case AddMode:
        info.setContext(TSampleInfo::AddContext);
        break;
    case EditMode:
        info.setContext(TSampleInfo::EditContext);
        break;
    case UpdateMode:
        info.setContext(TSampleInfo::UpdateContext);
        break;
    case ShowMode:
        info.setContext(TSampleInfo::GeneralContext);
        break;
    default:
        break;
    }
    info.setId(mid);
    TUserInfo u(msenderId, TUserInfo::BriefInfoContext);
    u.setLogin(msenderLogin);
    u.setRealName(msenderRealName);
    info.setSender(u);
    info.setTitle(mledtTitle->text());
    info.setFileName(createFileName(mledtFileName->text()));
    info.setProjectSize(mprojectSize);
    info.setTags(mtgswgt->tags());
    info.setRating(msboxRating->value());
    info.setType(mcmboxType->itemData(mcmboxType->currentIndex()).toInt());
    info.setCreationDateTime(QDateTime::fromString(mlblCreationDT->text(), DateTimeFormat));
    info.setUpdateDateTime(QDateTime::fromString(mlblUpdateDT->text(), DateTimeFormat));
    info.setAuthors(mlstwgt->items());
    info.setComment(mptedtComment->toPlainText().replace(QChar::ParagraphSeparator, '\n'));
    info.setAdminRemark(mptedtRemark->toPlainText().replace(QChar::ParagraphSeparator, '\n'));
    return info;
}

bool SampleWidget::checkSourceValidity() const
{
    return mcheckSource;
}

QString SampleWidget::actualFileName() const
{
    return mdoc ? mdoc->fileName() : mactualFileName;
}

QTextCodec *SampleWidget::codec() const
{
    return mdoc ? mdoc->codec() : mcodec;
}

BAbstractCodeEditorDocument *SampleWidget::document() const
{
    return mdoc;
}

QByteArray SampleWidget::saveState() const
{
    QVariantMap m;
    m.insert("tags", mtgswgt->availableTags());
    m.insert("authors", mlstwgt->availableItems());
    return BeQt::serialize(m);
}

QByteArray SampleWidget::saveSourceState() const
{
    QVariantMap m;
    m.insert("file_name", mactualFileName);
    m.insert("codec_name", BeQt::codecName(mcodec));
    return BeQt::serialize(m);
}

bool SampleWidget::isValid() const
{
    return mvalid;
}

/*============================== Public slots ==============================*/

void SampleWidget::clear()
{
    setInfo(TSampleInfo());
    mactualFileName.clear();
    mcodec = 0;
    mdoc = 0;
}

void SampleWidget::setFocus()
{
    mledtTitle->setFocus();
    if (!mledtTitle->isReadOnly())
        mledtTitle->selectAll();
}

void SampleWidget::setupFromCurrentDocument()
{
    mdoc = meditor ? meditor->currentDocument() : 0;
    if (!mdoc)
        return;
    setFile(mdoc->fileName(), mdoc->codec());
    if (!QFileInfo(mdoc->fileName()).isFile())
        setProjectSize(mdoc->text().length() * 2);
}

void SampleWidget::setupFromExternalFile(const QString &fileName, QTextCodec *codec)
{
    if (!fileName.isEmpty())
    {
        mdoc = 0;
        setFile(fileName, codec);
    }
    else
    {
        QString fn;
        QTextCodec *c = mcodec;
        if (!showSelectSampleDialog(fn, c, this))
            return;
        mdoc = 0;
        setFile(fn, c);
    }
}

/*============================== Private methods ===========================*/

void SampleWidget::init()
{
    mvalid = false;
    mcheckSource = false;
    mid = 0;
    msenderId = 0;
    mprojectSize = 0;
    mcodec = 0;
    mdoc = 0;
    Qt::TextInteractionFlags tiflags = Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse
            | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QFormLayout *flt = new QFormLayout;
        QHBoxLayout *hlt = new QHBoxLayout;
          mledtTitle = new QLineEdit;
            mledtTitle->setReadOnly(ShowMode == mmode);
            mledtTitle->setMaxLength(120);
            connect(mledtTitle, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
            minputTitle = new BInputField;
            minputTitle->addWidget(mledtTitle);
            minputTitle->setShowStyle(ShowMode == mmode ? BInputField::ShowNever : BInputField::ShowAlways);
          hlt->addWidget(minputTitle);
          QToolButton *tbtn = new QToolButton;
            tbtn->setIcon(Application::icon("pdf"));
            tbtn->setToolTip(tr("Preview sample", "tbtn toolTip"));
            tbtn->setEnabled(ShowMode == mmode);
            connect(tbtn, SIGNAL(clicked()), this, SLOT(previewSample()));
          hlt->addWidget(tbtn);
        flt->addRow(tr("Title:", "lbl text"), hlt);
        hlt = new QHBoxLayout;
          mledtFileName = new QLineEdit;
            mledtFileName->setReadOnly(ShowMode == mmode);
            mledtFileName->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9\\-]+(\\.tex)?")));
            connect(mledtFileName, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
            minputFileName = new BInputField;
            minputFileName->addWidget(mledtFileName);
            minputFileName->setShowStyle(ShowMode == mmode ? BInputField::ShowNever : BInputField::ShowAlways);
          hlt->addWidget(minputFileName);
          mlblSize = new QLabel;
            mlblSize->setTextInteractionFlags(tiflags);
            setProjectSize();
          hlt->addWidget(mlblSize);
          mtbtnUseCurrentDocument = new QToolButton;
            mtbtnUseCurrentDocument->setIcon(Application::icon("tex"));
            mtbtnUseCurrentDocument->setToolTip(tr("Use current document", "tbtn toolTip"));
            if (meditor)
                connect(meditor, SIGNAL(documentAvailableChanged(bool)), this, SLOT(documentAvailableChanged(bool)));
            documentAvailableChanged(meditor && meditor->documentAvailable());
            connect(mtbtnUseCurrentDocument, SIGNAL(clicked()), this, SLOT(setupFromCurrentDocument()));
          hlt->addWidget(mtbtnUseCurrentDocument);
          tbtn = new QToolButton;
            tbtn->setIcon(Application::icon("fileopen"));
            tbtn->setToolTip(tr("Use external file...", "tbtn toolTip"));
            tbtn->setEnabled(ShowMode != mmode);
            connect(tbtn, SIGNAL(clicked()), this, SLOT(setupFromExternalFile()));
          hlt->addWidget(tbtn);
        flt->addRow(tr("File name:", "lbl text"), hlt);
        mtgswgt = new TTagsWidget;
          mtgswgt->setReadOnly(ShowMode == mmode);
        flt->addRow(tr("Tags:", "lbl text"), mtgswgt);
      vlt->addLayout(flt);
      hlt = new QHBoxLayout;
        flt = new QFormLayout;
          msboxRating = new QSpinBox;
            msboxRating->setMinimum(0);
            msboxRating->setMaximum(100);
            msboxRating->setValue(0);
            msboxRating->setEnabled(EditMode == mmode);
          flt->addRow(tr("Rating:", "lbl text"), msboxRating);
          mcmboxType = new QComboBox;
            foreach (const TSampleInfo::Type &t, TSampleInfo::allTypes())
                mcmboxType->addItem(TSampleInfo::typeToString(t, true), t);
            mcmboxType->setEnabled(EditMode == mmode);
          flt->addRow(tr("Type:", "lbl text"), mcmboxType);
          mlblSender = new QLabel;
            mlblSender->setTextInteractionFlags(tiflags);
            connect(mlblSender, SIGNAL(linkActivated(QString)), this, SLOT(showSenderInfo()));
          flt->addRow(tr("Sender:", "lbl text"), mlblSender);
          mlblCreationDT = new QLabel;
            mlblCreationDT->setTextInteractionFlags(tiflags);
          flt->addRow(tr("Created:", "lbl text"), mlblCreationDT);
          mlblUpdateDT = new QLabel;
            mlblUpdateDT->setTextInteractionFlags(tiflags);
          flt->addRow(tr("Updated:", "lbl text"), mlblUpdateDT);
        hlt->addLayout(flt);
        QGroupBox *gbox = new QGroupBox(tr("Authors", "gbox title"));
          QHBoxLayout *hltw = new QHBoxLayout(gbox);
            mlstwgt = new TListWidget;
              mlstwgt->setReadOnly(ShowMode == mmode);
              mlstwgt->setButtonsVisible(ShowMode != mmode);
            hltw->addWidget(mlstwgt);
        hlt->addWidget(gbox);
      vlt->addLayout(hlt);
      hlt = new QHBoxLayout;
        gbox = new QGroupBox(tr("Comment", "gbox title"));
          hltw = new QHBoxLayout(gbox);
            mptedtComment = new QPlainTextEdit;
              mptedtComment->setReadOnly(ShowMode == mmode);
            hltw->addWidget(mptedtComment);
        hlt->addWidget(gbox);
        gbox = new QGroupBox(tr("Moderator remark", "gbox title"));
          hltw = new QHBoxLayout(gbox);
            mptedtRemark = new QPlainTextEdit;
              mptedtRemark->setReadOnly(EditMode != mmode);
            hltw->addWidget(mptedtRemark);
        hlt->addWidget(gbox);
      vlt->addLayout(hlt);
    //
    Application::setRowVisible(msboxRating, AddMode != mmode);
    Application::setRowVisible(mcmboxType, AddMode != mmode);
    Application::setRowVisible(mlblSender, AddMode != mmode);
    Application::setRowVisible(mlblCreationDT, AddMode != mmode);
    Application::setRowVisible(mlblUpdateDT, AddMode != mmode);
    checkInputs();
}

void SampleWidget::setProjectSize(int sz)
{
    if (sz < 0)
        sz = 0;
    mprojectSize = sz;
    QString s = tr("Size:", "lbl text") + " ";
    if (sz)
        s += BeQt::fileSizeToString(sz, BeQt::KilobytesFormat, 1);
    else
        s += tr("Unknown", "lbl text");
    mlblSize->setText(s);
}

/*============================== Private slots =============================*/

void SampleWidget::documentAvailableChanged(bool available)
{
    mtbtnUseCurrentDocument->setEnabled(ShowMode != mmode && meditor && available);
}

void SampleWidget::checkInputs()
{
    minputTitle->setValid(!mledtTitle->text().isEmpty() && mledtTitle->hasAcceptableInput());
    minputFileName->setValid(!mledtFileName->text().isEmpty() && mledtFileName->hasAcceptableInput());
    bool v = info().isValid() && (!mcheckSource || !mactualFileName.isEmpty() || mdoc);
    if (v == mvalid)
        return;
    mvalid = v;
    emit validityChanged(v);
}

void SampleWidget::showSenderInfo()
{
    if (!msenderId)
        return;
    TUserInfo info;
    if (sClient->isAuthorized())
    {
        if (!sClient->getUserInfo(msenderId, info, this))
            return;
    }
    else
    {
        info = sCache->userInfo(msenderId);
        if (!info.isValid())
            return;
    }
    BDialog dlg(this);
    dlg.setWindowTitle(tr("User:", "windowTitle") + " " + info.login());
    TUserWidget *uwgt = new TUserWidget(TUserWidget::ShowMode);
    uwgt->setInfo(info);
    dlg.setWidget(uwgt);
    dlg.addButton(QDialogButtonBox::Close, SLOT(close()));
    dlg.setMinimumSize(600, dlg.sizeHint().height());
    dlg.exec();
}

void SampleWidget::previewSample()
{
    if (!mid)
        return;
    if (!sClient->previewSample(mid))
    {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Failed to show preview", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to get or show sample preview", "msgbox text"));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
    }
}

void SampleWidget::setFile(const QString &fn, QTextCodec *codec)
{
    QFileInfo fi(fn);
    mcodec = codec;
    if (fi.isAbsolute() && fi.isFile())
    {
        mactualFileName = fn;
        setProjectSize(TTexProject::size(mactualFileName, codec, true));
        mledtFileName->setText(createFileName(mactualFileName));
    }
    else
    {
        mactualFileName.clear();
        setProjectSize();
        mledtFileName->setText(createFileName(fn));
    }
    if (!mledtFileName->hasAcceptableInput())
        mledtFileName->clear();
    checkInputs();
}

/*============================== Static private methods ====================*/

QString SampleWidget::createFileName(const QString &fn)
{
    return !fn.isEmpty() ? (QFileInfo(fn).baseName() + ".tex") : QString();
}

/*============================== Static private constants ==================*/

const QString SampleWidget::DateTimeFormat = "dd MMMM yyyy hh:mm";
