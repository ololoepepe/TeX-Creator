#include "samplewidget.h"
#include "client.h"
#include "application.h"
#include "userwidget.h"
#include "cache.h"

#include <TSampleInfo>
#include <TCompilationResult>
#include <TUserInfo>

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

#include <QDebug>

/*============================================================================
================================ UserWidget ==================================
============================================================================*/

/*============================== Public constructors =======================*/

SampleWidget::SampleWidget(Mode m, QWidget *parent) :
    QWidget(parent), mmode(m)
{
    mvalid = false;
    mid = 0;
    Qt::TextInteractionFlags tiflags = Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse
            | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
    QHBoxLayout *hlt = new QHBoxLayout(this);
      QVBoxLayout *vlt = new QVBoxLayout;
        QFormLayout *flt = new QFormLayout;
        if (ShowMode != m)
        {
            mledtTitle = new QLineEdit;
              connect(mledtTitle, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
            flt->addRow(tr("Title:", "lbl text"), mledtTitle);
            mledtFileName = new QLineEdit;
              connect(mledtFileName, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
            flt->addRow(tr("File name:"), mledtFileName);
        }
        else
        {
            mlblTitle = new QLabel;
              mlblTitle->setTextInteractionFlags(tiflags);
            flt->addRow(tr("Title:", "lbl text"), mlblTitle);
            mlblFileName = new QLabel;
              mlblFileName->setTextInteractionFlags(tiflags);
            flt->addRow(tr("File name:"), mlblFileName);
        }
        if (AddMode != m)
        {
            mlblAuthor = new QLabel;
              mlblAuthor->setTextInteractionFlags(tiflags);
              connect(mlblAuthor, SIGNAL(linkActivated(QString)), this, SLOT(showAuthorInfo(QString)));
            flt->addRow(tr("Author:", "lbl text"), mlblAuthor);
        }
        if (EditMode == m)
        {
            mcmboxType = new QComboBox;
              foreach (const TSampleInfo::Type &t, QList<TSampleInfo::Type>() << TSampleInfo::Unverified
                       << TSampleInfo::Approved << TSampleInfo::Rejected)
                  mcmboxType->addItem(TSampleInfo::typeToString(t), t);
            flt->addRow(tr("Type:", "lbl text"), mcmboxType);
            msboxRating = new QSpinBox;
              msboxRating->setMinimum(0);
              msboxRating->setMaximum(100);
              msboxRating->setValue(0);
            flt->addRow(tr("Rating:", "lbl text"), msboxRating);
        }
        else if (AddMode != m)
        {
            mlblType = new QLabel;
              mlblType->setTextInteractionFlags(tiflags);
            flt->addRow(tr("Type:", "lbl text"), mlblType);
            mlblRating = new QLabel;
              mlblRating->setTextInteractionFlags(tiflags);
            flt->addRow(tr("Rating:", "lbl text"), mlblRating);
        }
        vlt->addLayout(flt);
        QLabel *lbl = new QLabel(tr("Other authors:", "lbl text"));
        vlt->addWidget(lbl);
        mptedtExtraAuthors = new QPlainTextEdit;
          mptedtExtraAuthors->setFixedHeight(50);
          mptedtExtraAuthors->setReadOnly(ShowMode == m);
        vlt->addWidget(mptedtExtraAuthors);
        lbl = new QLabel(tr("Tags:", "lbl text"));
        vlt->addWidget(lbl);
        mptedtTags = new QPlainTextEdit;
          mptedtTags->setFixedHeight(50);
          mptedtTags->setReadOnly(ShowMode == m);
        vlt->addWidget(mptedtTags);
      hlt->addLayout(vlt);
      vlt = new QVBoxLayout;
        lbl = new QLabel(tr("Comment:", "lbl text"));
        vlt->addWidget(lbl);
        mptedtComment = new QPlainTextEdit;
          mptedtComment->setMaximumHeight(100);
          mptedtComment->setReadOnly(ShowMode == m);
        vlt->addWidget(mptedtComment);
        if (AddMode != m)
        {
            lbl = new QLabel(tr("Admin remark:", "lbl text"));
            vlt->addWidget(lbl);
            mptedtRemark = new QPlainTextEdit;
              mptedtRemark->setMaximumHeight(100);
              mptedtRemark->setReadOnly(EditMode != m);
            vlt->addWidget(mptedtRemark);
        }
      hlt->addLayout(vlt);
    //
    checkInputs();
}

/*============================== Public methods ============================*/

void SampleWidget::setInfo(const TSampleInfo &info)
{

    if (info.isValid())
    {
        mid = info.id();
        if (ShowMode != mmode)
        {
            mledtTitle->setText(info.title());
            mledtFileName->setText(info.fileName());
        }
        else
        {
            mlblTitle->setText(info.title());
            mlblFileName->setText(info.fileName());
        }
        if (AddMode != mmode)
        {
            QString s = "<a href=\"" + info.author().idString() + "\">" + info.author().login() + "</a>";
            if (!info.author().realName().isEmpty())
                s += " (" + info.author().realName() + ")";
            mlblAuthor->setText(s);
            mlblAuthor->setProperty("author_id", info.author().id());
        }
        if (EditMode == mmode)
        {
            mcmboxType->setCurrentIndex(mcmboxType->findData(info.type()));
            msboxRating->setValue(info.rating());
        }
        else if (AddMode != mmode)
        {
            mlblType->setText(info.typeString());
            mlblRating->setText(info.ratingString());
        }
        mptedtExtraAuthors->setPlainText(info.extraAuthorsString());
        mptedtTags->setPlainText(info.tagsString());
        mptedtComment->setPlainText(info.comment());
        if (AddMode != mmode)
            mptedtRemark->setPlainText(info.adminRemark());
        if (ShowMode == mmode)
            checkInputs();
    }
    else
    {
        mid = 0;
        if (ShowMode != mmode)
        {
            mledtTitle->clear();
            mledtFileName->clear();
        }
        else
        {
            mlblTitle->clear();
            mlblFileName->clear();
        }
        if (AddMode != mmode)
            mlblAuthor->clear();
        if (EditMode == mmode)
        {
            mcmboxType->setCurrentIndex(0);
            msboxRating->setValue(0);
        }
        else if (AddMode != mmode)
        {
            mlblType->setText(TSampleInfo::typeToString(TSampleInfo::Unverified));
            mlblRating->setText("0");
        }
        mptedtExtraAuthors->clear();
        mptedtTags->clear();
        mptedtComment->clear();
        if (AddMode != mmode)
            mptedtRemark->clear();
    }
}

void SampleWidget::setFileName(const QString &fn)
{
    QString fnn = QFileInfo(fn).fileName();
    if (ShowMode == mmode)
        mlblFileName->setText(fnn);
    else
        mledtFileName->setText(fnn);
}

SampleWidget::Mode SampleWidget::mode() const
{
    return mmode;
}

TSampleInfo SampleWidget::info() const
{
    TSampleInfo info;
    info.setId(mid);
    switch (mmode)
    {
    case AddMode:
        info.setContext(TSampleInfo::AddContext);
        info.setTitle(mledtTitle->text());
        info.setFileName(!mledtFileName->text().isEmpty() ? (QFileInfo(mledtFileName->text()).baseName() + ".tex") :
                                                            QString());
        info.setExtraAuthors(mptedtExtraAuthors->toPlainText());
        info.setTags(mptedtTags->toPlainText());
        info.setComment(mptedtComment->toPlainText());
        break;
    case EditMode:
        info.setContext(TSampleInfo::EditContext);
        info.setTitle(mledtTitle->text());
        info.setFileName(!mledtFileName->text().isEmpty() ? (QFileInfo(mledtFileName->text()).baseName() + ".tex") :
                                                            QString());
        info.setType(mcmboxType->itemData(mcmboxType->currentIndex()).toInt());
        info.setRating((quint8) msboxRating->value());
        info.setExtraAuthors(mptedtExtraAuthors->toPlainText());
        info.setTags(mptedtTags->toPlainText());
        info.setComment(mptedtComment->toPlainText());
        info.setAdminRemark(mptedtRemark->toPlainText());
        break;
    case UpdateMode:
        info.setContext(TSampleInfo::UpdateContext);
        info.setTitle(mledtTitle->text());
        info.setFileName(!mledtFileName->text().isEmpty() ? (QFileInfo(mledtFileName->text()).baseName() + ".tex") :
                                                            QString());
        info.setExtraAuthors(mptedtExtraAuthors->toPlainText());
        info.setTags(mptedtTags->toPlainText());
        info.setComment(mptedtComment->toPlainText());
        break;
    case ShowMode:
    {
        info.setContext(TSampleInfo::GeneralContext);
        info.setTitle(mlblTitle->text());
        info.setFileName(mlblFileName->text());
        TUserInfo author(TUserInfo::ShortInfoContext);
        author.setId(mlblAuthor->property("author_id").toULongLong());
        author.setLogin(mlblAuthor->text().remove(QRegExp("\\s+.*$")));
        info.setAuthor(author);
        info.setExtraAuthors(mptedtExtraAuthors->toPlainText());
        info.setTags(mptedtTags->toPlainText());
        info.setComment(mptedtComment->toPlainText());
        info.setAdminRemark(mptedtRemark->toPlainText());
        break;
    }
    default:
        break;
    }
    return info;
}

bool SampleWidget::isValid() const
{
    return mvalid;
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

void SampleWidget::showAuthorInfo(const QString &idString)
{
    quint64 id = idString.toULongLong();
    if (!id)
        return;
    TUserInfo info;
    if (sClient->isAuthorized())
    {
        if (!sClient->getUserInfo(id, info, this))
            return;
    }
    else
    {
        info = sCache->userInfo(id);
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
