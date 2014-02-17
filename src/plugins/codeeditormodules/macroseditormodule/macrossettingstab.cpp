#include "macrossettingstab.h"
#include "macroseditormodule.h"
#include "macroseditormoduleplugin.h"

#include <BAbstractSettingsTab>
#include <BeQtGlobal>
#include <BApplication>

#include <QString>
#include <QIcon>
#include <QObject>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QToolButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QMap>
#include <QFileDialog>
#include <QDir>
#include <QHBoxLayout>
#include <QCheckBox>

#include <QDebug>

/*============================================================================
================================ MacrosSettingsTab ===========================
============================================================================*/

/*============================== Public constructors =======================*/

MacrosSettingsTab::MacrosSettingsTab() :
    BAbstractSettingsTab()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("General", "gbox title"));
        QFormLayout *flts = new QFormLayout(gbox);
          QHBoxLayout *hlt = new QHBoxLayout;
            cboxSaveStack = new QCheckBox;
              cboxSaveStack->setChecked(MacrosEditorModulePlugin::saveMacroStack());
            hlt->addWidget(cboxSaveStack);
            QPushButton *btn = new QPushButton(tr("Clear stack", "btn text"));
              connect(btn, SIGNAL(clicked()), this, SLOT(clearStack()));
            hlt->addWidget(btn);
          flts->addRow(tr("Save stack:", "lbl text"), hlt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("External tools", "gbox title"));
        flt = new QFormLayout(gbox);
      vlt->addWidget(gbox);
      vlt->addStretch();
      hlt = new QHBoxLayout;
        hlt->addStretch();
        btn = new QPushButton(tr("Add line", "btn text"));
          connect(btn, SIGNAL(clicked()), this, SLOT(addRow()));
        hlt->addWidget(btn);
      vlt->addLayout(hlt);
    //
    QMap<QString, QString> map = MacrosEditorModulePlugin::externalTools();
    foreach (const QString &k, map.keys())
        addRow(k, map.value(k));
    addRow();
}

/*============================== Public methods ============================*/

QString MacrosSettingsTab::title() const
{
    return tr("Macros", "title");
}

QIcon MacrosSettingsTab::icon() const
{
    return BApplication::icon("player_record");
}

bool MacrosSettingsTab::saveSettings()
{
    MacrosEditorModulePlugin::setSaveMacroStack(cboxSaveStack->isChecked());
    QMap<QString, QString> map;
    foreach (QHBoxLayout *hlt, layoutMap)
    {
        QLineEdit *ledtName = BApplication::labelForField<QLineEdit>(hlt);
        QLineEdit *ledtPath = qobject_cast<QLineEdit *>(hlt->itemAt(0)->widget());
        if (!ledtName || !ledtPath || ledtName->text().isEmpty())
            continue;
        map.insert(ledtName->text(), ledtPath->text());
    }
    MacrosEditorModulePlugin::setExternalTools(map);
    return true;
}

/*============================== Private mslots ============================*/

void MacrosSettingsTab::addRow(const QString &name, const QString &path)
{
    QLineEdit *ledtName = new QLineEdit(name);
    QHBoxLayout *hlt = new QHBoxLayout;
      QLineEdit *ledtPath = new QLineEdit(path);
      hlt->addWidget(ledtPath);
      QPushButton *btn = new QPushButton(tr("Search...", "btn text"));
        connect(btn, SIGNAL(clicked()), this, SLOT(search()));
      hlt->addWidget(btn);
      QToolButton *tbtn = new QToolButton;
        tbtn->setIcon(BApplication::icon("editdelete"));
        tbtn->setToolTip(tr("Remove", "tbtn toolTip"));
        connect(tbtn, SIGNAL(clicked()), this, SLOT(removeRow()));
      hlt->addWidget(tbtn);
      pathMap.insert(btn, ledtPath);
    flt->addRow(ledtName, hlt);
    layoutMap.insert(tbtn, hlt);
}

void MacrosSettingsTab::removeRow()
{
    QHBoxLayout *hlt = layoutMap.value(sender());
    if (!hlt)
        return;
    layoutMap.remove(sender());
    pathMap.remove(hlt->itemAt(hlt->count() - 2)->widget());
    for (int i = hlt->count() - 1; i >= 0; --i)
        hlt->takeAt(i)->widget()->deleteLater();
    flt->labelForField(hlt)->deleteLater();
    hlt->deleteLater();
}

void MacrosSettingsTab::search()
{
    QLineEdit *ledt = pathMap.value(sender());
    if (!ledt)
        return;
    QString dir = ledt->text();
    if (dir.isEmpty())
        dir = QDir::homePath();
    QString fn = QFileDialog::getOpenFileName(this, tr("Select file", "fdlg caption"), dir, "", &dir);
    if (fn.isEmpty())
        return;
    ledt->setText(fn);
}

void MacrosSettingsTab::clearStack()
{
    MacrosEditorModule::clearMacroStack();
}
