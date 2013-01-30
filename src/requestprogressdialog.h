#ifndef REQUESTPROGRESSDIALOG_H
#define REQUESTPROGRESSDIALOG_H

class QCloseEvent;
class QProgressBar;
class QLabel;

#include <BNetworkOperation>

#include <QDialog>
#include <QPointer>
#include <QString>

/*============================================================================
================================ RequestProgressDialog =======================
============================================================================*/

class RequestProgressDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RequestProgressDialog(BNetworkOperation *request, QWidget *parent = 0);
public:
    void setUploadLabelText(const QString &text);
    void setDownloadLabelText(const QString &text);
    QString uploadLabelText() const;
    QString downloadLabelText() const;
    bool isValid() const;
    BNetworkOperation *request() const;
protected:
    void closeEvent(QCloseEvent *e);
private slots:
    void uploadProgress(int bytesReady, int bytesTotal);
    void downloadProgress(int bytesReady, int bytesTotal);
    void opFinished();
private:
    const QPointer<BNetworkOperation> Request;
private:
    QString muploadText;
    QString mdownloadText;
    bool muploadCompleted;
    bool mfinished;
    //
    QLabel *mlbl;
    QProgressBar *mpbar;
private:
    Q_DISABLE_COPY(RequestProgressDialog)
};

#endif // REQUESTPROGRESSDIALOG_H
