#ifndef SAMPLESWIDGET_H
#define SAMPLESWIDGET_H

class SamplesProxyModel;
class MainWindow;
class SampleInfoDialog;

class QToolBar;
class QGroupBox;
class QComboBox;
class QLineEdit;
class QTableView;
class QLabel;
class QAction;
class QPoint;
class QString;

#include "client.h"

#include <QWidget>
#include <QList>
#include <QMap>

/*============================================================================
================================ SamplesWidget ===============================
============================================================================*/

class SamplesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SamplesWidget(MainWindow *window, QWidget *parent = 0);
    ~SamplesWidget();
public:
    QList<QAction *> toolBarActions() const;
private:
    void retranslateCmboxType();
    void resetActConnection(const QString &toolTip, const QString &iconName, bool animated = false);
private slots:
    void retranslateUi();
    void actSendCurrentTriggreed();
    void actSendAllTriggreed();
    void actSendExternalTriggreed();
    void actSettingsTriggered();
    void actAccountSettingsTriggered();
    void clientStateChanged(Client::State state);
    void cmboxTypeCurrentIndexChanged(int index);
    void tblvwDoubleClicked(const QModelIndex &index);
    void tblvwCustomContextMenuRequested(const QPoint &pos);
    void updateSamplesList();
    void showSampleInfo();
    void previewSample();
    void insertSample();
    void deleteSample();
    void infoDialogDestroyed(QObject *obj);
private:
    MainWindow *const Window;
private:
    SamplesProxyModel *mproxyModel;
    quint64 mlastId;
    QMap<quint64, SampleInfoDialog *> minfoDialogMap;
    QMap<QObject *, quint64> minfoDialogIdMap;
    //
    QToolBar *mtbar;
      QAction *mactConnection;
        QAction *mactConnect;
        QAction *mactDisconnect;
      QAction *mactUpdate;
      QAction *mactSend;
        QAction *mactSendCurrent;
        QAction *mactSendAll;
        QAction *mactSendExternal;
      QAction *mactTools;
        QAction *mactSettings;
        QAction *mactAccountSettings;
    QGroupBox *mgboxSelect;
      QLabel *mlblType;
      QComboBox *mcmboxType;
      QLabel *mlblSearch;
      QLineEdit *mledtSearch;
    QTableView *mtblvw;
private:
    Q_DISABLE_COPY(SamplesWidget)
};

#endif // SAMPLESWIDGET_H