#ifndef USERWIDGET_H
#define USERWIDGET_H

class TUserInfo;

class BPasswordWidget;

class QLineEdit;
class QLabel;
class QComboBox;
class QToolButton;

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
public:
    void setInfo(const TUserInfo &info);
    void setPasswordState(const QByteArray &state);
    Mode mode() const;
    TUserInfo info() const;
    QByteArray passwordState() const;
    QString invite() const;
    bool isValid() const;
    bool passwordsMatch() const;
private:
    void resetAvatar(const QString &fileName);
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
    QLineEdit *mledtLogin;
    QLabel *mlblLogin;
    BPasswordWidget *mpwdwgt1;
    BPasswordWidget *mpwdwgt2;
    QComboBox *mcmboxAccessLevel;
    QLabel *mlblAccessLevel;
    QLineEdit *mledtRealName;
    QLabel *mlblRealName;
    QToolButton *mtbtnAvatar;
};

#endif // USERWIDGET_H
