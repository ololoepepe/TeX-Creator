#ifndef SAMPLEINFODIALOG_H
#define SAMPLEINFODIALOG_H

class TSampleInfo;

class QString;

#include "client.h"

#include <TUserInfo>

#include <QDialog>

/*============================================================================
================================ SampleInfoDialog ============================
============================================================================*/

class SampleInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SampleInfoDialog(const TSampleInfo *s, QWidget *parent = 0);
public:
    bool isValid() const;
private slots:
    void showAuthorInfo(const QString &login);
private:
    const TSampleInfo *const S;
private:
    Q_DISABLE_COPY(SampleInfoDialog)
};

/*============================================================================
================================ UserInfoDialog ==============================
============================================================================*/

class UserInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserInfoDialog(const TUserInfo &info, QWidget *parent = 0);
private:
    static QString pixmapInfo(const QPixmap &pm);
private slots:
    void showFullAvatar();
private:
    static const int MaxPixmapSize;
private:
    const TUserInfo Info;
private:
    Q_DISABLE_COPY(UserInfoDialog)
};

#endif // SAMPLEINFODIALOG_H
