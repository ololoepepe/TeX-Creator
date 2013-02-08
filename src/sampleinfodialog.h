#ifndef SAMPLEINFODIALOG_H
#define SAMPLEINFODIALOG_H

class Sample;

class QString;
class QLabel;
class QPushButton;

#include <QDialog>

/*============================================================================
================================ SampleInfoDialog ============================
============================================================================*/

class SampleInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SampleInfoDialog(const Sample *s, QWidget *parent = 0);
public:
    bool isValid() const;
private slots:
    void showAuthorInfo(const QString &login);
private:
    const Sample *const S;
    //
    QLabel *mlblTitle;
    QLabel *mlblAuthor;
    QLabel *mlblType;
    QLabel *mlblModified;
    QLabel *mlblComment;
    QPushButton *mbtnShowRemark;
    QPushButton *mbtnShowTags;
private:
    Q_DISABLE_COPY(SampleInfoDialog)
};

#endif // SAMPLEINFODIALOG_H
