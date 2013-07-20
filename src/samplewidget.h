#ifndef SAMPLEWIDGET_H
#define SAMPLEWIDGET_H

class TSampleInfo;

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
    explicit SampleWidget(Mode m, QWidget *parent = 0);
    explicit SampleWidget(Mode m, BAbstractCodeEditorDocument *currentDocument, QWidget *parent = 0);
public:
    void setInfo(const TSampleInfo &info);
    void setActualFileName(const QString &fn);
    void setFileName(const QString &fn);
    void restoreState(const QByteArray &state, bool full = true);
    Mode mode() const;
    TSampleInfo info() const;
    QString actualFileName() const;
    QByteArray saveState(bool full = true) const;
    bool isValid() const;
private:
    void init();
    void setProjectSize(int sz = 0);
private slots:
    void checkInputs();
    void showSenderInfo();
    void previewSample();
    void useCurrentDocument();
    void useExternalFile();
    void addTag();
signals:
    void validityChanged(bool valid);
private:
    static QString createFileName(const QString &fn);
private:
    static const QString DateTimeFormat;
private:
    const Mode mmode;
    BAbstractCodeEditorDocument *const mdoc;
private:
    bool mvalid;
    quint64 mid;
    quint64 msenderId;
    QString msenderLogin;
    QString msenderRealName;
    QString mactualFileName;
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
    QPlainTextEdit *mptedtComment;
    QPlainTextEdit *mptedtRemark;
};

#endif // SAMPLEWIDGET_H
