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

#include "texsamplewidget.h"

#include "application.h"
#include "connectionaction.h"
#include "mainwindow.h"
#include "samplemodel.h"
#include "sampleproxymodel.h"
#include "settings.h"
#include "texsamplecore.h"

#include <TAccessLevel>
#include <TNetworkClient>
#include <TSampleInfo>
#include <TSampleType>
#include <TUserInfo>

#include <BCodeEditor>
#include <BSignalDelayProxy>

#include <QAction>
#include <QByteArray>
#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QModelIndex>
#include <QString>
#include <QTableView>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

/*============================================================================
================================ TexsampleWidget =============================
============================================================================*/

/*============================== Public constructors =======================*/

TexsampleWidget::TexsampleWidget(MainWindow *window, QWidget *parent) :
    QWidget(parent), Window(window)
{
    mlastId = 0;
    mproxyModel = new SampleProxyModel(tSmp->sampleModel(), this);
    TNetworkClient *client = tSmp->client();
    connect(client, SIGNAL(stateChanged(TNetworkClient::State)),
            this, SLOT(clientStateChanged(TNetworkClient::State)));
    //
    mtbarIndicator = new QToolBar;
    //
    QVBoxLayout *vlt = new QVBoxLayout(this);
      mtbar = new QToolBar(this);
        mactConnection = new ConnectionAction(this);
          QMenu *mnu = new QMenu;
            mactConnect = new QAction(this);
              mactConnect->setIcon(Application::icon("connect_established"));
              connect(mactConnect, SIGNAL(triggered()), tSmp, SLOT(connectToServer()));
            mnu->addAction(mactConnect);
            mactDisconnect = new QAction(this);
              mactDisconnect->setIcon(Application::icon("connect_no"));
              connect(mactDisconnect, SIGNAL(triggered()), tSmp, SLOT(disconnectFromServer()));
            mnu->addAction(mactDisconnect);
          mactConnection->setMenu(mnu);
        mtbar->addAction(mactConnection);
        mtbarIndicator->addAction(mactConnection);
        mactUpdate = new QAction(this);
          mactUpdate->setIcon(Application::icon("reload"));
          connect(mactUpdate, SIGNAL(triggered()), tSmp, SLOT(updateSampleList()));
        mtbar->addAction(mactUpdate);
        mactSend = new QAction(this);
          mactSend->setIcon(Application::icon("mail_send"));
          connect(mactSend, SIGNAL(triggered()), this, SLOT(sendSample()));
        mtbar->addAction(mactSend);
        mactTools = new QAction(this);
          mactTools->setIcon(Application::icon("configure"));
          mnu = new QMenu;
            mactRegister = new QAction(this);
              mactRegister->setIcon(Application::icon("add_user"));
              connect(mactRegister, SIGNAL(triggered()), tSmp, SLOT(showRegisterDialog()));
            mnu->addAction(mactRegister);
            mnu->addSeparator();
            mactConfirm = new QAction(this);
              mactConfirm->setIcon(Application::icon("")); //TODO
              connect(mactConfirm, SIGNAL(triggered()), tSmp, SLOT(showConfirmRegistrationDialog()));
            mnu->addAction(mactConfirm);
            mnu->addSeparator();
            mactRecover = new QAction(this);
              mactRecover->setIcon(Application::icon("account_recover"));
              connect(mactRecover, SIGNAL(triggered()), tSmp, SLOT(showRecoverDialog()));
            mnu->addAction(mactRecover);
            mnu->addSeparator();
            mactAccountSettings = new QAction(this);
              mactAccountSettings->setIcon(Application::icon("user"));
              connect( mactAccountSettings, SIGNAL(triggered()), tSmp, SLOT(showAccountSettings()));
            mnu->addAction(mactAccountSettings);
            mnu->addSeparator();
            mactSettings = new QAction(this);
              mactSettings->setIcon(Application::icon("configure"));
              connect(mactSettings, SIGNAL(triggered()), tSmp, SLOT(showTexsampleSettings()));
            mnu->addAction(mactSettings);
            mactAdministration = new QAction(this);
              mactAdministration->setIcon(Application::icon("gear"));
              QMenu *submnu = new QMenu;
                mactUserManagement = submnu->addAction(Application::icon("users"), "", tSmp,
                                                       SLOT(showUserManagementWidget()));
                mactGroupManagement = submnu->addAction(Application::icon("group"), "", tSmp,
                                                        SLOT(showGroupManagementWidget()));
                mactInviteManagement = submnu->addAction(Application::icon(""), "mail_send", tSmp,
                                                         SLOT(showInviteManagementWidget()));
            mactAdministration->setMenu(submnu);
            mnu->addAction(mactAdministration);
          mactTools->setMenu(mnu);
        mtbar->addAction(mactTools);
        static_cast<QToolButton *>(mtbar->widgetForAction(mactTools))->setPopupMode(QToolButton::InstantPopup);
      vlt->addWidget(mtbar);
      mgboxSelect = new QGroupBox(this);
        QFormLayout *flt = new QFormLayout;
          mlblType = new QLabel;
          mcmboxType = new QComboBox;
            connect(mcmboxType, SIGNAL(currentIndexChanged(int)), this, SLOT(cmboxTypeCurrentIndexChanged(int)));
          flt->addRow(mlblType, mcmboxType);
          mlblSearch = new QLabel;
          QHBoxLayout *hlt = new QHBoxLayout;
            mledtSearch = new QLineEdit;
              BSignalDelayProxy *sdp = new BSignalDelayProxy(this);
              sdp->setStringConnection(mledtSearch, SIGNAL(textChanged(QString)),
                                       mproxyModel, SLOT(setSearchKeywordsString(QString)));
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
        connect(mtblvw, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(tblvwDoubleClicked(QModelIndex)));
        connect(mtblvw, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(tblvwCustomContextMenuRequested(QPoint)));
      vlt->addWidget(mtblvw);
    //
    retranslateUi();
    connect(bApp, SIGNAL(languageChanged()), this, SLOT(retranslateUi()));
    mcmboxType->setCurrentIndex(mcmboxType->findData(Settings::TexsampleWidget::selectedSampleType()));
    mtblvw->horizontalHeader()->restoreState(Settings::TexsampleWidget::sampleTableHeaderState());
    clientStateChanged(client->state());
}

