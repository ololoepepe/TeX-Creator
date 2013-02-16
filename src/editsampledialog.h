#ifndef EDITSAMPLEDIALOG_H
#define EDITSAMPLEDIALOG_H

class Sample;

class QLineEdit;
class QPlainTextEdit;
class QComboBox;
class QSpinBox;

#include <QDialog>

/*============================================================================
================================ EditSampleDialog ============================
============================================================================*/

class EditSampleDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditSampleDialog(const Sample *s, QWidget *parent = 0);
private slots:
    void updateSample();
    void checkButtons();
private:
    const Sample * const S;
private:
    QLineEdit *mledtTitle;
    QLineEdit *mledtTags;
    QPlainTextEdit *mptedtComment;
    QComboBox *mcmboxType;
    QSpinBox *msboxRating;
    QPlainTextEdit *mptedtRemark;
};

#endif // EDITSAMPLEDIALOG_H
