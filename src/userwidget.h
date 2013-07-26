#ifndef USERWIDGET_H
#define USERWIDGET_H

class TUserInfo;

class BPasswordWidget;
class BInputField;

class QLineEdit;
class QComboBox;
class QToolButton;
class QCheckBox;

#include <TService>
#include <TServiceList>

#include <BPassword>

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <QMap>

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
    void setAvailableServices(const TServiceList &list);
    void setInfo(const TUserInfo &info);
    void setPassword(const BPassword &pwd);
    void restoreState(const QByteArray &state);
    Mode mode() const;
    TServiceList availableServices() const;
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
    TServiceList mservices;
    quint64 mid;
    QString mavatarFileName;
    QByteArray mavatar;
    QLineEdit *mledtInvite;
    BInputField *minputInvite;
    QLineEdit *mledtEmail;
    BInputField *minputEmail;
    QLineEdit *mledtLogin;
    BInputField *minputLogin;
    BPasswordWidget *mpwdwgt1;
    BInputField *minputPwd1;
    BPasswordWidget *mpwdwgt2;
    BInputField *minputPwd2;
    QComboBox *mcmboxAccessLevel;
    QLineEdit *mledtRealName;
    QToolButton *mtbtnAvatar;
    QToolButton *mtbtnClearAvatar;
    QMap<TService, QCheckBox *> mcboxMap;
};

#endif // USERWIDGET_H
