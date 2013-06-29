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
            mpwdwgt->restoreState(Global::passwordState());
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
    //
    setRowVisible(mhltHost, false);
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

bool TexsampleSettingsTab::hasAdvancedMode() const
{
    return true;
}

void TexsampleSettingsTab::setAdvancedMode(bool enabled)
{
    setRowVisible(mhltHost, enabled);
}

bool TexsampleSettingsTab::restoreDefault()
{
    mcmboxHost->setCurrentIndex(0);
    return true;
}

bool TexsampleSettingsTab::saveSettings()
{
    if (mcmboxHost->currentText() != Global::host())
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
    Global::setHost(mcmboxHost->currentIndex() > 0 ? mcmboxHost->currentText() : QString("auto_select"));
    Global::setHostHistory(updateHostHistory());
    Global::setLogin(mledtLogin->text());
    Global::setPasswordSate(mpwdwgt->saveStateEncrypted());
    Global::setCachingEnabled(mcboxCaching->isChecked());
    sClient->updateSettings();
    return true;
}

/*============================== Private methods ===========================*/

QStringList TexsampleSettingsTab::updateHostHistory(const QStringList &history)
{
    QStringList list = QStringList() << tr("Auto select", "cmbox item text");
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
