#ifndef SAMPLEWIDGET_H
#define SAMPLEWIDGET_H

class TSampleInfo;

class BCodeEditor;

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
    explicit SampleWidget(Mode m, BCodeEditor *editor, QWidget *parent = 0);
    explicit SampleWidget(Mode m, BCodeEditor *editor, const QString &fileName, QTextCodec *codec,
                          QWidget *parent = 0);
public:
    void setInfo(const TSampleInfo &info);
    void restoreState(const QByteArray &state, bool full = true);
    Mode mode() const;
    TSampleInfo info() const;
    QString actualFileName() const;
    QByteArray saveState(bool full = true) const;
    bool isValid() const;
private:
    void init();
    void setProjectSize(int sz = 0);
    void setFile(const QString &fn, QTextCodec *codec = 0);
private slots:
    void documentAvailableChanged(bool available);
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
    BCodeEditor *const meditor;
private:
    bool mvalid;
    quint64 mid;
    quint64 msenderId;
    QString msenderLogin;
    QString msenderRealName;
    QString mactualFileName;
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
    QPlainTextEdit *mptedtComment;
    QPlainTextEdit *mptedtRemark;
};

#endif // SAMPLEWIDGET_H