TexsampleWidget::~TexsampleWidget()
{
    Settings::TexsampleWidget::setSampleTableHeaderState(mtblvw->horizontalHeader()->saveState());
    Settings::TexsampleWidget::setSelectedSampleType(mcmboxType->itemData(mcmboxType->currentIndex()).toInt());
}

/*============================== Public methods ============================*/

QWidget *TexsampleWidget::indicator() const
{
    return mtbarIndicator;
}

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
    mcmboxType->addItem(TSampleType::sampleTypeToString(TSampleType::Approved, false), int(TSampleType::Approved));
    mcmboxType->addItem(TSampleType::sampleTypeToString(TSampleType::Rejected, false), int(TSampleType::Rejected));
    mcmboxType->addItem(TSampleType::sampleTypeToString(TSampleType::Unverified, false), int(TSampleType::Unverified));
    mcmboxType->addItem(tr("My", "cmbox item text"), int(SampleProxyModel::CurrentUserSample));
    mcmboxType->setCurrentIndex(ind);
    mcmboxType->blockSignals(false);
}

/*============================== Private slots =============================*/

void TexsampleWidget::clientStateChanged(TNetworkClient::State state)
{
    QString s = tr("TeXSample state:", "act toolTip") + " ";
    switch (state)
    {
    case TNetworkClient::DisconnectedState:
        mactConnection->resetIcon(s + tr("disconnected", "act toolTip"), "connect_no");
        break;
    case TNetworkClient::ConnectingState:
        mactConnection->resetIcon(s + tr("connecting", "act toolTip"), "process", true);
        break;
    case TNetworkClient::ConnectedState:
        mactConnection->resetIcon(s + tr("connected", "act toolTip"), "process", true);
        break;
    case TNetworkClient::AuthorizedState:
        mactConnection->resetIcon(s + tr("authorized", "act toolTip"), "connect_established");
        break;
    case TNetworkClient::DisconnectingState:
        mactConnection->resetIcon(s + tr("disconnecting", "act toolTip"), "process", true);
        break;
    default:
        break;
    }
    int lvl = tSmp->client()->userInfo().accessLevel();
    bool authorized = tSmp->client()->isAuthorized();
    mactConnect->setEnabled(TNetworkClient::DisconnectedState == state);
    mactDisconnect->setEnabled(TNetworkClient::DisconnectedState != state
            && TNetworkClient::DisconnectingState != state);
    mactUpdate->setEnabled(authorized);
    mactSend->setEnabled(authorized);
    mactAccountSettings->setEnabled(authorized);
    mactAdministration->setEnabled(lvl >= TAccessLevel::ModeratorLevel);
    mactUserManagement->setEnabled(lvl >= TAccessLevel::AdminLevel);
    mactGroupManagement->setEnabled(lvl >= TAccessLevel::ModeratorLevel);
    mactInviteManagement->setEnabled(lvl >= TAccessLevel::AdminLevel);
}

void TexsampleWidget::cmboxTypeCurrentIndexChanged(int index)
{
    if (index < 0)
        return;
    mproxyModel->setSampleType(mcmboxType->itemData(index).toInt());
}

void TexsampleWidget::deleteSample()
{
    if (!mlastId)
        return;
    tSmp->deleteSample(mlastId);
}

void TexsampleWidget::editSample()
{
    if (!mlastId)
        return;
    tSmp->editSample(mlastId, Window->codeEditor());
}

