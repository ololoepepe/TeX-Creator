#include "samplewidget.h"
#include "client.h"
#include "application.h"
#include "userwidget.h"
#include "cache.h"

#include <TSampleInfo>
#include <TCompilationResult>
#include <TUserInfo>

#include <BDirTools>
#include <BCodeEditorDocument>

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

#include <QFileDialog>

#include <QListWidget>
#include <QListWidgetItem>
#include <QSplitter>
#include <QFileInfo>
#include <QVariantMap>
#include <QVariantList>
#include <QRegExp>
#include <QChar>

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
        lbl = new QLabel(tr("Admin remark:", "lbl text"));
        vlt->addWidget(lbl);
        mptedtRemark = new QPlainTextEdit;
          mptedtRemark->setMaximumHeight(100);
          mptedtRemark->setReadOnly(EditMode != m);
        vlt->addWidget(mptedtRemark);
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
            QString s = "<a href=\"" + info.idString() + "\">" + info.author().login() + "</a>";
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
        info.setContext(TUserInfo::AddContext);
        info.setTitle(mledtTitle->text());
        info.setFileName(mledtFileName->text());
        info.setExtraAuthors(mptedtExtraAuthors->toPlainText());
        info.setTags(mptedtTags->toPlainText());
        info.setComment(mptedtComment->toPlainText());
        break;
    case EditMode:
        info.setContext(TUserInfo::EditContext);
        info.setTitle(mledtTitle->text());
        info.setFileName(mledtFileName->text());
        info.setType(mcmboxType->itemData(mcmboxType->currentIndex()).toInt());
        info.setRating((quint8) msboxRating->value());
        info.setExtraAuthors(mptedtExtraAuthors->toPlainText());
        info.setTags(mptedtTags->toPlainText());
        info.setComment(mptedtComment->toPlainText());
        info.setAdminRemark(mptedtRemark->toPlainText());
        break;
    case UpdateMode:
        info.setContext(TUserInfo::UpdateContext);
        info.setTitle(mledtTitle->text());
        info.setFileName(mledtFileName->text());
        info.setExtraAuthors(mptedtExtraAuthors->toPlainText());
        info.setTags(mptedtTags->toPlainText());
        info.setComment(mptedtComment->toPlainText());
        break;
    case ShowMode:
    {
        info.setContext(TUserInfo::GeneralContext);
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

/*============================================================================
================================ LogDialog ===================================
============================================================================*/

class LogDialog : public QDialog
{
public:
    explicit LogDialog(const QString &log, QWidget *parent = 0);
};

/*============================================================================
================================ LogDialog ===================================
============================================================================*/

/*============================== Public constructors =======================*/

LogDialog::LogDialog(const QString &log, QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QPlainTextEdit *ptedt = new QPlainTextEdit(this);
        ptedt->setReadOnly(true);
        ptedt->setPlainText(log);
      vlt->addWidget(ptedt);
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        connect( dlgbbox->addButton(QDialogButtonBox::Close), SIGNAL( clicked() ), this, SLOT( close() ) );
      vlt->addWidget(dlgbbox);
}

/*============================================================================
================================ SendSamplesDialog ===========================
============================================================================*/

/*============================== Public constructors =======================*/

SendSamplesDialog::SendSamplesDialog(BCodeEditorDocument *doc, QWidget *parent) :
    QDialog(parent)
{
    QList<QListWidgetItem *> list;
    QString fn = doc ? doc->fileName() : QString();
    QString text = doc ? doc->text() : QString();
    if ( !fn.isEmpty() && !text.isEmpty() )
        list << createItem(fn, doc->codec(), text);
    init(list);
}

SendSamplesDialog::SendSamplesDialog(const QList<BCodeEditorDocument *> &docs, QWidget *parent) :
    QDialog(parent)
{
    QList<QListWidgetItem *> list;
    foreach (BCodeEditorDocument *doc, docs)
    {
        QString fn = doc ? doc->fileName() : QString();
        QString text = doc ? doc->text() : QString();
        if ( !fn.isEmpty() && !text.isEmpty() )
            list << createItem(fn, doc->codec(), text);
    }
    init(list);
}

SendSamplesDialog::SendSamplesDialog(const QStringList &fileNames, QTextCodec *codec, QWidget *parent) :
    QDialog(parent)
{
    QList<QListWidgetItem *> list;
    foreach (const QString &fn, fileNames)
        if ( !fn.isEmpty() )
            list << createItem(fn, codec);
    init(list);
}

/*============================== Static private methods ====================*/

QListWidgetItem *SendSamplesDialog::createItem(const QString &fileName, QTextCodec *codec, const QString &text)
{
    QString fn = QFileInfo(fileName).fileName();
    QListWidgetItem *lwi = new QListWidgetItem(fn);
    if ( !fileName.isEmpty() )
    {
        lwi->setData(InitialFileNameRole, fileName);
        lwi->setData(FileNameRole, fn);
    }
    if (codec)
        lwi->setData( CodecNameRole, QString( codec->name() ) );
    if ( !text.isEmpty() )
        lwi->setData(TextRole, text);
    return lwi;
}

QString SendSamplesDialog::fromPlainText(const QString &text)
{
    QString ntext = text;
    return ntext.replace(QChar::ParagraphSeparator, '\n');
}

/*============================== Private methods ===========================*/

void SendSamplesDialog::init(const QList<QListWidgetItem *> &items)
{
    setWindowTitle( tr("Sending samples", "windowTitle") );
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QSplitter *hspltr = new QSplitter(Qt::Horizontal, this);
        mlstwgt = new QListWidget;
          connect( mlstwgt, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem *) ),
                   this, SLOT( lstwgtCurrentItemChanged(QListWidgetItem *, QListWidgetItem *) ) );
          connect( mlstwgt, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem *) ),
                   this, SLOT( checkSendAvailable() ) );
        hspltr->addWidget(mlstwgt);
        QWidget *wgt = new QWidget;
          QVBoxLayout *vltw = new QVBoxLayout(wgt);
            QFormLayout *flt = new QFormLayout;
              mledtTitle = new QLineEdit(wgt);
                mledtTitle->setMaxLength(128);
                connect( mledtTitle, SIGNAL( textEdited(QString) ), this, SLOT( ledtTitleTextEdited(QString) ) );
                connect( mledtTitle, SIGNAL( textChanged(QString) ), this, SLOT( checkSendAvailable() ) );
              flt->addRow(tr("Title:", "lbl text"), mledtTitle);
              mledtFileName = new QLineEdit(wgt);
                mledtFileName->setMaxLength(128);
                connect( mledtFileName, SIGNAL( textChanged(QString) ), this, SLOT( checkSendAvailable() ) );
              flt->addRow(tr("File name:", "lbl text"), mledtFileName);
              mledtTags = new QLineEdit(wgt);
              flt->addRow(tr("Tags:", "lbl text"), mledtTags);
            vltw->addLayout(flt);
            vltw->addWidget( new QLabel(tr("Comment:", "lbl text"), wgt) );
            mptedtComment = new QPlainTextEdit(wgt);
              mptedtComment->setTabChangesFocus(true);
            vltw->addWidget(mptedtComment);
        hspltr->addWidget(wgt);
        hspltr->setSizes(QList<int>() << 200 << 300);
      vlt->addWidget(hspltr);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        dlgbbox->addButton(QDialogButtonBox::Close);
          connect( dlgbbox->button(QDialogButtonBox::Close), SIGNAL( clicked() ), this, SLOT( reject() ) );
        mbtnShowLog = dlgbbox->addButton(tr("Show log", "btn text"), QDialogButtonBox::ActionRole);
          mbtnShowLog->setEnabled(false);
          connect( mbtnShowLog, SIGNAL( clicked() ), this, SLOT( showLog() ) );
        mbtnSend = dlgbbox->addButton(tr("Send", "btn text"), QDialogButtonBox::ActionRole);
          mbtnSend->setDefault(true);
          mbtnSend->setEnabled(false);
          connect( mbtnSend, SIGNAL( clicked() ), this, SLOT( send() ) );
      vlt->addWidget(dlgbbox);
    //
    foreach (QListWidgetItem *lwi, items)
        mlstwgt->addItem(lwi);
    if ( mlstwgt->count() )
        mlstwgt->setCurrentRow(0);
    //
    bool b = (mlstwgt->count() <= 1);
    if (b)
        mlstwgt->hide();
    resize(b ? 300 : 500, 250);
}

