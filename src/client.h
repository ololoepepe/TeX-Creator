#ifndef CLIENT_H
#define CLIENT_H

class SamplesModel;
class Cache;

class TCompilerParameters;
class TCompilationResult;
class TOperationResult;

class BNetworkConnection;
class BNetworkOperation;
class BCodeEditorDocument;

class QTextCodec;

#include <TInviteInfo>
#include <TUserInfo>
#include <TSampleInfo>
#include <TeXSample>
#include <TAccessLevel>

#include <QObject>
#include <QAbstractSocket>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QList>
#include <QDateTime>
#include <QImage>
#include <QUuid>
#include <QVariantMap>
#include <QVariantList>
#include <QStringList>

#define sClient Client::instance()

/*============================================================================
================================ Client ======================================
============================================================================*/

class Client : public QObject
{
    Q_OBJECT
public:
    enum State
    {
        DisconnectedState,
        ConnectingState,
        ConnectedState,
        AuthorizedState,
        DisconnectingState
    };
public:
    static Client *instance();
    static TOperationResult registerUser(const TUserInfo &info, const QString &invite, QWidget *parent = 0);
public:
    explicit Client(QObject *parent = 0);
    ~Client();
public:
    bool updateSettings();
    void setConnected(bool b);
    State state() const;
    bool canConnect() const;
    bool canDisconnect() const;
    bool isAuthorized() const;
    QString login() const;
    TAccessLevel accessLevel() const;
    quint64 userId() const;
    TOperationResult addUser(const TUserInfo &info, QWidget *parent = 0);
    TOperationResult editUser(const TUserInfo &info, QWidget *parent = 0);
    TOperationResult updateAccount(TUserInfo info, QWidget *parent = 0);
    TOperationResult getUserInfo(quint64 id, TUserInfo &info, QWidget *parent = 0);
    TCompilationResult addSample(const QString &fileName, QTextCodec *codec, const TSampleInfo &info,
                                 QWidget *parent = 0);
    TCompilationResult addSample(const QString &fileName, QTextCodec *codec, const QString &text,
                                 const TSampleInfo &info, QWidget *parent = 0);
    TCompilationResult editSample(const TSampleInfo &newInfo, QWidget *parent = 0);
    TCompilationResult updateSample(const TSampleInfo &newInfo, QWidget *parent = 0);
    TOperationResult deleteSample(quint64 id, const QString &reason, QWidget *parent = 0);
    TOperationResult updateSamplesList(bool full = false, QWidget *parent = 0);
    TOperationResult insertSample(quint64 id, BCodeEditorDocument *doc, const QString &subdir);
    TOperationResult previewSample(quint64 id, QWidget *parent = 0, bool full = false);
    TOperationResult generateInvites(TInviteInfo::InvitesList &invites, const QDateTime &expiresDT, quint8 count = 1,
                                     QWidget *parent = 0);
    TOperationResult getInvitesList(TInviteInfo::InvitesList &list, QWidget *parent = 0);
    TCompilationResult compile(const QString &fileName, QTextCodec *codec, const TCompilerParameters &param,
                               TCompilationResult &makeindexResult, TCompilationResult &dvipsResult,
                               QWidget *parent = 0);
public slots:
    void connectToServer();
    void reconnect();
    void disconnectFromServer();
private:
    static inline QWidget *chooseParent(QWidget *supposed = 0);
    static QString notAuthorizedString();
    static QString invalidParametersString();
    static QString operationErrorString();
    static void showConnectionErrorMessage(const QString &errorString);
private:
    void setState(State s, TAccessLevel alvl = TAccessLevel::NoLevel);
    void updateSampleInfos(const TSampleInfo::SamplesList &newInfos, const Texsample::IdList &deletedInfos,
                           const QDateTime &updateDT);
    QDateTime sampleInfosUpdateDateTime(Qt::TimeSpec spec = Qt::UTC) const;
private slots:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError err);
signals:
    void loginChanged(const QString &login);
    void hostChanged(const QString &host);
    void stateChanged(Client::State s);
    void canConnectChanged(bool b);
    void canDisconnectChanged(bool b);
    void authorizedChanged(bool authorized);
    void accessLevelChanged(int lvl);
private:
    static const int ProgressDialogDelay;
    static const int MaxSampleSize;
private:
    static Client *minstance;
private:
    BNetworkConnection *mconnection;
    QString mhost;
    QString mlogin;
    QByteArray mpassword;
    TAccessLevel maccessLevel;
    quint64 mid;
    State mstate;
    bool mreconnect;
    mutable QDateTime msamplesListUpdateDT;
private:
    Q_DISABLE_COPY(Client)
};

#endif // CLIENT_H
