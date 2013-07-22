#include "samplewidget.h"
#include "client.h"
#include "application.h"
#include "userwidget.h"
#include "cache.h"

#include <TSampleInfo>
#include <TCompilationResult>
#include <TUserInfo>
#include <BFlowLayout>
#include <BAbstractCodeEditorDocument>
#include <BCodeEditor>
#include <BeQt>
#include <BExtendedFileDialog>
#include <BDialog>

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
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QAction>
#include <QSignalMapper>
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
    mledtFileName->setText(info.fileName());
    mledtTags->setText(info.tagsString());
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
    setAuthors(info.authors());
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
    mtbtnTags->menu()->clear();
    QStringList tags = m.value("tags").toStringList();
    tags.removeAll("");
    tags.removeDuplicates();
    while (tags.size() > 20)
        tags.removeFirst();
    foreach (const QString &tag, tags)
    {
        QAction *act = mtbtnTags->menu()->addAction(tag);
        bSetMapping(mmprTags, act, SIGNAL(triggered()), tag);
    }
    mtbtnTags->setEnabled(ShowMode != mmode && !mtbtnTags->menu()->isEmpty());
    mtbtnAdd->menu()->clear();
    QStringList list = m.value("authors").toStringList();
    list.removeAll("");
    list.removeDuplicates();
    while (list.size() > 20)
        list.removeFirst();
    foreach (const QString &s, list)
    {
        QAction *act = mtbtnAdd->menu()->addAction(s);
        bSetMapping(mmprAuthors, act, SIGNAL(triggered()), s);
    }
    mtbtnAdd->setEnabled(ShowMode != mmode && !mtbtnAdd->menu()->isEmpty());
}

void SampleWidget::restoreSourceState(const QByteArray &state)
{
    QVariantMap m = BeQt::deserialize(state).toMap();
    mactualFileName = m.value("file_name").toString();
    mcodec = BeQt::codec(m.value("codec_name").toString());
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
    info.setTags(mledtTags->text());
    info.setRating(msboxRating->value());
    info.setType(mcmboxType->itemData(mcmboxType->currentIndex()).toInt());
    info.setCreationDateTime(QDateTime::fromString(mlblCreationDT->text(), DateTimeFormat));
    info.setUpdateDateTime(QDateTime::fromString(mlblUpdateDT->text(), DateTimeFormat));
    info.setAuthors(authors());
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
    QStringList list = TSampleInfo::listFromString(mledtTags->text());
    foreach (QAction *act, mtbtnTags->menu()->actions())
        list << act->text();
    list.removeAll("");
    list.removeDuplicates();
    while (list.size() > 20)
        list.removeFirst();
    m.insert("tags", list);
    list = authors();
    foreach (QAction *act, mtbtnAdd->menu()->actions())
        list << act->text();
    list.removeAll("");
    list.removeDuplicates();
    while (list.size() > 20)
        list.removeFirst();
    m.insert("authors", list);
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
    if (mledtTitle->isReadOnly())
        return;
    mledtTitle->setFocus();
    mledtTitle->selectAll();
}

