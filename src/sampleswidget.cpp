#include "sampleswidget.h"
#include "samplesproxymodel.h"
#include "samplesmodel.h"
#include "application.h"
#include "client.h"
#include "texsamplesettingstab.h"
#include "mainwindow.h"
#include "sampleinfodialog.h"
#include "sample.h"
#include "sendsamplesdialog.h"
#include "accountsettingstab.h"
#include "administrationdialog.h"

#include <BApplication>
#include <BSettingsDialog>
#include <BDirTools>
#include <BSignalDelayProxy>
#include <BNetworkOperation>
#include <BCodeEditor>
#include <BCodeEditorDocument>
#include <BAbstractDocumentDriver>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QFormLayout>
#include <QLabel>
#include <QAction>
#include <QVariant>
#include <QVariantMap>
#include <QByteArray>
#include <QString>
#include <QMovie>
#include <QToolButton>
#include <QRegExp>
#include <QMenu>
#include <QSettings>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QTextCodec>

#include <QDebug>

/*============================================================================
================================ SamplesWidget ===============================
============================================================================*/

/*============================== Public constructors =======================*/

SamplesWidget::SamplesWidget(MainWindow *window, QWidget *parent) :
    QWidget(parent), Window(window)
{
    mlastId = 0;
    mproxyModel = new SamplesProxyModel(this);
    mproxyModel->setSourceModel(sModel);
    connect( sClient, SIGNAL( stateChanged(Client::State) ), this, SLOT( clientStateChanged(Client::State) ) );
    connect( sClient, SIGNAL( accessLevelChanged(int) ), this, SLOT( clientAccessLevelChanged(int) ) );
    //
    QVBoxLayout *vlt = new QVBoxLayout(this);
      mtbar = new QToolBar(this);
        mactConnection = new QAction(this);
          QMenu *mnu = new QMenu;
            mactConnect = new QAction(this);
              mactConnect->setEnabled( sClient->canConnect() );
              mactConnect->setIcon( Application::icon("connect_established") );
              connect( mactConnect, SIGNAL( triggered() ), sClient, SLOT( connectToServer() ) );
              connect( sClient, SIGNAL( canConnectChanged(bool) ), mactConnect, SLOT( setEnabled(bool) ) );
            mnu->addAction(mactConnect);
            mactDisconnect = new QAction(this);
              mactDisconnect->setEnabled( sClient->canDisconnect() );
              mactDisconnect->setIcon( Application::icon("connect_no") );
              connect( mactDisconnect, SIGNAL( triggered() ), sClient, SLOT( disconnectFromServer() ) );
              connect( sClient, SIGNAL( canDisconnectChanged(bool) ), mactDisconnect, SLOT( setEnabled(bool) ) );
            mnu->addAction(mactDisconnect);
          mactConnection->setMenu(mnu);
        mtbar->addAction(mactConnection);
        QToolButton *tbtn = static_cast<QToolButton *>( mtbar->widgetForAction(mactConnection) );
          tbtn->setPopupMode(QToolButton::InstantPopup);
          tbtn->setLayout(new QVBoxLayout);
          tbtn->layout()->setContentsMargins(0, 0, 0, 0);
            QLabel *lbl = new QLabel(tbtn);
            lbl->setAlignment(Qt::AlignCenter);
            QMovie *mov = new QMovie( BDirTools::findResource("icons/process.gif", BDirTools::GlobalOnly) );
            mov->setScaledSize( tbtn->iconSize() );
            lbl->setMovie(mov);
          tbtn->layout()->addWidget(lbl);
        mactUpdate = new QAction(this);
          mactUpdate->setEnabled( sClient->isAuthorized() );
          mactUpdate->setIcon( BApplication::icon("reload") );
          connect( mactUpdate, SIGNAL( triggered() ), this, SLOT( updateSamplesList() ) );
          connect( sClient, SIGNAL( authorizedChanged(bool) ), mactUpdate, SLOT( setEnabled(bool) ) );
        mtbar->addAction(mactUpdate);
        mactSend = new QAction(this);
          mactSend->setEnabled( sClient->isAuthorized() );
          mactSend->setIcon( BApplication::icon("mail_send") );
          connect( sClient, SIGNAL( authorizedChanged(bool) ), mactSend, SLOT( setEnabled(bool) ) );
          mnu = new QMenu;
            mactSendCurrent = new QAction(this);
              BCodeEditor *edr = Window->codeEditor();
              mactSendCurrent->setEnabled( edr->currentDocument() );
              mactSendCurrent->setIcon( Application::icon("tex") );
              connect( edr, SIGNAL( documentAvailableChanged(bool) ), mactSendCurrent, SLOT( setEnabled(bool) ) );
              connect( mactSendCurrent, SIGNAL( triggered() ), this, SLOT( actSendCurrentTriggreed() ) );
            mnu->addAction(mactSendCurrent);
            mactSendAll = new QAction(this);
              mactSendAll->setEnabled( edr->currentDocument() );
              mactSendAll->setIcon( Application::icon("document_multiple") );
              connect( edr, SIGNAL( documentAvailableChanged(bool) ), mactSendAll, SLOT( setEnabled(bool) ) );
              connect( mactSendAll, SIGNAL( triggered() ), this, SLOT( actSendAllTriggreed() ) );
            mnu->addAction(mactSendAll);
            mactSendExternal = new QAction(this);
              mactSendExternal->setIcon( Application::icon("fileopen") );
              connect( mactSendExternal, SIGNAL( triggered() ), this, SLOT( actSendExternalTriggreed() ) );
            mnu->addAction(mactSendExternal);
          mactSend->setMenu(mnu);
        mtbar->addAction(mactSend);
        static_cast<QToolButton *>( mtbar->widgetForAction(mactSend) )->setPopupMode(QToolButton::InstantPopup);
        mactTools = new QAction(this);
          mactTools->setIcon( Application::icon("configure") );
          mnu = new QMenu;
            mactSettings = new QAction(this);
              mactSettings->setIcon( Application::icon("configure") );
              connect( mactSettings, SIGNAL( triggered() ), this, SLOT( actSettingsTriggered() ) );
            mnu->addAction(mactSettings);
            mactAccountSettings = new QAction(this);
              mactAccountSettings->setEnabled( sClient->isAuthorized() );
              mactAccountSettings->setIcon( Application::icon("user") );
              connect( mactAccountSettings, SIGNAL( triggered() ), this, SLOT( actAccountSettingsTriggered() ) );
              connect( sClient, SIGNAL( authorizedChanged(bool) ), mactAccountSettings, SLOT( setEnabled(bool) ) );
            mnu->addAction(mactAccountSettings);
            mactAdministration = new QAction(this);
              mactAdministration->setEnabled(sClient->accessLevel() >= Client::AdminLevel);
              mactAdministration->setIcon( Application::icon("gear") );
              connect( mactAdministration, SIGNAL( triggered() ), this, SLOT( actAdministrationTriggered() ) );
            mnu->addAction(mactAdministration);
          mactTools->setMenu(mnu);
        mtbar->addAction(mactTools);
        static_cast<QToolButton *>( mtbar->widgetForAction(mactTools) )->setPopupMode(QToolButton::InstantPopup);
      vlt->addWidget(mtbar);
      mgboxSelect = new QGroupBox(this);
        QFormLayout *flt = new QFormLayout;
          mlblType = new QLabel;
          mcmboxType = new QComboBox;
            connect( mcmboxType, SIGNAL( currentIndexChanged(int) ),
                     this, SLOT( cmboxTypeCurrentIndexChanged(int) ) );
          flt->addRow(mlblType, mcmboxType);
          mlblSearch = new QLabel;
          QHBoxLayout *hlt = new QHBoxLayout;
            mledtSearch = new QLineEdit;
              BSignalDelayProxy *sdp = new BSignalDelayProxy(this);
              sdp->setStringConnection( mledtSearch, SIGNAL( textChanged(QString) ),
                                        mproxyModel, SLOT( setSearchKeywordsString(QString) ) );
            hlt->addWidget(mledtSearch);
          flt->addRow(mlblSearch, hlt);
        mgboxSelect->setLayout(flt);
      vlt->addWidget(mgboxSelect);
      mtblvw = new QTableView(this);
        mtblvw->setAlternatingRowColors(true);
        mtblvw->setEditTriggers(QTableView::NoEditTriggers);
        mtblvw->setSelectionBehavior(QTableView::SelectRows);
        mtblvw->setSelectionMode(QTableView::SingleSelection);
        mtblvw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        mtblvw->horizontalHeader()->setStretchLastSection(true);
        mtblvw->verticalHeader()->setVisible(false);
        mtblvw->setContextMenuPolicy(Qt::CustomContextMenu);
        mtblvw->setModel(mproxyModel);
        connect( mtblvw, SIGNAL( doubleClicked(QModelIndex) ), this, SLOT( tblvwDoubleClicked(QModelIndex) ) );
        connect( mtblvw, SIGNAL( customContextMenuRequested(QPoint) ),
                 this, SLOT( tblvwCustomContextMenuRequested(QPoint) ) );
      vlt->addWidget(mtblvw);
    //
    retranslateUi();
    connect( bApp, SIGNAL( languageChanged() ), this, SLOT( retranslateUi() ) );
    mcmboxType->setCurrentIndex( bSettings->value("SamplesWidget/samples_type_index", 0).toInt() );
}

