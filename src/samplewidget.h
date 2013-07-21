#ifndef SAMPLEWIDGET_H
#define SAMPLEWIDGET_H

class TSampleInfo;

class BCodeEditor;
class BAbstractCodeEditorDocument;

class QLineEdit;
class QLabel;
class QComboBox;
class QSpinBox;
class QPlainTextEdit;
class QToolButton;
class QByteArray;
class QListWidget;
class QStringList;
class QTextCodec;
class QListWidgetItem;
class QSignalMapper;

#include <QDialog>
#include <QString>

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
    static bool showSelectSampleDialog(QString &fileName, QTextCodec *&codec, QWidget *parent = 0);
public:
    explicit SampleWidget(Mode m, QWidget *parent = 0);
    explicit SampleWidget(Mode m, BCodeEditor *editor, QWidget *parent = 0);
    explicit SampleWidget(Mode m, BCodeEditor *editor, const QString &fileName, QTextCodec *codec,
                          QWidget *parent = 0);
public:
    void setInfo(const TSampleInfo &info);
    void setFocus();
    void restoreState(const QByteArray &state);
    Mode mode() const;
    TSampleInfo info() const;
    QString actualFileName() const;
    QTextCodec *codec() const;
    BAbstractCodeEditorDocument *document() const;
    QByteArray saveState() const;
    bool isValid() const;
private:
    void init();
    void setAuthors(const QStringList &list);
    QStringList authors() const;
    void setProjectSize(int sz = 0);
private slots:
    void documentAvailableChanged(bool available);
    void lstwgtCurrentItemChanged(QListWidgetItem *current);
    void addAuthor(const QString &s = QString());
    void removeAuthor();
    void clearAuthors();
    void authorUp();
    void authorDown();
    void checkInputs();
    void showSenderInfo();
    void previewSample();
    void useCurrentDocument();
    void useExternalFile();
    void addTag(const QString &tag);
    void setFile(const QString &fn, QTextCodec *codec = 0);
signals:
    void validityChanged(bool valid);
private:
    static QString createFileName(const QString &fn);
private:
    static const QString DateTimeFormat;
private:
    const Mode mmode;
    BCodeEditor *const meditor;
private:
    bool mvalid;
    quint64 mid;
    quint64 msenderId;
    QString msenderLogin;
    QString msenderRealName;
    QString mactualFileName;
    QTextCodec *mcodec;
    BAbstractCodeEditorDocument *mdoc;
    QSignalMapper *mmprTags;
    QSignalMapper *mmprAuthors;
    QToolButton *mtbtnUseCurrentDocument;
    int mprojectSize;
    QLineEdit *mledtTitle;
    QLineEdit *mledtFileName;
    QLineEdit *mledtTags;
    QToolButton *mtbtnTags;
    QSpinBox *msboxRating;
    QLabel *mlblSize;
    QComboBox *mcmboxType;
    QLabel *mlblSender;
    QLabel *mlblCreationDT;
    QLabel *mlblUpdateDT;
    QListWidget *mlstwgtAuthors;
    QToolButton *mtbtnAdd;
    QToolButton *mtbtnRemove;
    QToolButton *mtbtnClear;
    QToolButton *mtbtnUp;
    QToolButton *mtbtnDown;
    QPlainTextEdit *mptedtComment;
    QPlainTextEdit *mptedtRemark;
};

#endif // SAMPLEWIDGET_H