void SampleWidget::setupFromCurrentDocument()
{
    mdoc = meditor ? meditor->currentDocument() : 0;
    if (!mdoc)
        return;
    setFile(mdoc->fileName(), mdoc->codec());
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
    mmprTags = new QSignalMapper(this);
    connect(mmprTags, SIGNAL(mapped(QString)), this, SLOT(addTag(QString)));
    mmprAuthors = new QSignalMapper(this);
    connect(mmprAuthors, SIGNAL(mapped(QString)), this, SLOT(addAuthor(QString)));
    Qt::TextInteractionFlags tiflags = Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse
            | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QFormLayout *flt = new QFormLayout;
        QHBoxLayout *hlt = new QHBoxLayout;
          mledtTitle = new QLineEdit;
            mledtTitle->setReadOnly(ShowMode == mmode);
            mledtTitle->setMaxLength(120);
            connect(mledtTitle, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
          hlt->addWidget(mledtTitle);
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
            //mledtFileName->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9\\-]+(\\.tex)?")));
            connect(mledtFileName, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
          hlt->addWidget(mledtFileName);
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
        hlt = new QHBoxLayout;
          mledtTags = new QLineEdit;
            mledtTags->setReadOnly(ShowMode == mmode);
          hlt->addWidget(mledtTags);
          mtbtnTags = new QToolButton;
            mtbtnTags->setMenu(new QMenu);
            mtbtnTags->setPopupMode(QToolButton::InstantPopup);
            mtbtnTags->setIcon(Application::icon("flag"));
            mtbtnTags->setToolTip(tr("Add tag...", "tbtn toolTip"));
            mtbtnTags->setEnabled(false);
          hlt->addWidget(mtbtnTags);
        flt->addRow(tr("Tags:", "lbl text"), hlt);
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
            QList<TSampleInfo::Type> types;
            types << TSampleInfo::Unverified;
            types << TSampleInfo::Approved;
            types << TSampleInfo::Rejected;
            foreach (const TSampleInfo::Type &t, types)
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
            mlstwgtAuthors = new QListWidget;
              mlstwgtAuthors->setEditTriggers(QListWidget::EditKeyPressed | QListWidget::DoubleClicked);
            hltw->addWidget(mlstwgtAuthors);
            QVBoxLayout *vltw = new QVBoxLayout;
              mtbtnAdd = new QToolButton;
                mtbtnAdd->setMenu(new QMenu);
                mtbtnAdd->setPopupMode(QToolButton::InstantPopup);
                mtbtnAdd->setEnabled(false);
                mtbtnAdd->setIcon(Application::icon("edit_add"));
                mtbtnAdd->setToolTip(tr("Add author", "tbtnt toolTip"));
              vltw->addWidget(mtbtnAdd);
              QHBoxLayout *hltww = new QHBoxLayout;
                mtbtnRemove = new QToolButton;
                  mtbtnRemove->setEnabled(false);
                  mtbtnRemove->setIcon(Application::icon("editdelete"));
                  mtbtnRemove->setToolTip(tr("Remove selected author", "tbtnt toolTip"));
                  connect(mtbtnRemove, SIGNAL(clicked()), this, SLOT(removeAuthor()));
                hltww->addWidget(mtbtnRemove);
                mtbtnClear = new QToolButton;
                  mtbtnClear->setEnabled(false);
                  mtbtnClear->setIcon(Application::icon("editclear"));
                  mtbtnClear->setToolTip(tr("Clear authors list", "tbtnt toolTip"));
                  connect(mtbtnClear, SIGNAL(clicked()), this, SLOT(clearAuthors()));
                hltww->addWidget(mtbtnClear);
              vltw->addLayout(hltww);
              hltww = new QHBoxLayout;
                mtbtnUp = new QToolButton;
                  mtbtnUp->setEnabled(ShowMode != mmode);
                  mtbtnUp->setIcon(Application::icon("1uparrow"));
                  mtbtnUp->setToolTip(tr("Move up", "tbtnt toolTip"));
                  connect(mtbtnUp, SIGNAL(clicked()), this, SLOT(authorUp()));
                hltww->addWidget(mtbtnUp);
                mtbtnDown = new QToolButton;
                  mtbtnDown->setEnabled(ShowMode != mmode);
                  mtbtnDown->setIcon(Application::icon("1downarrow"));
                  mtbtnDown->setToolTip(tr("Move down", "tbtnt toolTip"));
                  connect(mtbtnDown, SIGNAL(clicked()), this, SLOT(authorDown()));
                hltww->addWidget(mtbtnDown);
              vltw->addLayout(hltww);
            hltw->addLayout(vltw);
        hlt->addWidget(gbox);
      vlt->addLayout(hlt);
      hlt = new QHBoxLayout;
        gbox = new QGroupBox(tr("Comment", "gbox title"));
          vltw = new QVBoxLayout(gbox);
            mptedtComment = new QPlainTextEdit;
              mptedtComment->setReadOnly(ShowMode == mmode);
            vltw->addWidget(mptedtComment);
        hlt->addWidget(gbox);
        gbox = new QGroupBox(tr("Moderator remark", "gbox title"));
          vltw = new QVBoxLayout(gbox);
            mptedtRemark = new QPlainTextEdit;
              mptedtRemark->setReadOnly(EditMode != mmode);
            vltw->addWidget(mptedtRemark);
        hlt->addWidget(gbox);
      vlt->addLayout(hlt);
    //
    Application::setRowVisible(msboxRating, AddMode != mmode);
    Application::setRowVisible(mcmboxType, AddMode != mmode);
    Application::setRowVisible(mlblSender, AddMode != mmode);
    Application::setRowVisible(mlblCreationDT, AddMode != mmode);
    Application::setRowVisible(mlblUpdateDT, AddMode != mmode);
    checkInputs();
    if (ShowMode != mmode)
    {
        connect(mlstwgtAuthors, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(addAuthor()));
        connect(mlstwgtAuthors, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
                this, SLOT(lstwgtCurrentItemChanged(QListWidgetItem *)));
        addAuthor();
        lstwgtCurrentItemChanged(mlstwgtAuthors->currentItem());
    }
}

void SampleWidget::setAuthors(const QStringList &list)
{
    mlstwgtAuthors->clear();
    foreach (const QString &s, list)
        addAuthor(s);
    if (ShowMode != mmode)
        addAuthor();
}

QStringList SampleWidget::authors() const
{
    QStringList sl;
    foreach (QListWidgetItem *lwi, mlstwgtAuthors->findItems("*", Qt::MatchWrap | Qt::MatchWildcard))
        sl << lwi->text();
    sl.removeAll("");
    sl.removeDuplicates();
    return sl;
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

void SampleWidget::lstwgtCurrentItemChanged(QListWidgetItem *current)
{
    mtbtnRemove->setEnabled(current && current != mlstwgtAuthors->item(mlstwgtAuthors->count() - 1));
    mtbtnClear->setEnabled(mlstwgtAuthors->count() > 1);
    mtbtnUp->setEnabled(current && current != mlstwgtAuthors->item(mlstwgtAuthors->count() - 1)
            && current != mlstwgtAuthors->item(0));
    mtbtnDown->setEnabled(current && current != mlstwgtAuthors->item(mlstwgtAuthors->count() - 1)
            && current != mlstwgtAuthors->item(mlstwgtAuthors->count() - 2));
}

void SampleWidget::addAuthor(const QString &s)
{
    if (s.isEmpty())
    {
        QListWidgetItem *lwi = mlstwgtAuthors->item(mlstwgtAuthors->count() - 1);
        if (lwi && lwi->text().isEmpty())
            return;
    }
    if (!s.isEmpty() && authors().contains(s))
        return;
    QListWidgetItem *lwi = new QListWidgetItem(s);
    if (ShowMode != mmode)
        lwi->setFlags(lwi->flags () | Qt::ItemIsEditable);
    int ind = mlstwgtAuthors->count();
    mlstwgtAuthors->insertItem(s.isEmpty() ? ind : ind - 1, lwi);
    if (ShowMode != mmode && s.isEmpty())
        mlstwgtAuthors->setCurrentItem(lwi);
}

void SampleWidget::removeAuthor()
{
    delete mlstwgtAuthors->takeItem(mlstwgtAuthors->currentRow());
}

void SampleWidget::clearAuthors()
{
    mlstwgtAuthors->clear();
    if (ShowMode != mmode)
        addAuthor();
}

void SampleWidget::authorUp()
{
    int ind = mlstwgtAuthors->currentRow();
    if (!ind)
        return;
    QListWidgetItem *lwi = mlstwgtAuthors->takeItem(ind);
    if (!lwi)
        return;
    mlstwgtAuthors->insertItem(ind - 1, lwi);
    mlstwgtAuthors->setCurrentItem(lwi);
}

void SampleWidget::authorDown()
{
    int ind = mlstwgtAuthors->currentRow();
    if (ind == mlstwgtAuthors->count() - 1)
        return;
    QListWidgetItem *lwi = mlstwgtAuthors->takeItem(ind);
    if (!lwi)
        return;
    mlstwgtAuthors->insertItem(ind + 1, lwi);
    mlstwgtAuthors->setCurrentItem(lwi);
}

void SampleWidget::checkInputs()
{
    bool v = info().isValid() && (!mcheckSource || (mledtFileName->hasAcceptableInput()
                                                    && (!mactualFileName.isEmpty() || mdoc)));
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
    UserWidget *uwgt = new UserWidget(UserWidget::ShowMode);
    uwgt->setInfo(info);
    dlg.setWidget(uwgt);
    dlg.addButton(QDialogButtonBox::Close, SLOT(close()));
    dlg.setFixedSize(dlg.sizeHint());
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

void SampleWidget::addTag(const QString &tag)
{
    QStringList tags = TSampleInfo::listFromString(mledtTags->text());
    if (!tags.contains(tag, Qt::CaseInsensitive))
        tags << tag;
    mledtTags->setText(TSampleInfo::listToString(tags));
}

void SampleWidget::setFile(const QString &fn, QTextCodec *codec)
{
    QFileInfo fi(fn);
    mcodec = codec;
    if (fi.isAbsolute() && fi.isFile())
    {
        mactualFileName = fn;
        setProjectSize(TProject::size(mactualFileName, codec, true));
        mledtFileName->setText(createFileName(mactualFileName));
    }
    else
    {
        mactualFileName.clear();
        setProjectSize();
        mledtFileName->setText(createFileName(fn));
    }
    checkInputs();
}

/*============================== Static private methods ====================*/

QString SampleWidget::createFileName(const QString &fn)
{
    return !fn.isEmpty() ? (QFileInfo(fn).baseName() + ".tex") : QString();
}

/*============================== Static private constants ==================*/

const QString SampleWidget::DateTimeFormat = "dd MMMM yyyy hh:mm";
