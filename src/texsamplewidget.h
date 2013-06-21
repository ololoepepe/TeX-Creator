#ifndef TEXSAMPLEWIDGET_H
#define TEXSAMPLEWIDGET_H

class SamplesProxyModel;
class MainWindow;
class ConnectionAction;

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

#include "client.h"

#include <QWidget>
#include <QList>
#include <QMap>

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
private slots:
    void retranslateUi();
    void actSendCurrentTriggreed();
    void actSendExternalTriggreed();
    void actSettingsTriggered();
    void actRegisterTriggered();
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
    void editSample();
    void deleteSample();
    void infoDialogDestroyed(QObject *obj);
private:
    MainWindow *const Window;
private:
    SamplesProxyModel *mproxyModel;
    quint64 mlastId;
    QMap<quint64, QDialog *> minfoDialogMap;
    QMap<QObject *, quint64> minfoDialogIdMap;
    //
    QToolBar *mtbarIndicator;
    //
    QToolBar *mtbar;
      ConnectionAction *mactConnection;
        QAction *mactConnect;
        QAction *mactDisconnect;
      QAction *mactUpdate;
      QAction *mactSend;
        QAction *mactSendCurrent;
        QAction *mactSendExternal;
      QAction *mactTools;
        QAction *mactRegister;
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
