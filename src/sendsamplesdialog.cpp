#include "sendsamplesdialog.h"
#include "client.h"
#include "application.h"

#include <BDirTools>
#include <BCodeEditorDocument>

#include <QDialog>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSplitter>
#include <QWidget>
#include <QList>
#include <QPlainTextEdit>
#include <QFileInfo>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include <QRegExp>
#include <QChar>
#include <QFormLayout>
#include <QMessageBox>

#include <QDebug>

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
    Client::SampleData data;
    data.title = mledtTitle->text();
    data.fileName = mledtFileName->text();
    data.text = lwi->data(TextRole).toString();
    data.initialFileName = lwi->data(InitialFileNameRole).toString();
    data.codec = QTextCodec::codecForName( lwi->data(CodecNameRole).toString().toLatin1() );
    data.tags = Sample::stringToTags( mledtTags->text() );
    data.comment = fromPlainText( mptedtComment->toPlainText() );
    QString errs;
    QString log;
    bool b = sClient->addSample(data, &errs, &log, this);
    if ( !log.isEmpty() )
    {
        lwi->setData(HasLogRole, true);
        lwi->setData(LogRole, log);
        mbtnShowLog->setEnabled(true);
    }
    if (b)
    {
        lwi->setData(SentRole, true);
        lwi->setIcon( Application::icon("ok") );
        if (mlstwgt->count() == 1)
            mbtnSend->setIcon( Application::icon("ok") );
        mledtTitle->setEnabled(false);
        mledtFileName->setEnabled(false);
        mledtTags->setEnabled(false);
        mptedtComment->setEnabled(false);
        mbtnSend->setEnabled(false);
    }
    else
    {
        lwi->setIcon( Application::icon("messagebox_critical") );
        if (mlstwgt->count() == 1)
            mbtnSend->setIcon( Application::icon("messagebox_critical") );
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Failed to send sample", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("The following error occured:", "msgbox text") );
        msg.setInformativeText(errs);
        if ( !log.isEmpty() )
            msg.setDetailedText(log);
        msg.setStandardButtons(QMessageBox::Ok);
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
        previous->setData( TagsRole, Sample::stringToTags( mledtTags->text() ) );
        previous->setData( CommentRole, fromPlainText( mptedtComment->toPlainText() ) );
    }
    if (current)
    {
        mledtTitle->setText( current->data(TitleRole).toString() );
        mledtFileName->setText( current->data(FileNameRole).toString() );
        mledtTags->setText( Sample::tagsToString( current->data(TagsRole).toStringList() ) );
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
