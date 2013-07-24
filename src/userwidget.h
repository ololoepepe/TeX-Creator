#ifndef USERWIDGET_H
#define USERWIDGET_H

class TUserInfo;

class BPasswordWidget;

class QLineEdit;
class QComboBox;
class QToolButton;
class QCheckBox;

#include <BPassword>

#include <QWidget>
#include <QByteArray>
#include <QString>

/*============================================================================
================================ UserWidget ==================================
============================================================================*/

class UserWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode
    {
        AddMode,
        RegisterMode,
        EditMode,
        UpdateMode,
        ShowMode
    };
public:
    explicit UserWidget(Mode m, QWidget *parent = 0);
    ~UserWidget();
public:
    void setInfo(const TUserInfo &info);
    void setPassword(const BPassword &pwd);
    void restoreState(const QByteArray &state);
    Mode mode() const;
    TUserInfo info() const;
    BPassword password() const;
    QByteArray saveState() const;
    bool isValid() const;
private slots:
    void resetAvatar(const QByteArray &data = QByteArray());
    void checkInputs();
    void tbtnAvatarClicked();
signals:
    void validityChanged(bool valid);
private:
    const Mode mmode;
private:
    bool mvalid;
    quint64 mid;
    QString mavatarFileName;
    QByteArray mavatar;
    QLineEdit *mledtInvite;
    QLineEdit *mledtEmail;
    QLineEdit *mledtLogin;
    BPasswordWidget *mpwdwgt1;
    BPasswordWidget *mpwdwgt2;
    QComboBox *mcmboxAccessLevel;
    QLineEdit *mledtRealName;
    QToolButton *mtbtnAvatar;
    QToolButton *mtbtnClearAvatar;
    QCheckBox *mcboxTexsample;
    QCheckBox *mcboxClab;
};

#endif // USERWIDGET_H
