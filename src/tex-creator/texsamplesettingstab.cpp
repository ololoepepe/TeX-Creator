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

#include "texsamplesettingstab.h"
#include "application.h"
#include "client.h"
#include "cache.h"
#include "global.h"

#include <BAbstractSettingsTab>
#include <BPasswordWidget>
#include <BDirTools>

#include <QObject>
#include <QVariantMap>
#include <QString>
#include <QIcon>
#include <QCheckBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QVariant>
#include <QByteArray>
#include <QSettings>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QToolButton>
#include <QHBoxLayout>

/*============================================================================
================================ TexsampleSettingsTab ========================
============================================================================*/

/*============================== Public constructors =======================*/

TexsampleSettingsTab::TexsampleSettingsTab() :
    BAbstractSettingsTab()
{
    autoSelectText = tr("Auto select");
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Connection", "gbox title"), this);
        QFormLayout *flt = new QFormLayout;
          mhltHost = new QHBoxLayout;
            mcmboxHost = new QComboBox(gbox);
              mcmboxHost->setEditable(true);
              mcmboxHost->setMaxCount(10);
              updateHostHistory(Global::hostHistory());
              int ind = mcmboxHost->findText(Global::host());
              mcmboxHost->setCurrentIndex(ind > 0 ? ind : 0);
              connect(mcmboxHost, SIGNAL(currentIndexChanged(int)), this, SLOT(cmboxHostCurrentIndexChanged(int)));
            mhltHost->addWidget(mcmboxHost);
            mtbtnRemoveFromHistory = new QToolButton(gbox);
              mtbtnRemoveFromHistory->setIcon(Application::icon("editdelete"));
              mtbtnRemoveFromHistory->setToolTip(tr("Remove current host from history", "tbtn toolTip"));
              cmboxHostCurrentIndexChanged(mcmboxHost->currentIndex());
              connect(mtbtnRemoveFromHistory, SIGNAL(clicked()), this, SLOT(removeCurrentHostFromHistory()));
            mhltHost->addWidget(mtbtnRemoveFromHistory);
          flt->addRow(tr("Host:", "lbl text"), mhltHost);
          mledtLogin = new QLineEdit(gbox);
            mledtLogin->setText(Global::login());
          flt->addRow(tr("Login:", "lbl text"), mledtLogin);
          mpwdwgt = new BPasswordWidget(gbox);
            mpwdwgt->restoreWidgetState(Global::passwordWidgetState());
            mpwdwgt->restorePasswordState(Global::passwordState());
          flt->addRow(tr("Password:", "lbl text"), mpwdwgt);
          mcboxAutoconnection = new QCheckBox(gbox);
            mcboxAutoconnection->setChecked(Global::autoconnection());
          flt->addRow(tr("Autoconnection:", "lbl text"), mcboxAutoconnection);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Other", "gbox title"), this);
        flt = new QFormLayout;
          QHBoxLayout *hlt = new QHBoxLayout;
            mcboxCaching = new QCheckBox(gbox);
              mcboxCaching->setChecked(Global::cachingEnabled());
            hlt->addWidget(mcboxCaching);
            QPushButton *btn = new QPushButton(tr("Clear cache", "btn text"), gbox);
              connect( btn, SIGNAL( clicked() ), this, SLOT( clearCache() ) );
            hlt->addWidget(btn);
          flt->addRow(tr("Enable caching:", "lbl text"), hlt);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
}

/*============================== Public methods ============================*/

QString TexsampleSettingsTab::title() const
{
    return tr("TeXSample", "title");
}

QIcon TexsampleSettingsTab::icon() const
{
    return Application::icon("tex");
}

bool TexsampleSettingsTab::restoreDefault()
{
    mcmboxHost->setCurrentIndex(0);
    return true;
}

bool TexsampleSettingsTab::saveSettings()
{
    QString nhost = mcmboxHost->currentText();
    if (autoSelectText == nhost)
        nhost = "auto_select";
    if (Global::hasTexsample() && Global::host() != nhost)
    {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Confirmation", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Question);
        msg.setText(tr("You are going to change server address. All cached files will be removed.", "msgbox text"));
        msg.setInformativeText( tr("Are you absolutely sure?", "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg.setDefaultButton(QMessageBox::Yes);
        if (msg.exec() != QMessageBox::Yes)
            return false;
    }
    Global::setAutoconnection(mcboxAutoconnection->isChecked());
    Global::setHost(nhost);
    Global::setHostHistory(updateHostHistory());
    Global::setLogin(mledtLogin->text());
    Global::setPasswordState(mpwdwgt->savePasswordState(BPassword::AlwaysEncryptedMode));
    Global::setPasswordWidgetSate(mpwdwgt->saveWidgetState());
    Global::setCachingEnabled(mcboxCaching->isChecked());
    sClient->updateSettings();
    return true;
}

/*============================== Private methods ===========================*/

QStringList TexsampleSettingsTab::updateHostHistory(const QStringList &history)
{
    QStringList list = QStringList() << autoSelectText;
    if (history.isEmpty())
    {
        list << mcmboxHost->currentText();
        foreach (int i, bRangeD(1, mcmboxHost->count() - 1))
            list << mcmboxHost->itemText(i);
        list.removeAll("");
        list.removeDuplicates();
    }
    else
    {
        list << history;
    }
    list = list.mid(0, 10);
    mcmboxHost->clear();
    mcmboxHost->addItems(list);
    mcmboxHost->setCurrentIndex(mcmboxHost->count() > 1 ? 1 : 0);
    return list.mid(1);
}

/*============================== Private slots =============================*/

void TexsampleSettingsTab::clearCache()
{
    if (!Cache::cacheExists())
        return;
    QMessageBox msg(this);
    msg.setWindowTitle( tr("Confirmation", "msgbox windowTitle") );
    msg.setIcon(QMessageBox::Question);
    msg.setText( tr("You are going to delete all cached files. This action is irreversible", "msgbox text") );
    msg.setInformativeText( tr("Are you absolutely sure?", "msgbox informativeText") );
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setDefaultButton(QMessageBox::Yes);
    if (msg.exec() != QMessageBox::Yes)
        return;
    sCache->clear();
}

void TexsampleSettingsTab::removeCurrentHostFromHistory()
{
    QString text = mcmboxHost->currentText();
    QStringList list = updateHostHistory();
    list.removeAll(text);
    mcmboxHost->clear();
    Global::setHostHistory(updateHostHistory(list));
}

void TexsampleSettingsTab::cmboxHostCurrentIndexChanged(int index)
{
    mtbtnRemoveFromHistory->setEnabled(index > 0);
}
