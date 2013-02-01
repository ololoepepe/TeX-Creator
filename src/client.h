#ifndef CLIENT_H
#define CLIENT_H

class SamplesModel;
class Cache;

class BNetworkConnection;
class BNetworkOperation;
class BCodeEditor;
class BCodeEditorDocument;

class QStringList;
class QTextCodec;

#include "sample.h"

#include <QObject>
#include <QAbstractSocket>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QList>
#include <QDateTime>

#define sModel Client::samplesModelInstance()

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
    enum AccessLevel
    {
        NoLevel = 0,
        UserLevel = 10,
        ModeratorLevel = 100,
        AdminLevel = 1000
    };
public:
    struct SampleData
    {
        QString title;
        QString fileName;
        QString text;
        QString initialFileName;
        QTextCodec *codec;
        QStringList tags;
        QString comment;
        //
        SampleData()
        {
            codec = 0;
        }
    };
public:
    explicit Client(QObject *parent = 0);
    ~Client();
public:
    static SamplesModel *samplesModelInstance();
public:
    bool updateSettings();
    void setConnected(bool b);
    State state() const;
    bool canConnect() const;
    bool canDisconnect() const;
    bool isAuthorized() const;
    int accessLevel() const;
    QString realName() const;
    bool updateSamplesList(bool full = false, QString *errorString = 0, QWidget *parent = 0);
    bool previewSample(quint64 id, QWidget *parent = 0, bool full = false);
    bool insertSample(quint64 id, BCodeEditor *edr);
    bool addSample(const SampleData &data, QString *errorString = 0, QString *log = 0, QWidget *parent = 0);
    bool deleteSample(quint64 id, QWidget *parent = 0);
    bool updateAccount(const QByteArray &password, const QString &realName, QWidget *parent = 0);
    bool addUser(const QString &login, const QByteArray &password, const QString &realName, int accessLevel,
                 QWidget *parent = 0);
public slots:
    void connectToServer();
    void reconnect();
    void disconnectFromServer();
private:
    static QStringList auxFileNames(const QString &text);
    static QStringList restrictedCommands(const QString &text);
    static QStringList absoluteFileNames(const QStringList &fileNames);
    static inline bool retErr(QString *errs, const QString &string);
    static inline QWidget *chooseParent(QWidget *supposed = 0);
    static QString operationErrorString();
    static bool writeSample(const QString &path, const QVariantMap &sample, QTextCodec *codec = 0);
    static bool insertSample(BCodeEditorDocument *doc, const QVariantMap &sample);
private:
    void setState( State s, int accessLvl = -1, const QString &realName = QString() );
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
    void realNameChanged(const QString &name);
private:
    SamplesModel *msamplesModel;
    Cache *mcache;
    BNetworkConnection *mconnection;
    QString mhost;
    QString mlogin;
    QByteArray mpassword;
    State mstate;
    int maccessLevel;
    QString mrealName;
    bool mreconnect;
    QDateTime mlastUpdated;
private:
    Q_DISABLE_COPY(Client)
};

#endif // CLIENT_H
