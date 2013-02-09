#ifndef SENDSAMPLESDIALOG_H
#define SENDSAMPLESDIALOG_H

class QTextCodec;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QListWidgetItem;
class QPushButton;
class QString;
class QStringList;

class BCodeEditorDocument;

#include <QDialog>
#include <QList>

/*============================================================================
================================ SendSamplesDialog ===========================
============================================================================*/

class SendSamplesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SendSamplesDialog(BCodeEditorDocument *doc, QWidget *parent = 0);
    explicit SendSamplesDialog(const QList<BCodeEditorDocument *> &docs, QWidget *parent = 0);
    explicit SendSamplesDialog(const QStringList &fileNames, QTextCodec *codec, QWidget *parent = 0);
private:
    enum DataRole
    {
        InitialFileNameRole = Qt::UserRole + 1,
        FileNameRole,
        TextRole,
        CodecNameRole,
        TitleRole,
        TagsRole,
        CommentRole,
        HasLogRole,
        LogRole,
        SentRole
    };
private:
    static QListWidgetItem *createItem( const QString &fileName, QTextCodec *codec, const QString &text = QString() );
    static QString fromPlainText(const QString &text);
private:
    void init(const QList<QListWidgetItem *> &items);
private slots:
    void send();
    void showLog();
    void lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void ledtTitleTextEdited(const QString &text);
    void checkSendAvailable();
private:
    QListWidget *mlstwgt;
    QLineEdit *mledtTitle;
    QLineEdit *mledtFileName;
    QLineEdit *mledtTags;
    QPlainTextEdit *mptedtComment;
    QPushButton *mbtnShowLog;
    QPushButton *mbtnSend;
private:
    Q_DISABLE_COPY(SendSamplesDialog)
};

#endif // SENDSAMPLESDIALOG_H
