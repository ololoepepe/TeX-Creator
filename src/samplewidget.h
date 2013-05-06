#ifndef SAMPLEWIDGET_H
#define SAMPLEWIDGET_H

class TSampleInfo;

class BCodeEditorDocument;

class QTextCodec;
class QLineEdit;
class QLabel;
class QComboBox;
class QSpinBox;
class QString;

class QListWidget;
class QPlainTextEdit;
class QListWidgetItem;
class QPushButton;

class QStringList;

#include <QDialog>
#include <QList>

/*============================================================================
================================ SampleWidget ================================
============================================================================*/

class SampleWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode
    {
        AddMode,
        EditMode,
        UpdateMode,
        ShowMode
    };
public:
    explicit SampleWidget(Mode m, QWidget *parent = 0);
public:
    void setInfo(const TSampleInfo &info);
    void setFileName(const QString &fn);
    Mode mode() const;
    TSampleInfo info() const;
    bool isValid() const;
private slots:
    void checkInputs();
    void showAuthorInfo(const QString &idString);
signals:
    void validityChanged(bool valid);
private:
    const Mode mmode;
private:
    bool mvalid;
    quint64 mid;
    QLineEdit *mledtTitle;
    QLabel *mlblTitle;
    QLabel *mlblAuthor;
    QComboBox *mcmboxType;
    QLabel *mlblType;
    QSpinBox *msboxRating;
    QLabel *mlblRating;
    QLineEdit *mledtFileName;
    QLabel *mlblFileName;
    QPlainTextEdit *mptedtExtraAuthors;
    QPlainTextEdit *mptedtTags;
    QPlainTextEdit *mptedtComment;
    QPlainTextEdit *mptedtRemark;
};

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

#endif // SAMPLEWIDGET_H
