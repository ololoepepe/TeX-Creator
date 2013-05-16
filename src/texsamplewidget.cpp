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

#include <QDebug>

/*============================================================================
================================ TexsampleWidget ===============================
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
            mactRegister = new QAction(this);
              mactRegister->setIcon( Application::icon("add_user") );
              connect( mactRegister, SIGNAL( triggered() ), this, SLOT( actRegisterTriggered() ) );
            mnu->addAction(mactRegister);
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        mtblvw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#endif
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
}

TexsampleWidget::~TexsampleWidget()
{
    bSettings->setValue( "TexsampleWidget/samples_type_index", mcmboxType->currentIndex() );
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

void TexsampleWidget::resetActConnection(const QString &toolTip, const QString &iconName, bool animated)
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
    mactSend->setToolTip( tr("Send sample", "act toolTip") );
    mactSendCurrent->setText( tr("Current document...", "act text") );
    mactSendExternal->setText( tr("External file...", "act text") );
    mactTools->setText( tr("Tools", "act text") );
    mactTools->setToolTip( tr("Tools", "act toolTip") );
    mactRegister->setText( tr("Register...", "act tooTip") );
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

void TexsampleWidget::actSendCurrentTriggreed()
{
    BCodeEditorDocument *doc = Window->codeEditor()->currentDocument();
    if (!doc)
        return;
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Sending current file", "windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      SampleWidget *swgt = new SampleWidget(SampleWidget::AddMode);
        swgt->setFileName(doc->fileName());
      vlt->addWidget(swgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Ok);
        dlgbbox->button(QDialogButtonBox::Ok)->setEnabled(false);
        dlgbbox->button(QDialogButtonBox::Ok)->setDefault(true);
        connect(swgt, SIGNAL(validityChanged(bool)), dlgbbox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
        connect(dlgbbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dlg, SLOT(accept()));
        dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(dlgbbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dlg, SLOT(reject()));
      vlt->addWidget(dlgbbox);
      dlg.setFixedSize(dlg.sizeHint());
    if (dlg.exec() != QDialog::Accepted)
        return;
    TCompilationResult r = sClient->addSample(doc->fileName(), doc->codec(), doc->text(), swgt->info(), this);
    if (!r)
    {
        //TODO: Show message
    }
}

void TexsampleWidget::actSendExternalTriggreed()
{
    QStringList list;
    QTextCodec *codec = 0;
    //TODO: Open only 1 file at a time
    if (!Window->codeEditor()->driver()->getOpenFileNames(this, list, codec)) //TODO: May open non-tex files!
        return;
    if (list.size() != 1)
        return;
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Sending external file", "windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      SampleWidget *swgt = new SampleWidget(SampleWidget::AddMode);
        swgt->setFileName(list.first());
      vlt->addWidget(swgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Ok);
        dlgbbox->button(QDialogButtonBox::Ok)->setEnabled(false);
        connect(swgt, SIGNAL(validityChanged(bool)), dlgbbox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
        connect(dlgbbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dlg, SLOT(accept()));
        dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(dlgbbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dlg, SLOT(reject()));
      vlt->addWidget(dlgbbox);
      dlg.setFixedSize(dlg.sizeHint());
    if (dlg.exec() != QDialog::Accepted)
        return;
    TCompilationResult r = sClient->addSample(list.first(), codec, swgt->info(), this);
    if (!r)
    {
        //TODO: Show message
    }
}

void TexsampleWidget::actSettingsTriggered()
{
    BSettingsDialog( new TexsampleSettingsTab, window() ).exec();
}

void TexsampleWidget::actRegisterTriggered()
{
    if (!Application::showRegisterDialog(Window))
        return;
    bool b = sClient->isAuthorized();
    sClient->updateSettings();
    if (!b)
        sClient->connectToServer();
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
      dlg.setFixedSize(640, 240);
    while (dlg.exec() == QDialog::Accepted)
    {
        TUserInfo info = uwgt->info();
        TOperationResult r = sClient->addUser(info, this);
        if (r)
        {
            return;
        }
        else
        {
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Adding user error", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to add user due to the following error:", "msgbox text"));
            msg.setInformativeText(r.errorString());
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
                                                (int) sClient->userId(), 1, 2147483647, 1, &ok);
    if (!ok)
        return;
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
      dlg.setFixedSize(640, 240);
    while (dlg.exec() == QDialog::Accepted)
    {
        TUserInfo info = uwgt->info();
        TOperationResult r = sClient->editUser(info, this);
        if (r)
        {
            return;
        }
        else
        {
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Editing user error", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to edit user due to the following error:", "msgbox text"));
            msg.setInformativeText(r.errorString());
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
    QAction *act = mnu.addAction( tr("Insert", "act text") );
      act->setEnabled(sClient->isAuthorized() && Window->codeEditor()->documentAvailable());
      act->setIcon( Application::icon("editpaste") );
      connect( act, SIGNAL( triggered() ), this, SLOT( insertSample() ) );
    mnu.addSeparator();
    act = mnu.addAction( tr("Information...", "act text") );
      act->setIcon( Application::icon("help_about") );
      connect( act, SIGNAL( triggered() ), this, SLOT( showSampleInfo() ) );
    act = mnu.addAction( tr("Preview", "act text") );
      act->setEnabled(sClient->isAuthorized());
      act->setIcon( Application::icon("pdf") );
      connect( act, SIGNAL( triggered() ), this, SLOT( previewSample() ) );
    mnu.addSeparator();
    act = mnu.addAction( tr("Edit...", "act text") );
    bool ownEditable = sModel->sample(mlastId) && sModel->sample(mlastId)->author().login() == sClient->login()
                         && sModel->sample(mlastId)->type() != TSampleInfo::Approved;
      act->setEnabled(sClient->isAuthorized()
                      && (ownEditable || sClient->accessLevel() >= TAccessLevel::ModeratorLevel));
      act->setIcon( Application::icon("edit") );
      connect( act, SIGNAL( triggered() ), this, SLOT( editSample() ) );
    act = mnu.addAction( tr("Delete...", "act text") );
      act->setEnabled(sClient->isAuthorized() && (ownEditable || sClient->accessLevel() >= TAccessLevel::AdminLevel));
      act->setIcon( Application::icon("editdelete") );
      connect( act, SIGNAL( triggered() ), this, SLOT( deleteSample() ) );
    mnu.exec( mtblvw->mapToGlobal(pos) );
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
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
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
      dlg->setFixedSize(dlg->sizeHint());
    minfoDialogMap.insert(mlastId, dlg);
    minfoDialogIdMap.insert(dlg, mlastId);
    connect(dlg, SIGNAL(destroyed(QObject *)), this, SLOT(infoDialogDestroyed(QObject *)));
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
    }
}

void TexsampleWidget::insertSample()
{
    if (!mlastId)
        return;
    bool ok = false;
    QString subdir = QInputDialog::getText(this, "Subdir", "Subdir:", QLineEdit::Normal,
                                           "texsample-" + QString::number(mlastId), &ok);
    if (!ok)
        return;
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
    if (!sClient->insertSample(mlastId, Window->codeEditor()->currentDocument(), subdir))
    {
        QMessageBox msg( window() );
        msg.setWindowTitle( tr("Failed to insert sample", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to get or insert sample preview", "msgbox text") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
    }
}

void TexsampleWidget::editSample()
{
    if (!mlastId)
        return;
    const TSampleInfo *s = sModel->sample(mlastId);
    if (!s)
        return;
    QDialog dlg(this);
    bool moder = sClient->accessLevel() >= TAccessLevel::ModeratorLevel;
    dlg.setWindowTitle(moder ? tr("Editing sample", "windowTitle") : tr("Updating sample", "windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      SampleWidget *swgt = new SampleWidget(moder ? SampleWidget::EditMode : SampleWidget::UpdateMode);
        swgt->setInfo(*s);
      vlt->addWidget(swgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Ok);
        dlgbbox->button(QDialogButtonBox::Ok)->setEnabled(swgt->isValid());
        connect(swgt, SIGNAL(validityChanged(bool)), dlgbbox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
        connect(dlgbbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dlg, SLOT(accept()));
        dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(dlgbbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dlg, SLOT(reject()));
      vlt->addWidget(dlgbbox);
      dlg.setFixedSize(dlg.sizeHint());
    if (dlg.exec() != QDialog::Accepted)
        return;
    TCompilationResult r = moder ? sClient->editSample(swgt->info(), this) : sClient->updateSample(swgt->info(), this);
    if (!r)
    {
        //TODO: Show message
    }
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
    sClient->deleteSample( mlastId, reason, Window->codeEditor() );
}

void TexsampleWidget::infoDialogDestroyed(QObject *obj)
{
    minfoDialogMap.remove( minfoDialogIdMap.take(obj) );
}