SamplesWidget::~SamplesWidget()
{
    bSettings->setValue( "SamplesWidget/samples_type_index", mcmboxType->currentIndex() );
}

/*============================== Public methods ============================*/

QList<QAction *> SamplesWidget::toolBarActions() const
{
    QList<QAction *> list;
    list << mactConnection;
    list << mactUpdate;
    list << mactSend;
    list << mactTools;
    return list;
}

/*============================== Private methods ===========================*/

void SamplesWidget::retranslateCmboxType()
{
    mcmboxType->blockSignals(true);
    int ind = mcmboxType->currentIndex();
    if (ind < 0)
        ind = 0;
    mcmboxType->clear();
    mcmboxType->addItem(Sample::typeToLocalizedString(Sample::Approved, false), Sample::Approved);
    mcmboxType->addItem(Sample::typeToLocalizedString(Sample::Rejected, false), Sample::Rejected);
    mcmboxType->addItem(Sample::typeToLocalizedString(Sample::Unverified, false), Sample::Unverified);
    mcmboxType->addItem(tr("My", "cmbox item text"), SamplesProxyModel::CurrentUserSample);
    mcmboxType->setCurrentIndex(ind);
    mcmboxType->blockSignals(false);
}

void SamplesWidget::resetActConnection(const QString &toolTip, const QString &iconName, bool animated)
{
    QToolButton *tbtn = static_cast<QToolButton *>( mtbar->widgetForAction(mactConnection) );
    QLabel *lbl = tbtn->findChild<QLabel *>();
    mactConnection->setIcon( BApplication::icon(iconName) );
    mactConnection->setToolTip(toolTip);
    if (animated)
    {
        tbtn->setText("");
        tbtn->setIcon( QIcon() );
        lbl->setVisible(true);
        lbl->movie()->start();
    }
    else
    {
        lbl->movie()->stop();
        lbl->setVisible(false);
    }
}