/*============================== Private slots =============================*/

void SendSamplesDialog::send()
{
    QListWidgetItem *lwi = mlstwgt->currentItem();
    if ( !lwi || lwi->data(SentRole).toBool() )
        return;
    QString fn = lwi->data(InitialFileNameRole).toString();
    QString text = lwi->data(TextRole).toString();
    QTextCodec *codec = QTextCodec::codecForName(lwi->data(CodecNameRole).toString().toLatin1());
    TSampleInfo info(TSampleInfo::AddContext);
    info.setTitle(mledtTitle->text());
    info.setFileName(QFileInfo(mledtFileName->text()).baseName() + ".tex");
    info.setTags(mledtTags->text());
    info.setComment(mptedtComment->toPlainText());
    TCompilationResult r = sClient->addSample(fn, codec, text, info, this);
    if (!r.log().isEmpty())
    {
        lwi->setData(HasLogRole, true);
        lwi->setData(LogRole, r.log());
        mbtnShowLog->setEnabled(true);
    }
    if (r.success())
    {
        lwi->setData(SentRole, true);
        lwi->setIcon(Application::icon("ok"));
        if (mlstwgt->count() == 1)
            mbtnSend->setIcon(Application::icon("ok"));
        mledtTitle->setEnabled(false);
        mledtFileName->setEnabled(false);
        mledtTags->setEnabled(false);
        mptedtComment->setEnabled(false);
        mbtnSend->setEnabled(false);
    }
    else
    {
        lwi->setIcon(Application::icon("messagebox_critical"));
        if (mlstwgt->count() == 1)
            mbtnSend->setIcon(Application::icon("messagebox_critical"));
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Failed to send sample", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("The following error occured:", "msgbox text"));
        msg.setInformativeText(r.errorString());
        msg.setStandardButtons(QMessageBox::Ok);
        if (!r.log().isEmpty())
        {
            QPushButton *btn = msg.addButton(tr("Show log", "btn text"), QMessageBox::AcceptRole);
            msg.setDefaultButton(btn);
            connect(btn, SIGNAL(clicked()), this, SLOT(showLog()));
        }
        else
        {
            msg.setDefaultButton(QMessageBox::Ok);
        }
        msg.exec();
    }
}

