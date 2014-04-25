/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
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

#ifndef TEXSAMPLEWIDGET_H
#define TEXSAMPLEWIDGET_H

class SamplesProxyModel;
class MainWindow;
class ConnectionAction;
class AddSampleDialog;
class SampleWidget;

class BCodeEditor;
class BInputField;

class QToolBar;
class QGroupBox;
class QComboBox;
class QLineEdit;
class QTableView;
class QLabel;
class QAction;
class QPoint;
class QString;
class QModelIndex;
class QDialog;
class QByteArray;
class QTextCodec;
class QCloseEvent;

#include "client.h"

#include <BDialog>

#include <QWidget>
#include <QList>
#include <QMap>
#include <QPointer>

/*============================================================================
================================ AddSampleDialog =============================
============================================================================*/

class AddSampleDialog : public BDialog
{
public:
    explicit AddSampleDialog(BCodeEditor *editor, QWidget *parent = 0);
public:
    SampleWidget *sampleWidget() const;
protected:
    void closeEvent(QCloseEvent *e);
private:
    SampleWidget *msmpwgt;
private:
    Q_DISABLE_COPY(AddSampleDialog)
};

/*============================================================================
================================ EditSampleDialog ============================
============================================================================*/

class EditSampleDialog : public BDialog
{
    Q_OBJECT
public:
    explicit EditSampleDialog(BCodeEditor *editor, quint64 id, QWidget *parent = 0);
public:
    SampleWidget *sampleWidget() const;
protected:
    void closeEvent(QCloseEvent *e);
private:
    SampleWidget *msmpwgt;
private:
    Q_DISABLE_COPY(EditSampleDialog)
};

/*============================================================================
================================ SelectUserDialog ============================
============================================================================*/

class SelectUserDialog : public BDialog
{
    Q_OBJECT
public:
    explicit SelectUserDialog(QWidget *parent = 0);
public:
    quint64 userId() const;
    QString userLogin() const;
private:
    QLineEdit *mledt;
    BInputField *mfield;
private slots:
    void buttonClicked(int id);
    void checkValidity();
private:
    Q_DISABLE_COPY(SelectUserDialog)
};

/*============================================================================
================================ TexsampleWidget =============================
============================================================================*/

class TexsampleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TexsampleWidget(MainWindow *window, QWidget *parent = 0);
    ~TexsampleWidget();
public:
    QList<QAction *> toolBarActions() const;
    QWidget *indicator() const;
signals:
    void message(const QString &msg);
private:
    void retranslateCmboxType();
    void showAddingSampleFailedMessage(const QString &errorString = QString());
    void showEditingSampleFailedMessage(const QString &errorString = QString());
private slots:
    void retranslateUi();
    void actSettingsTriggered();
    void actRegisterTriggered();
    void actRecoverTriggered();
    void actAccountSettingsTriggered();
    void actAddUserTriggered();
    void actEditUserTriggered();
    void actInvitesTriggered();
    void clientStateChanged(Client::State state);
    void clientAccessLevelChanged(int lvl);
    void cmboxTypeCurrentIndexChanged(int index);
    void tblvwDoubleClicked(const QModelIndex &index);
    void tblvwCustomContextMenuRequested(const QPoint &pos);
    void updateSamplesList();
    void showSampleInfo();
    void previewSample();
    void insertSample();
    void saveSample();
    void addSample();
    void addSampleCurrentFile();
    void addSampleExternalFile();
    void editSample();
    void deleteSample();
    void infoDialogFinished();
    void addDialogFinished();
    void editDialogFinished();
private:
    MainWindow *const Window;
private:
    SamplesProxyModel *mproxyModel;
    quint64 mlastId;
    QMap< quint64, QPointer<QDialog> > minfoDialogMap;
    QMap<QPointer<QObject>, quint64> minfoDialogIdMap;
    QMap< quint64, QPointer<QDialog> > meditDialogMap;
    QMap<QPointer<QObject>, quint64> meditDialogIdMap;
    QPointer<AddSampleDialog> maddDialog;
    //
    QToolBar *mtbarIndicator;
    //
    QToolBar *mtbar;
      ConnectionAction *mactConnection;
        QAction *mactConnect;
        QAction *mactDisconnect;
      QAction *mactUpdate;
      QAction *mactSend;
        QAction *mactSendVariant;
        QAction *mactSendCurrent;
        QAction *mactSendExternal;
      QAction *mactTools;
        QAction *mactRegister;
        QAction *mactRecover;
        QAction *mactSettings;
        QAction *mactAccountSettings;
        QAction *mactAdministration;
          QAction *mactAddUser;
          QAction *mactEditUser;
          QAction *mactInvites;
    QGroupBox *mgboxSelect;
      QLabel *mlblType;
      QComboBox *mcmboxType;
      QLabel *mlblSearch;
      QLineEdit *mledtSearch;
    QTableView *mtblvw;
private:
    Q_DISABLE_COPY(TexsampleWidget)
};

#endif // TEXSAMPLEWIDGET_H
