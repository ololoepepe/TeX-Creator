#ifndef EDITSAMPLEDIALOG_H
#define EDITSAMPLEDIALOG_H

class TSampleInfo;

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
    explicit EditSampleDialog(const TSampleInfo *s, QWidget *parent = 0);
private slots:
    void updateSample();
    void checkButtons();
private:
    const TSampleInfo * const S;
private:
    QLineEdit *mledtTitle;
    QLineEdit *mledtTags;
    QPlainTextEdit *mptedtComment;
    QComboBox *mcmboxType;
    QSpinBox *msboxRating;
    QPlainTextEdit *mptedtRemark;
};

#endif // EDITSAMPLEDIALOG_H
