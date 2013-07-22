#include "texsamplewidget.h"
#include "samplesproxymodel.h"
#include "samplesmodel.h"
#include "application.h"
#include "client.h"
#include "texsamplesettingstab.h"
#include "mainwindow.h"
#include "samplewidget.h"
#include "invitesdialog.h"
#include "userwidget.h"
#include "global.h"
#include "recoverydialog.h"

#include <TSampleInfo>
#include <TOperationResult>
#include <TUserInfo>
#include <TAccessLevel>
#include <TCompilationResult>

#include <BApplication>
#include <BSettingsDialog>
#include <BDirTools>
#include <BSignalDelayProxy>
#include <BNetworkOperation>
#include <BCodeEditor>
#include <BCodeEditorDocument>
#include <BAbstractDocumentDriver>
#include <BAbstractSettingsTab>
#include <BeQtGlobal>
#include <BDialog>
#include <BFileDialog>

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
#include <QInputDialog>
#include <QMessageBox>
#include <QIcon>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QWidgetAction>
#include <QList>
#include <QMetaObject>
#include <QScrollArea>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTimer>

#include <QDebug>

/*============================================================================
================================ ConnectionAction ============================
============================================================================*/

class ConnectionAction : public QWidgetAction
{
public:
    explicit ConnectionAction(QObject *parent);
public:
    void resetIcon(const QString &toolTip, const QString &iconName, bool animated = false);
protected:
    QWidget *createWidget(QWidget *parent);
    void deleteWidget(QWidget *widget);
};

/*============================================================================
================================ AddSampleDialog =============================
============================================================================*/

/*============================== Public constructors =======================*/