/*============================== Private slots =============================*/

void SamplesWidget::retranslateUi()
{
    mactConnection->setText( tr("Connection", "act text") );
    mactConnection->setWhatsThis( tr("This action shows current connection state. "
                                     "Use it to connect or disconnect from the server", "act whatsThis") );
    clientStateChanged( bApp->clientInstance()->state() );
    mactConnect->setText( tr("Connect", "act text") );
    mactDisconnect->setText( tr("Disconnect", "act text") );
    mactUpdate->setText( tr("Update", "act text") );
    mactUpdate->setToolTip( tr("Update samples list", "act toolTip") );
    mactSend->setText( tr("Send sample", "act text") );
    mactSend->setToolTip( tr("Send sample", "act toolTip") );
    mactSendCurrent->setText( tr("Current document...", "act text") );
    mactSendAll->setText( tr("All opened documents...", "act text") );
    mactSendExternal->setText( tr("External files...", "act text") );
    mactTools->setText( tr("Tools", "act text") );
    mactTools->setToolTip( tr("Tools", "act toolTip") );
    mactSettings->setText( tr("TeXSample settings...", "act text") );
    mactAccountSettings->setText( tr("Account management...", "act text") );
    mactAdministration->setText( tr("Administration...", "act text") );
    //
    mgboxSelect->setTitle( tr("Selection", "gbox title") );
    //
    mlblType->setText( tr("Type:", "lbl text") );
    mlblSearch->setText( tr("Search:", "lbl text") );
    //
    retranslateCmboxType();
}

void SamplesWidget::actSendCurrentTriggreed()
{
    BCodeEditorDocument *doc = Window->codeEditor()->currentDocument();
    if (!doc)
        return;
    SendSamplesDialog(doc, this).exec();
}

void SamplesWidget::actSendAllTriggreed()
{
    QList<BCodeEditorDocument *> list = Window->codeEditor()->documents();
    if ( list.isEmpty() )
        return;
    SendSamplesDialog(list, this).exec();
}

void SamplesWidget::actSendExternalTriggreed()
{
    QStringList list;
    QTextCodec *codec = 0;
    if ( !Window->codeEditor()->driver()->getOpenFileNames(this, list, codec) ) //TODO: May open non-tex files!
        return;
    SendSamplesDialog(list, codec, this).exec();
}

