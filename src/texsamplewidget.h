#ifndef TEXSAMPLEWIDGET_H
#define TEXSAMPLEWIDGET_H

class SamplesProxyModel;
class MainWindow;
class ConnectionAction;
class AddSampleDialog;
class SampleWidget;

class BCodeEditor;

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
    bool showEditSampleDialog(quint64 id, TSampleInfo &info, bool moder);
    void showAddingSampleFailedMessage(const QString &errorString = QString());
    void showEditingSampleFailedMessage(const QString &errorString = QString());
private slots:
    void retranslateUi();
    void actSendVariantTriggreed();
    void actSendCurrentTriggreed();
    void actSendExternalTriggreed();
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
    void editSample();
    void editSampleCurrentDocument();
    void editSampleExternalFile();
    void deleteSample();
    void infoDialogFinished();
    void addDialogFinished();
private:
    MainWindow *const Window;
private:
    SamplesProxyModel *mproxyModel;
    quint64 mlastId;
    QMap<quint64, QDialog *> minfoDialogMap;
    QMap<QObject *, quint64> minfoDialogIdMap;
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