AddSampleDialog::AddSampleDialog(BCodeEditor *editor, QWidget *parent) :
    BDialog(parent)
{
    msmpwgt = new SampleWidget(SampleWidget::AddMode, editor);
    msmpwgt->setCheckSourceValidity(true);
    setWindowTitle(tr("Sending sample..."));
    msmpwgt->setInfo(bSettings->value("AddSampleDialog/sample_widget_info").value<TSampleInfo>());
    msmpwgt->restoreState(bSettings->value("TexsampleWidget/sample_widget_state").toByteArray());
    msmpwgt->restoreSourceState(bSettings->value("AddSampleDialog/sample_widget_source_state").toByteArray());
    setWidget(msmpwgt);
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    addButton(tr("Clear", "btn text"), QDialogButtonBox::ActionRole, msmpwgt, SLOT(clear()));
    button(QDialogButtonBox::Ok)->setEnabled(msmpwgt->isValid());
    connect(msmpwgt, SIGNAL(validityChanged(bool)), button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
    connect(button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    restoreGeometry(bSettings->value("AddSampleDialog/geometry").toByteArray());
}

/*============================== Public methods ============================*/

SampleWidget *AddSampleDialog::sampleWidget() const
{
    return msmpwgt;
}

/*============================== Protected methods =========================*/

void AddSampleDialog::closeEvent(QCloseEvent *e)
{
    bSettings->setValue("AddSampleDialog/sample_widget_info", msmpwgt->info());
    bSettings->setValue("TexsampleWidget/sample_widget_state", msmpwgt->saveState());
    bSettings->setValue("AddSampleDialog/sample_widget_source_state", msmpwgt->saveSourceState());
    bSettings->setValue("AddSampleDialog/geometry", saveGeometry());
    e->accept();
}

/*============================================================================
================================ EditSampleDialog ============================
============================================================================*/

/*============================== Public constructors =======================*/

EditSampleDialog::EditSampleDialog(BCodeEditor *editor, quint64 id, QWidget *parent) :
    BDialog(parent)
{
    if (!id)
        return;
    bool moder = sClient->accessLevel() >= TAccessLevel::ModeratorLevel;
    const TSampleInfo *info = sModel->sample(id);
    if (!info)
        return;
    msmpwgt = new SampleWidget(moder ? SampleWidget::EditMode : SampleWidget::UpdateMode, editor);
    setWindowTitle((moder ? tr("Editing sample:") : tr("Updating sample:")) + " " + info->title());
    msmpwgt->restoreState(bSettings->value("TexsampleWidget/sample_widget_state").toByteArray());
    msmpwgt->setInfo(*info);
    setWidget(msmpwgt);
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    addButton(tr("Clear", "btn text"), QDialogButtonBox::ActionRole, msmpwgt, SLOT(clear()));
    button(QDialogButtonBox::Ok)->setEnabled(msmpwgt->isValid());
    connect(msmpwgt, SIGNAL(validityChanged(bool)), button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
    connect(button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    restoreGeometry(bSettings->value("EditSampleDialog/geometry").toByteArray());
}

/*============================== Public methods ============================*/

SampleWidget *EditSampleDialog::sampleWidget() const
{
    return msmpwgt;
}

/*============================== Protected methods =========================*/

void EditSampleDialog::closeEvent(QCloseEvent *e)
{
    bSettings->setValue("TexsampleWidget/sample_widget_state", msmpwgt->saveState());
    bSettings->setValue("EditSampleDialog/geometry", saveGeometry());
    e->accept();
}

/*============================================================================
================================ ConnectionAction ============================
============================================================================*/

/*============================== Public constructors =======================*/

ConnectionAction::ConnectionAction(QObject *parent) :
    QWidgetAction(parent)
{
    //
}

/*============================== Public methods ============================*/

void ConnectionAction::resetIcon(const QString &toolTip, const QString &iconName, bool animated)
{
    setToolTip(toolTip);
    setIcon(BApplication::icon(iconName));
    foreach (QWidget *wgt, createdWidgets())
    {
        if (QString("QLabel") != wgt->metaObject()->className())
        {
            QToolButton *tbtn = static_cast<QToolButton *>(wgt);
            tbtn->setIcon(BApplication::icon(iconName));
            tbtn->setToolTip(toolTip);
            QLabel *lbl = tbtn->findChild<QLabel *>();
            if (animated)
            {
                tbtn->setText("");
                tbtn->setIcon(QIcon());
                lbl->setVisible(true);
                lbl->movie()->start();
            }
            else
            {
                lbl->movie()->stop();
                lbl->setVisible(false);
            }
        }
    }
}

/*============================== Protected methods =========================*/

QWidget *ConnectionAction::createWidget(QWidget *parent)
{
    if (!parent || QString("QMenu") == parent->metaObject()->className())
        return 0;
    QToolButton *tbtn = new QToolButton(parent);
      tbtn->setMenu(this->menu());
      tbtn->setPopupMode(QToolButton::InstantPopup);
      tbtn->setLayout(new QVBoxLayout);
      tbtn->layout()->setContentsMargins(0, 0, 0, 0);
        QLabel *lbl = new QLabel(tbtn);
        lbl->setAlignment(Qt::AlignCenter);
        QMovie *mov = new QMovie(BDirTools::findResource("icons/process.gif", BDirTools::GlobalOnly));
        mov->setScaledSize(tbtn->iconSize());
        lbl->setMovie(mov);
      tbtn->layout()->addWidget(lbl);
    return tbtn;
}

void ConnectionAction::deleteWidget(QWidget *widget)
{
    if (!widget)
        return;
    widget->deleteLater();
}

/*============================================================================
================================ TexsampleWidget =============================
============================================================================*/

/*============================== Public constructors =======================*/

TexsampleWidget::TexsampleWidget(MainWindow *window, QWidget *parent) :
    QWidget(parent), Window(window)
{
    mlastId = 0;
    mproxyModel = new SamplesProxyModel(this);
    mproxyModel->setSourceModel(sModel);
    connect( sClient, SIGNAL( stateChanged(Client::State) ), this, SLOT( clientStateChanged(Client::State) ) );
    connect( sClient, SIGNAL( accessLevelChanged(int) ), this, SLOT( clientAccessLevelChanged(int) ) );
    //
    mtbarIndicator = new QToolBar;
    //
    QVBoxLayout *vlt = new QVBoxLayout(this);
      mtbar = new QToolBar(this);
        mactConnection = new ConnectionAction(this);
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
        mtbarIndicator->addAction(mactConnection);
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
          connect(mactSend, SIGNAL(triggered()), this, SLOT(addSample()));
          mnu = new QMenu;
            mactSendCurrent = new QAction(this);
              BCodeEditor *edr = Window->codeEditor();
              mactSendCurrent->setEnabled(edr->documentAvailable());
              mactSendCurrent->setIcon( Application::icon("tex") );
              connect( edr, SIGNAL( documentAvailableChanged(bool) ), mactSendCurrent, SLOT( setEnabled(bool) ) );
              connect(mactSendCurrent, SIGNAL(triggered()), this, SLOT(addSampleCurrentFile()));
            mnu->addAction(mactSendCurrent);
            mactSendExternal = new QAction(this);
              mactSendExternal->setIcon( Application::icon("fileopen") );
              connect(mactSendExternal, SIGNAL(triggered()), this, SLOT(addSampleExternalFile()));
            mnu->addAction(mactSendExternal);
          mactSend->setMenu(mnu);
        mtbar->addAction(mactSend);
        mactTools = new QAction(this);
          mactTools->setIcon( Application::icon("configure") );
          mnu = new QMenu;
            mactRegister = new QAction(this);
              mactRegister->setIcon( Application::icon("add_user") );
              connect( mactRegister, SIGNAL( triggered() ), this, SLOT( actRegisterTriggered() ) );
            mnu->addAction(mactRegister);
            mnu->addSeparator();
            mactRecover = new QAction(this);
              mactRecover->setIcon(Application::icon("account_recover"));
              connect(mactRecover, SIGNAL(triggered()), this, SLOT(actRecoverTriggered()));
            mnu->addAction(mactRecover);
            mnu->addSeparator();
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
            mnu->addSeparator();
            mactAdministration = new QAction(this);
              mactAdministration->setEnabled(sClient->accessLevel() >= TAccessLevel::ModeratorLevel);
              mactAdministration->setIcon( Application::icon("gear") );
              QMenu *submnu = new QMenu;
                mactAddUser = submnu->addAction(Application::icon("add_user"), "", this, SLOT(actAddUserTriggered()));
                  mactAddUser->setEnabled(sClient->accessLevel() >= TAccessLevel::AdminLevel);
                mactEditUser = submnu->addAction(Application::icon("edit_user"), "", this, SLOT(actEditUserTriggered()));
                  mactEditUser->setEnabled(sClient->accessLevel() >= TAccessLevel::AdminLevel);
                mactInvites = submnu->addAction(Application::icon("mail_send"), "", this, SLOT(actInvitesTriggered()));
            mactAdministration->setMenu(submnu);
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
    mcmboxType->setCurrentIndex( bSettings->value("TexsampleWidget/samples_type_index", 0).toInt() );
    mtblvw->horizontalHeader()->restoreState(bSettings->value("TexsampleWidget/table_header_state").toByteArray());
}

TexsampleWidget::~TexsampleWidget()
{
    bSettings->setValue("TexsampleWidget/samples_type_index", mcmboxType->currentIndex());
    bSettings->setValue("TexsampleWidget/table_header_state", mtblvw->horizontalHeader()->saveState());
}

/*============================== Public methods ============================*/

QList<QAction *> TexsampleWidget::toolBarActions() const
{
    QList<QAction *> list;
    list << mactConnection;
    list << mactUpdate;
    list << mactSend;
    list << mactTools;
    return list;
}

QWidget *TexsampleWidget::indicator() const
{
    return mtbarIndicator;
}

/*============================== Private methods ===========================*/

void TexsampleWidget::retranslateCmboxType()
{
    mcmboxType->blockSignals(true);
    int ind = mcmboxType->currentIndex();
    if (ind < 0)
        ind = 0;
    mcmboxType->clear();
    mcmboxType->addItem(TSampleInfo::typeToString(TSampleInfo::Approved, false), TSampleInfo::Approved);
    mcmboxType->addItem(TSampleInfo::typeToString(TSampleInfo::Rejected, false), TSampleInfo::Rejected);
    mcmboxType->addItem(TSampleInfo::typeToString(TSampleInfo::Unverified, false), TSampleInfo::Unverified);
    mcmboxType->addItem(tr("My", "cmbox item text"), SamplesProxyModel::CurrentUserSample);
    mcmboxType->setCurrentIndex(ind);
    mcmboxType->blockSignals(false);
}

void TexsampleWidget::showAddingSampleFailedMessage(const QString &errorString)
{
    QMessageBox msg(this);
    msg.setWindowTitle(tr("Sending sample error", "msgbox windowTitle"));
    msg.setIcon(QMessageBox::Critical);
    msg.setText(tr("Failed to send sample due to the following error:", "msgbox text"));
    msg.setInformativeText(errorString);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    msg.exec();
}

void TexsampleWidget::showEditingSampleFailedMessage(const QString &errorString)
{
    QMessageBox msg(this);
    msg.setWindowTitle(tr("Editing sample error", "msgbox windowTitle"));
    msg.setIcon(QMessageBox::Critical);
    msg.setText(tr("Failed to edit sample due to the following error:", "msgbox text"));
    msg.setInformativeText(errorString);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    msg.exec();
}

/*============================== Private slots =============================*/

void TexsampleWidget::retranslateUi()
{
    mactConnection->setText( tr("Connection", "act text") );
    mactConnection->setWhatsThis( tr("This action shows current connection state. "
                                     "Use it to connect or disconnect from the server", "act whatsThis") );
    clientStateChanged(sClient->state());
    mactConnect->setText( tr("Connect", "act text") );
    mactDisconnect->setText( tr("Disconnect", "act text") );
    mactUpdate->setText( tr("Update", "act text") );
    mactUpdate->setToolTip( tr("Update samples list", "act toolTip") );
    mactSend->setText( tr("Send sample", "act text") );
    mactSend->setToolTip( tr("Send sample...", "act toolTip") );
    mactSendCurrent->setText( tr("Current document...", "act text") );
    mactSendExternal->setText( tr("External file...", "act text") );
    mactTools->setText( tr("Tools", "act text") );
    mactTools->setToolTip( tr("Tools", "act toolTip") );
    mactRegister->setText( tr("Register...", "act tooTip") );
    mactRecover->setText(tr("Recover account...", "act text"));
    mactSettings->setText( tr("TeXSample settings...", "act text") );
    mactAccountSettings->setText( tr("Account management...", "act text") );
    mactAdministration->setText( tr("Administration...", "act text") );
    mactAddUser->setText(tr("Add user...", "act text"));
    mactEditUser->setText(tr("Edit user...", "act text"));
    mactInvites->setText(tr("Manage invites...", "act text"));
    //
    mgboxSelect->setTitle( tr("Selection", "gbox title") );
    //
    mlblType->setText( tr("Type:", "lbl text") );
    mlblSearch->setText( tr("Search:", "lbl text") );
    //
    retranslateCmboxType();
}

void TexsampleWidget::actSettingsTriggered()
{
    BSettingsDialog( new TexsampleSettingsTab, window() ).exec();
}

void TexsampleWidget::actRegisterTriggered()
{
    if (!Application::showRegisterDialog(Window))
        return;
    emit message(tr("You have successfully registered", "message"));
    bool b = sClient->isAuthorized();
    sClient->updateSettings();
    if (!b)
        sClient->connectToServer();
}

void TexsampleWidget::actRecoverTriggered()
{
    RecoveryDialog(this).exec();
}

void TexsampleWidget::actAccountSettingsTriggered()
{
    if (!sClient->isAuthorized())
        return;
    Application::showSettings(Application::AccountSettings, window());
}

void TexsampleWidget::actAddUserTriggered()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Adding user", "dlg windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      UserWidget *uwgt = new UserWidget(UserWidget::AddMode);
      vlt->addWidget(uwgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Ok);
        dlgbbox->button(QDialogButtonBox::Ok)->setEnabled(uwgt->isValid());
        connect(uwgt, SIGNAL(validityChanged(bool)), dlgbbox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
        connect(dlgbbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dlg, SLOT(accept()));
        dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(dlgbbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dlg, SLOT(reject()));
      vlt->addWidget(dlgbbox);
      dlg.setFixedHeight(dlg.sizeHint().height());
      dlg.setMinimumWidth(600);
    while (dlg.exec() == QDialog::Accepted)
    {
        TUserInfo info = uwgt->info();
        TOperationResult r = sClient->addUser(info, this);
        if (r)
        {
            emit message(tr("User was successfully added", "message"));
            return;
        }
        else
        {
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Adding user error", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to add user due to the following error:", "msgbox text"));
            msg.setInformativeText(r.messageString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
        }
    }
}

void TexsampleWidget::actEditUserTriggered()
{
    bool ok = false;
    quint64 id = (quint64) QInputDialog::getInt(this, tr("Enter user ID", "idlg title"), tr("User ID:", "idlg label"),
                                                1, 1, 2147483647, 1, &ok);
    if (!ok)
        return;
    if (sClient->userId() == id)
    {
        //TODO: show message
        return;
    }
    TUserInfo info;
    if (!sClient->getUserInfo(id, info, this))
        return;
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Editing user", "dlg windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      UserWidget *uwgt = new UserWidget(UserWidget::EditMode);
        uwgt->setInfo(info);
      vlt->addWidget(uwgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Ok);
        dlgbbox->button(QDialogButtonBox::Ok)->setEnabled(uwgt->isValid());
        connect(uwgt, SIGNAL(validityChanged(bool)), dlgbbox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
        connect(dlgbbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dlg, SLOT(accept()));
        dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(dlgbbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dlg, SLOT(reject()));
      vlt->addWidget(dlgbbox);
      dlg.setFixedHeight(dlg.sizeHint().height());
      dlg.setMinimumWidth(600);
    while (dlg.exec() == QDialog::Accepted)
    {
        TUserInfo info = uwgt->info();
        TOperationResult r = sClient->editUser(info, this);
        if (r)
        {
            emit message(tr("User info was successfully edited", "message"));
            return;
        }
        else
        {
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Editing user error", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to edit user due to the following error:", "msgbox text"));
            msg.setInformativeText(r.messageString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
        }
    }
}

void TexsampleWidget::actInvitesTriggered()
{
    InvitesDialog(this).exec();
}

void TexsampleWidget::clientStateChanged(Client::State state)
{
    QString s = tr("TeXSample state:", "act toolTip") + " ";
    switch (state)
    {
    case Client::DisconnectedState:
        mactConnection->resetIcon(s + tr("disconnected", "act toolTip"), "connect_no");
        break;
    case Client::ConnectingState:
        mactConnection->resetIcon(s + tr("connecting", "act toolTip"), "process", true);
        break;
    case Client::ConnectedState:
        mactConnection->resetIcon(s + tr("connected", "act toolTip"), "process", true);
        break;
    case Client::AuthorizedState:
        mactConnection->resetIcon(s + tr("authorized", "act toolTip"), "connect_established");
        break;
    case Client::DisconnectingState:
        mactConnection->resetIcon(s + tr("disconnecting", "act toolTip"), "process", true);
        break;
    default:
        break;
    }
}

void TexsampleWidget::clientAccessLevelChanged(int lvl)
{
    mactAdministration->setEnabled(lvl >= TAccessLevel::ModeratorLevel);
    mactAddUser->setEnabled(lvl >= TAccessLevel::AdminLevel);
    mactEditUser->setEnabled(lvl >= TAccessLevel::AdminLevel);
}

void TexsampleWidget::cmboxTypeCurrentIndexChanged(int index)
{
    if (index < 0)
        return;
    mproxyModel->setSampleType( mcmboxType->itemData(index).toInt() );
}

void TexsampleWidget::tblvwDoubleClicked(const QModelIndex &index)
{
    if ( !Window->codeEditor()->documentAvailable() )
        return;
    QModelIndex ind = mproxyModel->mapToSource(index);
    if ( !ind.isValid() )
        return;
    mlastId = sModel->indexAt( ind.row() );
    insertSample();
}

void TexsampleWidget::tblvwCustomContextMenuRequested(const QPoint &pos)
{
    mlastId = sModel->indexAt( mproxyModel->mapToSource( mtblvw->indexAt(pos) ).row() );
    if (!mlastId)
        return;
    QMenu mnu;
    QAction *act = mnu.addAction(tr("Insert...", "act text"), this, SLOT(insertSample()));
      act->setEnabled(sClient->isAuthorized() && Window->codeEditor()->documentAvailable());
      act->setIcon(Application::icon("editpaste"));
    act = mnu.addAction(tr("Save...", "act text"), this, SLOT(saveSample()));
      act->setEnabled(sClient->isAuthorized());
      act->setIcon(Application::icon("filesave"));
    mnu.addSeparator();
    act = mnu.addAction(tr("Information...", "act text"), this, SLOT(showSampleInfo()));
      act->setIcon(Application::icon("help_about"));
    act = mnu.addAction(tr("Preview", "act text"), this, SLOT(previewSample()));
      act->setEnabled(sClient->isAuthorized());
      act->setIcon(Application::icon("pdf"));
    mnu.addSeparator();
    act = mnu.addAction(tr("Edit...", "act text"), this, SLOT(editSample()));
      act->setIcon(Application::icon("edit"));
      bool ownEditable = sModel->sample(mlastId) && sModel->sample(mlastId)->sender().login() == sClient->login()
                           && sModel->sample(mlastId)->type() != TSampleInfo::Approved;
      act->setEnabled(sClient->isAuthorized()
                      && (ownEditable || sClient->accessLevel() >= TAccessLevel::ModeratorLevel));
    act = mnu.addAction(tr("Delete...", "act text"), this, SLOT(deleteSample()));
      act->setEnabled(sClient->isAuthorized() && (ownEditable || sClient->accessLevel() >= TAccessLevel::AdminLevel));
      act->setIcon(Application::icon("editdelete"));
    mnu.exec(mtblvw->mapToGlobal(pos));
}

void TexsampleWidget::updateSamplesList()
{
    sClient->updateSamplesList(false, this);
}

void TexsampleWidget::showSampleInfo()
{
    if (!mlastId)
        return;
    if (minfoDialogMap.contains(mlastId))
        return minfoDialogMap.value(mlastId)->activateWindow();
    const TSampleInfo *s = sModel->sample(mlastId);
    if (!s)
        return;
    QDialog *dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("Sample:", "windowTitle") + " " + s->title());
    QVBoxLayout *vlt = new QVBoxLayout(dlg);
      SampleWidget *swgt = new SampleWidget(SampleWidget::ShowMode);
        swgt->setInfo(*s);
      vlt->addWidget(swgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Close);
        connect(dlgbbox->button(QDialogButtonBox::Close), SIGNAL(clicked()), dlg, SLOT(close()));
      vlt->addWidget(dlgbbox);
      dlg->resize(bSettings->value("TexsampleWidget/sample_info_dialog_size", QSize(750, 550)).toSize());
    minfoDialogMap.insert(mlastId, dlg);
    minfoDialogIdMap.insert(dlg, mlastId);
    connect(dlg, SIGNAL(finished(int)), this, SLOT(infoDialogFinished()));
    dlg->show();
}

void TexsampleWidget::previewSample()
{
    if (!mlastId)
        return;
    if ( !sClient->previewSample(mlastId) )
    {
        QMessageBox msg( window() );
        msg.setWindowTitle( tr("Failed to show preview", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to get or show sample preview", "msgbox text") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
}

void TexsampleWidget::insertSample()
{
    if (!mlastId)
        return;
    BAbstractCodeEditorDocument *doc = Window->codeEditor()->currentDocument();
    if (!doc)
        return;
    QFileInfo fi(doc->fileName());
    QString subdir = fi.path() + "/texsample-" + QString::number(mlastId);
    if (!fi.isAbsolute() || !fi.isFile() || !BDirTools::mkpath(subdir))
        return;
    BFileDialog dlg(fi.path(), this);
    dlg.selectFile(subdir);
    //TODO: save geometry and state
    dlg.resize(700, 400);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::Directory);
    bool b = dlg.exec() == BFileDialog::Accepted;
    QStringList files = dlg.selectedFiles();
    b = b && !files.isEmpty();
    if (!b)
        return bRet(BDirTools::rmdir(subdir));
    subdir = files.first().remove(fi.path() + "/");
    /*if (!spath.isEmpty())
    {
        QMessageBox msg(edr);
        msg.setWindowTitle( tr("Reloading sample", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Question);
        msg.setText(tr("It seems like this sample is already in the target directory", "msgbox text"));
        msg.setInformativeText(tr("Do you want to download it again, or use existing version?",
                                  "magbox informativeText"));
        msg.addButton(tr("Download", "btn text"), QMessageBox::AcceptRole);
        QPushButton *btnEx = msg.addButton(tr("Use existing", "btn text"), QMessageBox::AcceptRole);
        msg.setDefaultButton(btnEx);
        msg.addButton(QMessageBox::Cancel);
        if (msg.exec() == QMessageBox::Cancel)
            return TOperationResult(tr("", ""));
        if (msg.clickedButton() == btnEx)
            return TOperationResult(insertSample(doc, id, sampleSourceFileName(spath)));
    }*/
    if (!sClient->insertSample(mlastId, doc, subdir))
    {
        QMessageBox msg( window() );
        msg.setWindowTitle( tr("Failed to insert sample", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to get or insert sample preview", "msgbox text") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    emit message(tr("Sample was successfully inserted", "message"));
}

void TexsampleWidget::saveSample()
{
    if (!mlastId)
        return;
    const TSampleInfo *info = sModel->sample(mlastId);
    if (!info)
        return;
    QString fn = QFileDialog::getSaveFileName(this, "Select directory", QDir::homePath() + "/" + info->fileName());
    if (fn.isEmpty())
        return;
    if (!sClient->saveSample(mlastId, fn))
    {
        QMessageBox msg( window() );
        msg.setWindowTitle( tr("Failed to save sample", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to get or save sample", "msgbox text") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    emit message(tr("Sample was successfully saved", "message"));
}

void TexsampleWidget::addSample()
{
    if (!maddDialog.isNull())
        return maddDialog->activateWindow();
    maddDialog = new AddSampleDialog(Window->codeEditor(), this);
    maddDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(maddDialog.data(), SIGNAL(finished(int)), this, SLOT(addDialogFinished()));
    maddDialog->show();
}

void TexsampleWidget::addSampleCurrentFile()
{
    if (!maddDialog.isNull())
        return maddDialog->activateWindow();
    maddDialog = new AddSampleDialog(Window->codeEditor(), this);
    maddDialog->setAttribute(Qt::WA_DeleteOnClose);
    maddDialog->sampleWidget()->setupFromCurrentDocument();
    connect(maddDialog.data(), SIGNAL(finished(int)), this, SLOT(addDialogFinished()));
    maddDialog->show();
}

void TexsampleWidget::addSampleExternalFile()
{
    if (!maddDialog.isNull())
        return maddDialog->activateWindow();
    QString fn;
    QTextCodec *c = Window->codeEditor()->defaultCodec();
    if (!SampleWidget::showSelectSampleDialog(fn, c, this))
        return;
    maddDialog = new AddSampleDialog(Window->codeEditor(), this);
    maddDialog->setAttribute(Qt::WA_DeleteOnClose);
    maddDialog->sampleWidget()->setupFromExternalFile(fn, c);
    connect(maddDialog.data(), SIGNAL(finished(int)), this, SLOT(addDialogFinished()));
    maddDialog->show();
}

void TexsampleWidget::editSample()
{
    if (!mlastId)
        return;
    if (meditDialogMap.contains(mlastId))
        return meditDialogMap.value(mlastId)->activateWindow();
    const TSampleInfo *s = sModel->sample(mlastId);
    if (!s)
        return;
    EditSampleDialog *dlg = new EditSampleDialog(Window->codeEditor(), mlastId, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    meditDialogMap.insert(mlastId, dlg);
    meditDialogIdMap.insert(dlg, mlastId);
    connect(dlg, SIGNAL(finished(int)), this, SLOT(editDialogFinished()));
    dlg->show();
}

void TexsampleWidget::deleteSample()
{
    if (!mlastId)
        return;
    QString title = tr("Deleting sample", "idlg title");
    QString lblText = tr("You are going to delete a sample. Please, enter the reason:", "idlg lblText");
    bool ok = false;
    QString reason = QInputDialog::getText(Window->codeEditor(), title, lblText, QLineEdit::Normal, QString(), &ok);
    if (!ok)
        return;
    TOperationResult r = sClient->deleteSample(mlastId, reason, Window->codeEditor());
    if (!r)
    {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Deleting sample error", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Failed to delete sample due to the following error:", "msgbox text"));
        msg.setInformativeText(r.messageString());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return;
    }
    emit message(tr("Sample was successfully deleted", "message"));
}

void TexsampleWidget::infoDialogFinished()
{
    QDialog *dlg = qobject_cast<QDialog *>(sender());
    if (!dlg)
        return;
    bSettings->setValue("TexsampleWidget/sample_info_dialog_size", dlg->size());
    minfoDialogMap.remove(minfoDialogIdMap.take(dlg));
    dlg->deleteLater();
}

void TexsampleWidget::addDialogFinished()
{
    if (maddDialog.isNull())
        return;
    if (maddDialog->result() == AddSampleDialog::Accepted)
    {
        SampleWidget *smpwgt = maddDialog->sampleWidget();
        QString fn = smpwgt->actualFileName();
        QString text = smpwgt->document() ? smpwgt->document()->text() : BDirTools::readTextFile(fn);
        TCompilationResult r = sClient->addSample(smpwgt->info(), fn, smpwgt->codec(), text, this);
        if (!r)
            return showAddingSampleFailedMessage(r.messageString());
        emit message(tr("Sample was successfully sent", "message"));
    }
    maddDialog->close();
    maddDialog->deleteLater();
}

void TexsampleWidget::editDialogFinished()
{
    EditSampleDialog *dlg = qobject_cast<EditSampleDialog *>(sender());
    if (!dlg)
        return;
    if (dlg->result() == EditSampleDialog::Accepted)
    {
        bool moder = sClient->accessLevel() >= TAccessLevel::ModeratorLevel;
        TSampleInfo info = dlg->sampleWidget()->info();
        SampleWidget *smpwgt = dlg->sampleWidget();
        QString fn = smpwgt->actualFileName();
        QString text = smpwgt->document() ? smpwgt->document()->text() : BDirTools::readTextFile(fn);
        TCompilationResult r = moder ? sClient->editSample(info, fn, smpwgt->codec(), text, this) :
                                       sClient->updateSample(info, fn, smpwgt->codec(), text, this);
        if (!r)
            return showEditingSampleFailedMessage(r.messageString());
        if (moder)
            emit message(tr("Sample was successfully edited", "message"));
        else
            emit message(tr("Sample was successfully updated", "message"));
    }
    meditDialogMap.remove(meditDialogIdMap.take(dlg));
    dlg->close();
    dlg->deleteLater();
}
