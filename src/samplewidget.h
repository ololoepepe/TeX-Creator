#ifndef SAMPLEWIDGET_H
#define SAMPLEWIDGET_H

class TSampleInfo;

class QLineEdit;
class QLabel;
class QComboBox;
class QSpinBox;
class QString;
class QPlainTextEdit;

#include <QDialog>

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

#endif // SAMPLEWIDGET_H
