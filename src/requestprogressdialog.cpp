#include "requestprogressdialog.h"

#include <BNetworkOperation>

#include <QDialog>
#include <QCloseEvent>
#include <QPointer>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QDialogButtonBox>
#include <QPushButton>

/*============================================================================
================================ RequestProgressDialog =======================
============================================================================*/

/*============================== Public constructors =======================*/

RequestProgressDialog::RequestProgressDialog(BNetworkOperation *request, QWidget *parent) :
    QDialog(parent), Request(request)
{
    setWindowTitle( tr("Request progress", "windowTitle") );
    muploadText = tr("Sending request...", "lbl text");
    mdownloadText = tr("Receiving reply...", "lbl text");
    muploadCompleted = false;
    mfinished = false;
    //
    QVBoxLayout *vlt = new QVBoxLayout(this);
      mlbl = new QLabel(this);
      vlt->addWidget(mlbl);
      mpbar = new QProgressBar(this);
        mpbar->setMinimum(0);
        mpbar->setMaximum(0);
        mpbar->setValue(0);
      vlt->addWidget(mpbar);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        QPushButton *btn = dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(btn, SIGNAL(clicked()), request, SLOT(cancel));
      vlt->addWidget(dlgbbox);
    //
    if ( isValid() )
    {
        mlbl->setText(muploadText);
        uploadProgress( Request->uploadBytesReady(), Request->uploadBytesTotal() );
        if (muploadCompleted)
            downloadProgress( Request->downloadBytesReady(), Request->downloadBytesTotal() );
        if ( Request->isFinished() || Request->isError() )
            QTimer::singleShot( 0, this, SLOT( opFinished() ) );
        connect( Request.data(), SIGNAL( uploadProgress(int, int) ), this, SLOT( uploadProgress(int, int) ) );
        connect( Request.data(), SIGNAL( downloadProgress(int, int) ), this, SLOT( downloadProgress(int, int) ) );
        connect( Request.data(), SIGNAL( finished() ), this, SLOT( opFinished() ) );
        connect( Request.data(), SIGNAL( error() ), this, SLOT( opFinished() ) );
    }
    else
    {
        btn->setEnabled(false);
        mlbl->setText( tr("Invalid operation", "lbl text") );
        mfinished = true;
        QTimer::singleShot( 0, this, SLOT( opFinished() ) );
    }
}

/*============================== Public methods ============================*/

void RequestProgressDialog::setUploadLabelText(const QString &text)
{
    muploadText = text;
    if ( !isValid() )
        return;
    if (!muploadCompleted)
        mlbl->setText(text);
}

void RequestProgressDialog::setDownloadLabelText(const QString &text)
{
    mdownloadText = text;
    if (muploadCompleted)
        mlbl->setText(text);
}

QString RequestProgressDialog::uploadLabelText() const
{
    return muploadText;
}

QString RequestProgressDialog::downloadLabelText() const
{
    return mdownloadText;
}

bool RequestProgressDialog::isValid() const
{
    return !Request.isNull() && Request->isRequest();
}

BNetworkOperation *RequestProgressDialog::request() const
{
    return Request.data();
}

/*============================== Protected methods =========================*/

void RequestProgressDialog::closeEvent(QCloseEvent *e)
{
    if (mfinished)
        QDialog::closeEvent(e);
    else
        e->ignore();
}

/*============================== Private slots =============================*/

void RequestProgressDialog::uploadProgress(int bytesReady, int bytesTotal)
{
    if (bytesTotal < 0)
        return;
    mpbar->setMaximum(bytesTotal);
    mpbar->setValue(bytesReady);
    if (bytesReady == bytesTotal)
    {
        muploadCompleted = true;
        mlbl->setText(mdownloadText);
        mpbar->setValue(0);
        mpbar->setMaximum(0);
    }
}

void RequestProgressDialog::downloadProgress(int bytesReady, int bytesTotal)
{
    if (bytesTotal < 0)
        return;
    mpbar->setMaximum(bytesTotal);
    mpbar->setValue(bytesReady);
}

void RequestProgressDialog::opFinished()
{
    mfinished = true;
    close();
}