void SamplesWidget::actSettingsTriggered()
{
    BSettingsDialog( new TexsampleSettingsTab, window() ).exec();
}

void SamplesWidget::actAccountSettingsTriggered()
{
    if ( !sClient->isAuthorized() )
        return;
    BSettingsDialog( new AccountSettingsTab, window() ).exec();
}

void SamplesWidget::actAdministrationTriggered()
{
    if (sClient->accessLevel() < Client::AdminLevel)
        return;
    AdministrationDialog( window() ).exec();
}

void SamplesWidget::clientStateChanged(Client::State state)
{
    switch (state)
    {
    case Client::DisconnectedState:
        resetActConnection(tr("Connection state: disconnected", "act toolTip"), "connect_no");
        break;
    case Client::ConnectingState:
        resetActConnection(tr("Connection state: connecting", "act toolTip"), "process", true);
        break;
    case Client::ConnectedState:
        resetActConnection(tr("Connection state: connected", "act toolTip"), "process", true);
        break;
    case Client::AuthorizedState:
        resetActConnection(tr("Connection state: authorized", "act toolTip"), "connect_established");
        break;
    case Client::DisconnectingState:
        resetActConnection(tr("Connection state: disconnecting", "act toolTip"), "process", true);
        break;
    default:
        break;
    }
}

void SamplesWidget::clientAccessLevelChanged(int lvl)
{
    mactAdministration->setEnabled(lvl >= Client::AdminLevel);
}

void SamplesWidget::cmboxTypeCurrentIndexChanged(int index)
{
    if (index < 0)
        return;
    mproxyModel->setSampleType( mcmboxType->itemData(index).toInt() );
}

void SamplesWidget::tblvwDoubleClicked(const QModelIndex &index)
{
    if ( !Window->codeEditor()->documentAvailable() )
        return;
    QModelIndex ind = mproxyModel->mapToSource(index);
    if ( !ind.isValid() )
        return;
    mlastId = sModel->indexAt( ind.row() );
    insertSample();
}

void SamplesWidget::tblvwCustomContextMenuRequested(const QPoint &pos)
{
    mlastId = sModel->indexAt( mproxyModel->mapToSource( mtblvw->indexAt(pos) ).row() );
    if (!mlastId)
        return;
    QMenu mnu;
    QAction *actInsert = mnu.addAction( tr("Insert", "act text") );
    actInsert->setEnabled( Window->codeEditor()->documentAvailable() );
    connect( actInsert, SIGNAL(triggered()), this, SLOT( insertSample() ) );
    mnu.addSeparator();
    QAction *actInfo = mnu.addAction( tr("Information...", "act text") );
    connect( actInfo, SIGNAL( triggered() ), this, SLOT( showSampleInfo() ) );
    QAction *actPreview = mnu.addAction( tr("Preview", "act text") );
    connect( actPreview, SIGNAL( triggered() ), this, SLOT( previewSample() ) );
    mnu.addSeparator();
    QAction *actDelete = mnu.addAction( tr("Delete", "act text") );
    connect( actDelete, SIGNAL( triggered() ), this, SLOT( deleteSample() ) );
    mnu.exec( mtblvw->mapToGlobal(pos) );
}

void SamplesWidget::updateSamplesList()
{
    QString errs;
    sClient->updateSamplesList(false, &errs, this);
}

void SamplesWidget::showSampleInfo()
{
    if (!mlastId)
        return;
    if ( minfoDialogMap.contains(mlastId) )
        return minfoDialogMap.value(mlastId)->activateWindow();
    const Sample *s = sModel->sample(mlastId);
    if (!s)
        return;
    SampleInfoDialog *sd = new SampleInfoDialog(s, this);
    sd->setAttribute(Qt::WA_DeleteOnClose, true);
    minfoDialogMap.insert(mlastId, sd);
    minfoDialogIdMap.insert(sd, mlastId);
    connect( sd, SIGNAL( destroyed(QObject *) ), this, SLOT( infoDialogDestroyed(QObject *) ) );
    sd->show();
}

void SamplesWidget::previewSample()
{
    if (!mlastId)
        return;
    sClient->previewSample(mlastId);
}

void SamplesWidget::insertSample()
{
    if (!mlastId)
        return;
    sClient->insertSample( mlastId, Window->codeEditor() );
}

void SamplesWidget::deleteSample()
{
    if (!mlastId)
        return;
    sClient->deleteSample( mlastId, Window->codeEditor() );
}

void SamplesWidget::infoDialogDestroyed(QObject *obj)
{
    minfoDialogMap.remove( minfoDialogIdMap.take(obj) );
}
