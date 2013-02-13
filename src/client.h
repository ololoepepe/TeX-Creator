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
#include <QImage>
#include <QUuid>
#include <QVariantMap>
#include <QVariantList>

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
    struct UserInfo
    {
        QString login;
        AccessLevel accessLevel;
        QString realName;
        QByteArray avatar;
        //
        UserInfo()
        {
            accessLevel = NoLevel;
        }
    };
    struct Invite
    {
        QUuid uuid;
        QDateTime expires;
    };
    struct CompileParameters
    {
        QString fileName;
        QTextCodec *codec;
        QString compiler;
        bool makeindex;
        bool dvips;
        QStringList options;
        QStringList commands;
        //
        CompileParameters()
        {
            codec = 0;
            compiler = "pdflatex";
            makeindex = false;
            dvips = false;
        }
    };
public:
    explicit Client(QObject *parent = 0);
    ~Client();
public:
    static SamplesModel *samplesModelInstance();
    static QString accessLevelToLocalizedString(AccessLevel lvl);
public:
    bool updateSettings();
    void setConnected(bool b);
    State state() const;
    bool canConnect() const;
    bool canDisconnect() const;
    bool isAuthorized() const;
    QString login() const;
    int accessLevel() const;
    QString realName() const;
    QByteArray avatar() const;
    bool updateSamplesList(bool full = false, QString *errorString = 0, QWidget *parent = 0);
    bool previewSample(quint64 id, QWidget *parent = 0, bool full = false);
    bool insertSample(quint64 id, BCodeEditor *edr);
    bool addSample(const SampleData &data, QString *errorString = 0, QString *log = 0, QWidget *parent = 0);
    bool updateSample(const Sample &newInfo, bool moderLevel = false, QWidget *parent = 0);
    bool deleteSample(quint64 id, const QString &reason = QString(), QWidget *parent = 0);
    bool updateAccount(const QByteArray &password, const QString &realName, const QByteArray &avatar,
                       QWidget *parent = 0);
    bool generateInvite(QUuid &invite, const QDateTime &expires = QDateTime(), QWidget *parent = 0);
    bool getInvitesList(QList<Invite> &list, QWidget *parent = 0);
    bool addUser(const QString &login, const QByteArray &password, const QString &realName, int accessLevel,
                 QWidget *parent = 0);
    UserInfo getUserInfo(const QString &login, QWidget *parent = 0);
    bool compile(const CompileParameters &param, QString *errorString = 0, int *exitCode = 0, QString *log = 0,
                 QWidget *parent = 0);
public slots:
    void connectToServer();
    void reconnect();
    void disconnectFromServer();
private:
    enum FilePackingMode
    {
        PackAuto,
        PackAsBinary,
        PackAsText
    };
private:
    static QStringList auxFileNames(const QString &text, const QString &path = QString(), QTextCodec *codec = 0,
                                    bool *ok = 0);
    static QString withoutRestrictedCommands(const QString &text);
    static QStringList restrictedCommands(const QString &text);
    static QStringList absoluteFileNames(const QStringList &fileNames);
    static inline bool retErr(QString *errs, const QString &string);
    static inline QWidget *chooseParent(QWidget *supposed = 0);
    static QString operationErrorString();
    static QString sampleSubdirName(quint64 id, const QString &fileName);
    static QString sampleSubdirPath(const QString &path, quint64 id);
    static QString sampleSourceFileName(const QString &subdirPath);
    static bool writeSample(const QString &path, quint64 id, const QVariantMap &sample, QTextCodec *codec = 0);
    static QVariantMap packProject(const CompileParameters &param, bool *ok = 0, QString *errorString = 0);
    static QVariantMap packSample(const SampleData &data, bool *ok = 0, QString *errorString = 0);
    static QVariantList packAuxFiles(const QStringList &fileNames, const QString &path,
                                     bool *ok = 0, QString *errorString = 0, qint64 *sz = 0);
    static bool insertSample(BCodeEditorDocument *doc, quint64 id, const QString &fileName);
    static UserInfo userInfoFromVariantMap(const QVariantMap &m, const QString &login);
    static bool packFile(QVariantMap &target, const QString &path, const QString &relativeFileName,
                         FilePackingMode mode = PackAuto, QTextCodec *codec = 0);
    static bool packFile(QVariantMap &target, const QString &fileName,
                         FilePackingMode mode = PackAuto, QTextCodec *codec = 0);
    static QVariantMap packFile(const QString &path, const QString &relativeFileName,
                                FilePackingMode mode = PackAuto, QTextCodec *codec = 0, bool *ok = 0);
    static QVariantMap packFile(const QString &fileName,
                                FilePackingMode mode = PackAuto, QTextCodec *codec = 0, bool *ok = 0);
    static bool packTextFile(QVariantMap &target, const QString &text, const QString &fileName,
                             const QString &initialFileName, QTextCodec *codec = 0);
    static QVariantMap packTextFile(const QString &text, const QString &fileName,
                                    const QString &initialFileName, QTextCodec *codec = 0, bool *ok = 0);
private:
    void setState( State s, int accessLvl = -1, const QString &realName = QString(),
                   const QByteArray &avatar = QByteArray() );
    QDateTime lastUpdated() const;
    void setLastUpdated(const QDateTime &dt);
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
    static const int ProgressDialogDelay;
    static const int MaxSampleSize;
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
    QByteArray mavatar;
    bool mreconnect;
    QDateTime mlastUpdated;
private:
    Q_DISABLE_COPY(Client)
};

#endif // CLIENT_H
