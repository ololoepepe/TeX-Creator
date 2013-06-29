#include "samplewidget.h"
#include "client.h"
#include "application.h"
#include "userwidget.h"
#include "cache.h"

#include <TSampleInfo>
#include <TCompilationResult>
#include <TUserInfo>
#include <BFlowLayout>

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

#include <QDebug>

/*============================================================================
================================ SampleWidget ================================
============================================================================*/

/*============================== Public constructors =======================*/

SampleWidget::SampleWidget(Mode m, QWidget *parent) :
    QWidget(parent), mmode(m)
{
    mvalid = false;
    mid = 0;
    Qt::TextInteractionFlags tiflags = Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse
            | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
    BFlowLayout *fwlt = new BFlowLayout(this);
      QGroupBox *gbox = new QGroupBox;
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
              mlblSender = new QLabel;
                mlblSender->setTextInteractionFlags(tiflags);
                connect(mlblSender, SIGNAL(linkActivated(QString)), this, SLOT(showSenderInfo(QString)));
              flt->addRow(tr("Sender:", "lbl text"), mlblSender);
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
        gbox->setLayout(flt);
        gbox->setMinimumWidth(400);
      fwlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Authors:", "gbox title"));
        QVBoxLayout *vlt = new QVBoxLayout;
          mptedtAuthors = new QPlainTextEdit;
            mptedtAuthors->setReadOnly(ShowMode == m);
            mptedtAuthors->setTabChangesFocus(true);
            if (ShowMode != m)
                mptedtAuthors->setToolTip(tr("Authors must be separated by commas or by end of line. "
                                             "Write additional authors information in parentheses", "ptedt toolTip"));
          vlt->addWidget(mptedtAuthors);
        gbox->setLayout(vlt);
        gbox->setMaximumHeight(150);
        gbox->setMinimumWidth(200);
      fwlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Tags:", "gbox title"));
        vlt = new QVBoxLayout;
          mptedtTags = new QPlainTextEdit;
            mptedtTags->setReadOnly(ShowMode == m);
            mptedtTags->setTabChangesFocus(true);
            if (ShowMode != m)
                mptedtTags->setToolTip(tr("Tags must be separated by commas or by end of line", "ptedt toolTip"));
          vlt->addWidget(mptedtTags);
        gbox->setLayout(vlt);
        gbox->setMaximumHeight(150);
        gbox->setMinimumWidth(200);
      fwlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Comment:", "gbox title"));
        vlt = new QVBoxLayout;
          mptedtComment = new QPlainTextEdit;
            mptedtComment->setReadOnly(ShowMode == m);
            mptedtComment->setTabChangesFocus(true);
          vlt->addWidget(mptedtComment);
        gbox->setLayout(vlt);
        gbox->setMaximumHeight(150);
        gbox->setMinimumWidth(400);
      fwlt->addWidget(gbox);
      if (AddMode != m)
      {
          gbox = new QGroupBox(tr("Admin remark:", "gbox title"));
            vlt = new QVBoxLayout;
              mptedtRemark = new QPlainTextEdit;
                mptedtRemark->setReadOnly(EditMode != m);
                mptedtRemark->setTabChangesFocus(true);
              vlt->addWidget(mptedtRemark);
            gbox->setLayout(vlt);
            gbox->setMaximumHeight(150);
            gbox->setMinimumWidth(400);
          fwlt->addWidget(gbox);
      }
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
            QString s = "<a href=\"" + info.sender().idString() + "\">" + info.sender().login() + "</a>";
            if (!info.sender().realName().isEmpty())
                s += " (" + info.sender().realName() + ")";
            mlblSender->setText(s);
            mlblSender->setProperty("sender_id", info.sender().id());
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
        mptedtAuthors->setPlainText(info.authorsString());
        mptedtTags->setPlainText(info.tagsString());
        mptedtComment->setPlainText(info.comment());
        if (AddMode != mmode)
            mptedtRemark->setPlainText(info.adminRemark());
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
            mlblSender->clear();
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
        mptedtAuthors->clear();
        mptedtTags->clear();
        mptedtComment->clear();
        if (AddMode != mmode)
            mptedtRemark->clear();
    }
    checkInputs();
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
        info.setAuthors(mptedtAuthors->toPlainText());
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
        info.setAuthors(mptedtAuthors->toPlainText());
        info.setTags(mptedtTags->toPlainText());
        info.setComment(mptedtComment->toPlainText());
        info.setAdminRemark(mptedtRemark->toPlainText());
        break;
    case UpdateMode:
        info.setContext(TSampleInfo::UpdateContext);
        info.setTitle(mledtTitle->text());
        info.setFileName(!mledtFileName->text().isEmpty() ? (QFileInfo(mledtFileName->text()).baseName() + ".tex") :
                                                            QString());
        info.setAuthors(mptedtAuthors->toPlainText());
        info.setTags(mptedtTags->toPlainText());
        info.setComment(mptedtComment->toPlainText());
        break;
    case ShowMode:
    {
        info.setContext(TSampleInfo::GeneralContext);
        info.setTitle(mlblTitle->text());
        info.setFileName(mlblFileName->text());
        TUserInfo sender(TUserInfo::ShortInfoContext);
        sender.setId(mlblSender->property("sender_id").toULongLong());
        sender.setLogin(mlblSender->text().remove(QRegExp("\\s+.*$")));
        info.setSender(sender);
        info.setAuthors(mptedtAuthors->toPlainText());
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

void SampleWidget::showSenderInfo(const QString &idString)
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
