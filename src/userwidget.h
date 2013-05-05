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
    Mode mode() const;
    TUserInfo info() const;
    QString invite() const;
private:
    void resetAvatar(const QString &fileName);
private slots:
    void resetAvatar(const QByteArray &data = QByteArray());
    void tbtnAvatarClicked();
private:
    const Mode mmode;
private:
    quint64 mid;
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
