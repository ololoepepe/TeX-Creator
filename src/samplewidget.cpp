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

#include <QDebug>

/*============================================================================
================================ SampleWidget ================================
============================================================================*/

/*============================== Public constructors =======================*/

SampleWidget::SampleWidget(Mode m, QWidget *parent) :
    QWidget(parent), mmode(m), mdoc(0)
{
    init();
}

SampleWidget::SampleWidget(Mode m, BAbstractCodeEditorDocument *currentDocument, QWidget *parent) :
    QWidget(parent), mmode(m), mdoc(currentDocument)
{
    init();
}

/*============================== Public methods ============================*/

void SampleWidget::setInfo(const TSampleInfo &info)
{
    if (info.isValid())
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
        mlblSender->setText("<a href=x>" + msenderLogin + "</a>" + (!msenderRealName.isEmpty() ?
                                                                        (" (" + msenderRealName + ")") : QString()));
        mlblCreationDT->setText(info.creationDateTime(Qt::LocalTime).toString(DateTimeFormat));
        mlblUpdateDT->setText(info.updateDateTime(Qt::LocalTime).toString(DateTimeFormat));
        //TODO: Authors
        mptedtComment->setPlainText(info.comment());
        mptedtRemark->setPlainText(info.adminRemark());
    }
    else
    {
        mid = 0;
        msenderId = 0;
        mprojectSize = 0;
        setProjectSize();
        msenderLogin.clear();
        msenderRealName.clear();
        mledtTitle->clear();
        mledtFileName->clear();
        mledtTags->clear();
        msboxRating->setValue(0);
        mcmboxType->setCurrentIndex(mcmboxType->findData(TSampleInfo::Unverified));
        mlblSender->setText("");
        mlblCreationDT->clear();
        mlblUpdateDT->clear();
        //TODO: Authors
        mptedtComment->clear();
        mptedtRemark->clear();
    }
    checkInputs();
}

void SampleWidget::setActualFileName(const QString &fn)
{
    //
}

void SampleWidget::setFileName(const QString &fn)
{
    mledtFileName->setText(createFileName(fn));
}

void SampleWidget::restoreState(const QByteArray &state, bool full)
{
    //
}

SampleWidget::Mode SampleWidget::mode() const
{
    return mmode;
}

TSampleInfo SampleWidget::info() const
{
    TSampleInfo info;
    info.setId(mid);
    TUserInfo u(msenderId, TUserInfo::ShortInfoContext);
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
    //TODO: Authors
    info.setComment(mptedtComment->toPlainText().replace(QChar::ParagraphSeparator, '\n'));
    info.setAdminRemark(mptedtRemark->toPlainText().replace(QChar::ParagraphSeparator, '\n'));
    return info;
}

QString SampleWidget::actualFileName() const
{
    return mactualFileName;
}

QByteArray SampleWidget::saveState(bool full) const
{
    //
}

bool SampleWidget::isValid() const
{
    return mvalid;
}

/*============================== Private methods ===========================*/

void SampleWidget::init()
{
    mvalid = false;
    mid = 0;
    msenderId = 0;
    mprojectSize = 0;
    Qt::TextInteractionFlags tiflags = Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse
            | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QFormLayout *flt = new QFormLayout;
        QHBoxLayout *hlt = new QHBoxLayout;
          mledtTitle = new QLineEdit;
            mledtTitle->setReadOnly(ShowMode == mmode);
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
            connect(mledtFileName, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
          hlt->addWidget(mledtFileName);
          mlblSize = new QLabel;
            mlblSize->setTextInteractionFlags(tiflags);
            setProjectSize();
          hlt->addWidget(mlblSize);
          tbtn = new QToolButton;
            tbtn->setIcon(Application::icon("tex"));
            tbtn->setToolTip(tr("Use current document", "tbtn toolTip"));
            tbtn->setEnabled(ShowMode != mmode && mdoc);
            connect(tbtn, SIGNAL(clicked()), this, SLOT(useCurrentDocument()));
          hlt->addWidget(tbtn);
          tbtn = new QToolButton;
            tbtn->setIcon(Application::icon("fileopen"));
            tbtn->setToolTip(tr("Use external file...", "tbtn toolTip"));
            tbtn->setEnabled(ShowMode != mmode);
            connect(tbtn, SIGNAL(clicked()), this, SLOT(useExternalFile()));
          hlt->addWidget(tbtn);
        flt->addRow(tr("File name:", "lbl text"), hlt);
        hlt = new QHBoxLayout;
          mledtTags = new QLineEdit;
            mledtTags->setReadOnly(ShowMode == mmode);
          hlt->addWidget(mledtTags);
          tbtn = new QToolButton;
            tbtn->setPopupMode(QToolButton::InstantPopup);
            //tbtn->setIcon(Application::icon("tex")); //TODO
            tbtn->setToolTip(tr("Add tag...", "tbtn toolTip"));
            tbtn->setEnabled(ShowMode != mmode);
          hlt->addWidget(tbtn);
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
          flt->addRow(tr("Creation time:", "lbl text"), mlblCreationDT);
          mlblUpdateDT = new QLabel;
            mlblUpdateDT->setTextInteractionFlags(tiflags);
          flt->addRow(tr("Update time:", "lbl text"), mlblUpdateDT);
        hlt->addLayout(flt);
        //TODO: Authors
        mlstwgtAuthors = new QListWidget;
        hlt->addWidget(mlstwgtAuthors);
      vlt->addLayout(hlt);
      hlt = new QHBoxLayout;
        QGroupBox *gbox = new QGroupBox(tr("Comment", "gbox title"));
          QVBoxLayout *vltw = new QVBoxLayout(gbox);
            mptedtComment = new QPlainTextEdit;
              mptedtComment->setReadOnly(ShowMode == mmode);
            vltw->addWidget(mptedtComment);
        hlt->addWidget(gbox);
        gbox = new QGroupBox(tr("Admin remark", "gbox title"));
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
}

void SampleWidget::setProjectSize(int sz)
{
    if (sz < 0)
        sz = 0;
    mprojectSize = sz;
    QString s = tr("Size:", "lbl text") + " ";
    if (sz)
        s += TSampleInfo::projectSizeToString(sz, TSampleInfo::KilobytesFormat);
    else
        s += tr("Unknown", "lbl text");
    mlblSize->setText(s);
}

/*============================== Private slots =============================*/

void SampleWidget::checkInputs()
{
    bool v = info().isValid();
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
    QDialog dlg(this);
    dlg.setWindowTitle(tr("User:", "windowTitle") + " " + info.login());
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      UserWidget *uwgt = new UserWidget(UserWidget::ShowMode);
        uwgt->setInfo(info);
      vlt->addWidget(uwgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Close);
        connect(dlgbbox->button(QDialogButtonBox::Close), SIGNAL(clicked()), &dlg, SLOT(close()));
      vlt->addWidget(dlgbbox);
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

void SampleWidget::useCurrentDocument()
{
    //
}

void SampleWidget::useExternalFile()
{
    //
}

void SampleWidget::addTag()
{
    //
}

/*============================== Static private methods ====================*/

QString SampleWidget::createFileName(const QString &fn)
{
    return !fn.isEmpty() ? (QFileInfo(fn).baseName() + ".tex") : QString();
}

/*============================== Static private constants ==================*/

const QString SampleWidget::DateTimeFormat = "dd MMMM yyyy hh:mm";