void SendSamplesDialog::showLog()
{
    QListWidgetItem *lwi = mlstwgt->currentItem();
    if (!lwi)
        return;
    LogDialog(lwi->data(LogRole).toString(), this).exec();
}

void SendSamplesDialog::lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (previous)
    {
        previous->setData( TitleRole, mledtTitle->text() );
        previous->setData( FileNameRole, mledtFileName->text() );
        previous->setData(TagsRole, TSampleInfo::listFromString(mledtTags->text()));
        previous->setData( CommentRole, fromPlainText( mptedtComment->toPlainText() ) );
    }
    if (current)
    {
        mledtTitle->setText( current->data(TitleRole).toString() );
        mledtFileName->setText( current->data(FileNameRole).toString() );
        mledtTags->setText(TSampleInfo::listToString(current->data(TagsRole).toStringList()));
        mptedtComment->setPlainText( current->data(CommentRole).toString() );
    }
    else
    {
        mledtTitle->clear();
        mledtFileName->clear();
        mledtTags->clear();
        mptedtComment->clear();
    }
    bool b = current && !current->data(SentRole).toBool();
    mledtTitle->setEnabled(b);
    mledtFileName->setEnabled(b);
    mledtTags->setEnabled(b);
    mptedtComment->setEnabled(b);
    mbtnShowLog->setEnabled( current && current->data(HasLogRole).toBool() );
}

void SendSamplesDialog::ledtTitleTextEdited(const QString &text)
{
    QListWidgetItem *lwi = mlstwgt->currentItem();
    if (!lwi)
        return;
    QString t = !text.isEmpty() ? text : mledtFileName->text();
    if ( t.isEmpty() )
        return;
    lwi->setText(t);
}

void SendSamplesDialog::checkSendAvailable()
{
    QListWidgetItem *lwi = mlstwgt->currentItem();
    mbtnSend->setEnabled( lwi && !lwi->data(SentRole).toBool() && !mledtTitle->text().isEmpty() &&
                          !mledtFileName->text().isEmpty() );
}
