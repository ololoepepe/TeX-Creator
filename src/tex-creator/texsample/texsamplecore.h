/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
**
** This file is part of TeX Creator.
**
** TeX Creator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** TeX Creator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with TeX Creator.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef TEXSAMPLECORE_H
#define TEXSAMPLECORE_H

class Cache;
class SampleModel;

class TGroupModel;
class TInviteModel;
class TNetworkClient;
class TTexProject;
class TUserModel;

class BCodeEditor;
class BNetworkConnection;
class BNetworkOperation;

class QString;
class QWidget;

#include "application.h"

#include <BVersion>

#include <QDateTime>
#include <QFuture>
#include <QFutureWatcher>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QWidget>

#if defined(tSmp)
#   undef tSmp
#endif
#define tSmp (static_cast<Application *>(BApplication::instance())->texsampleCore())

/*============================================================================
================================ TexsampleCore ===============================
============================================================================*/

class TexsampleCore : public QObject
{
    Q_OBJECT
private:
    Cache *mcache;
    TNetworkClient *mclient;
    bool mdestructorCalled;
    QList<QObject *> mfutureWatchers;
    QPointer<QWidget> mgroupManagementWidget;
    TGroupModel *mgroupModel;
    QPointer<QWidget> minviteManagementWidget;
    TInviteModel *minviteModel;
    QMap< quint64, QPointer<QWidget> > msampleInfoWidgets;
    QDateTime msampleListLastUpdateDateTime;
    SampleModel *msampleModel;
    QMap< quint64, QPointer<QWidget> > muserInfoWidgets;
    QPointer<QWidget> muserManagementWidget;
    TUserModel *muserModel;
public:
    explicit TexsampleCore(QObject *parent = 0);
    ~TexsampleCore();
public:
    Cache *cache() const;
    TNetworkClient *client() const;
    TGroupModel *groupModel() const;
    TInviteModel *inviteModel() const;
    SampleModel *sampleModel() const;
    void updateCacheSettings();
    void updateClientSettings();
    TUserModel *userModel() const;
public slots:
    bool checkForNewVersion(bool persistent = false);
    bool checkForNewVersionPersistent();
    void connectToServer();
    bool deleteSample(quint64 sampleId, QWidget *parent = 0);
    void disconnectFromServer();
    void editSample(quint64 sampleId, BCodeEditor *editor = 0);
    bool insertSample(quint64 sampleId, BCodeEditor *editor);
    bool saveSample(quint64 sampleId, QWidget *parent = 0);
    void sendSample(BCodeEditor *editor = 0);
    bool showAccountManagementDialog(QWidget *parent = 0);
    bool showConfirmRegistrationDialog(QWidget *parent = 0);
    void showGroupManagementWidget();
    void showInviteManagementWidget();
    bool showRecoverDialog(QWidget *parent = 0);
    bool showRegisterDialog(QWidget *parent = 0);
    void showSampleInfo(quint64 sampleId);
    void showSamplePreview(quint64 sampleId);
    bool showTexsampleSettings(QWidget *parent = 0);
    void showUserInfo(quint64 userId);
    void showUserManagementWidget();
    void updateSampleList();
signals:
    void stopWaiting();
private:
    struct CheckForNewVersionResult
    {
    public:
        QString message;
        bool persistent;
        bool success;
        QUrl url;
        BVersion version;
    public:
        explicit CheckForNewVersionResult(bool persistent = false);
    };
private:
    typedef QFuture<CheckForNewVersionResult> Future;
    typedef QFutureWatcher<CheckForNewVersionResult> Watcher;
private:
    static CheckForNewVersionResult checkForNewVersionFunction(bool persistent);
    static void showMessageFunction(const QString &text, const QString &informativeText, bool error,
                                    QWidget *parentWidget);
    static bool waitForConnectedFunction(BNetworkConnection *connection, int timeout, QWidget *parentWidget,
                                         QString *msg);
    static bool waitForFinishedFunction(BNetworkOperation *op, int timeout, QWidget *parentWidget, QString *msg);
private:
    bool getSampleSource(quint64 sampleId, TTexProject &source, QWidget *parent = 0);
private slots:
    void checkingForNewVersionFinished();
};

#endif // TEXSAMPLECORE_H