void TexsampleWidget::insertSample()
{
    if (!mlastId)
        return;
    tSmp->insertSample(mlastId, Window->codeEditor());
}

void TexsampleWidget::retranslateUi()
{
    mactConnection->setText(tr("Connection", "act text"));
    mactConnection->setWhatsThis(tr("This action shows current connection state. "
                                    "Use it to connect or disconnect from the server", "act whatsThis"));
    clientStateChanged(tSmp->client()->state());
    mactConnect->setText(tr("Connect", "act text"));
    mactDisconnect->setText(tr("Disconnect", "act text"));
    mactUpdate->setText(tr("Update", "act text"));
    mactUpdate->setToolTip(tr("Update samples list", "act toolTip"));
    mactSend->setText(tr("Send sample...", "act text"));
    mactSend->setToolTip(tr("Send sample...", "act toolTip"));
    mactTools->setText(tr("Tools", "act text"));
    mactTools->setToolTip(tr("Tools", "act toolTip"));
    mactRegister->setText(tr("Register...", "act tooTip"));
    mactConfirm->setText(tr("Confirm registration...", "act text"));
    mactRecover->setText(tr("Recover account...", "act text"));
    mactSettings->setText(tr("TeXSample settings...", "act text"));
    mactAccountSettings->setText(tr("Account management...", "act text"));
    mactAdministration->setText(tr("Administration", "act text"));
    mactUserManagement->setText(tr("User management...", "act text"));
    mactGroupManagement->setText(tr("Group management...", "act text"));
    mactInviteManagement->setText(tr("Invite management...", "act text"));
    //
    mgboxSelect->setTitle( tr("Selection", "gbox title") );
    //
    mlblType->setText(tr("Type:", "lbl text"));
    mlblSearch->setText(tr("Search:", "lbl text"));
    //
    retranslateCmboxType();
}

void TexsampleWidget::saveSample()
{
    if (!mlastId)
        return;
    tSmp->saveSample(mlastId, Window);
}

void TexsampleWidget::sendSample()
{
    tSmp->sendSample(Window->codeEditor());
}

void TexsampleWidget::showSampleInfo()
{
    if (!mlastId)
        return;
    tSmp->showSampleInfo(mlastId);
}

void TexsampleWidget::showSamplePreview()
{
    if (!mlastId)
        return;
    tSmp->showSamplePreview(mlastId);
}

void TexsampleWidget::tblvwCustomContextMenuRequested(const QPoint &pos)
{
    mlastId = tSmp->sampleModel()->sampleIdAt(mproxyModel->mapToSource(mtblvw->indexAt(pos)).row());
    if (!mlastId)
        return;
    TNetworkClient *client = tSmp->client();
    QMenu mnu;
    QAction *act = mnu.addAction(tr("Insert...", "act text"), this, SLOT(insertSample()));
      act->setEnabled(client->isAuthorized() && Window->codeEditor()->documentAvailable());
      act->setIcon(Application::icon("editpaste"));
    act = mnu.addAction(tr("Save...", "act text"), this, SLOT(saveSample()));
      act->setEnabled(client->isAuthorized());
      act->setIcon(Application::icon("filesave"));
    mnu.addSeparator();
    act = mnu.addAction(tr("Information...", "act text"), this, SLOT(showSampleInfo()));
      act->setIcon(Application::icon("help_about"));
      act = mnu.addAction(tr("Preview", "act text"), this, SLOT(showSampleInfo()));
      act->setEnabled(client->isAuthorized());
      act->setIcon(Application::icon("pdf"));
    mnu.addSeparator();
    act = mnu.addAction(tr("Edit...", "act text"), this, SLOT(editSample()));
      act->setIcon(Application::icon("edit"));
      TSampleInfo sampleInfo = tSmp->sampleModel()->sampleInfo(mlastId);
      TUserInfo userInfo = client->userInfo();
      bool own = (sampleInfo.senderId() == userInfo.id() && int(sampleInfo.type()) != TSampleType::Approved);
      act->setEnabled(client->isAuthorized() && (own || int(userInfo.accessLevel()) >= TAccessLevel::ModeratorLevel));
    act = mnu.addAction(tr("Delete...", "act text"), this, SLOT(deleteSample()));
    act->setEnabled(client->isAuthorized() && (own || int(userInfo.accessLevel()) >= TAccessLevel::AdminLevel));
      act->setIcon(Application::icon("editdelete"));
    mnu.exec(mtblvw->mapToGlobal(pos));
}

void TexsampleWidget::tblvwDoubleClicked(const QModelIndex &index)
{
    if (!Window->codeEditor()->documentAvailable())
        return;
    QModelIndex ind = mproxyModel->mapToSource(index);
    if (!ind.isValid())
        return;
    mlastId = tSmp->sampleModel()->sampleIdAt(ind.row());
    if (!mlastId)
        return;
    insertSample();
}
