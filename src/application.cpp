﻿#include "application.h"
#include "client.h"
#include "texsamplesettingstab.h"
#include "mainwindow.h"
#include "global.h"

#include <TUserInfo>
#include <TOperationResult>
#include <TCompilerParameters>
#include <TUserWidget>

#include <BApplication>
#include <BSettingsDialog>
#include <BCodeEditor>
#include <BPasswordWidget>
#include <BAbstractSettingsTab>
#include <BLocaleComboBox>
#include <BSpellChecker>
#include <BDirTools>
#include <BTextCodecComboBox>
#include <BDialog>

#include <QObject>
#include <QVariantMap>
#include <QByteArray>
#include <QList>
#include <QMessageBox>
#include <QApplication>
#include <QPushButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QSize>
#include <QPushButton>
#include <QFontComboBox>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QRegExp>
#include <QSettings>

#include <QDebug>

/*============================================================================
================================ CodeEditorSettingsTab =======================
============================================================================*/

class CodeEditorSettingsTab : public BAbstractSettingsTab
{
    Q_DECLARE_TR_FUNCTIONS(CodeEditorSettingsTab)
public:
    explicit CodeEditorSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool hasDefault() const;
    bool restoreDefault();
    bool saveSettings();
private:
    QCheckBox *mcboxSimple;
    QFontComboBox *mfntcmbox;
    QSpinBox *msboxFontPointSize;
    QSpinBox *msboxLineLength;
    QComboBox *mcmboxTabWidth;
    BTextCodecComboBox *mcmboxEncoding;
private:
    Q_DISABLE_COPY(CodeEditorSettingsTab)
};

/*============================================================================
================================ ConsoleSettingsTab ==========================
============================================================================*/

class ConsoleSettingsTab : public BAbstractSettingsTab
{
    Q_DECLARE_TR_FUNCTIONS(ConsoleSettingsTab)
public:
    explicit ConsoleSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool hasDefault() const;
    bool restoreDefault();
    bool saveSettings();
private:
    QComboBox *mcmboxCompiler;
    QLineEdit *mledtOptions;
    QLineEdit *mledtCommands;
    QCheckBox *mcboxMakeindex;
    QCheckBox *mcboxDvips;
    QCheckBox *mcboxRemoteCompiler;
    QCheckBox *mcboxFallbackToLocalCompiler;
    QCheckBox *mcboxAlwaysLatin;
private:
    Q_DISABLE_COPY(ConsoleSettingsTab)
};

/*============================================================================
================================ GeneralSettingsTab ==========================
============================================================================*/

class GeneralSettingsTab : public BAbstractSettingsTab
{
    Q_DECLARE_TR_FUNCTIONS(GeneralSettingsTab)
public:
    explicit GeneralSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool hasDefault() const;
    bool restoreDefault();
    bool saveSettings();
private:
    BLocaleComboBox *mlcmbox;
    QCheckBox *mcboxMultipleWindows;
private:
    Q_DISABLE_COPY(GeneralSettingsTab)
};

/*============================================================================
================================ PasswordWidget ==============================
============================================================================*/

class PasswordDialog : public QDialog
{
    Q_DECLARE_TR_FUNCTIONS(PasswordDialog)
public:
    explicit PasswordDialog(QWidget *parent = 0);
public:
    void setPasswordState(const QByteArray &state);
    QByteArray passwordState() const;
private:
    BPasswordWidget *mpwdwgt;
};

/*============================================================================
================================ CodeEditorSettingsTab =======================
============================================================================*/

/*============================== Public constructors =======================*/

CodeEditorSettingsTab::CodeEditorSettingsTab()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Document type", "gbox title"), this);
        QFormLayout *flt = new QFormLayout;
          mcboxSimple = new QCheckBox;
            mcboxSimple->setChecked(Global::editorDocumentType() == BCodeEditor::SimpleDocument);
          flt->addRow(tr("Classic documents:", "lbl text"), mcboxSimple);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Font", "gbox title"), this);
        flt = new QFormLayout;
          mfntcmbox = new QFontComboBox(gbox);
            mfntcmbox->setFontFilters(QFontComboBox::MonospacedFonts);
            mfntcmbox->setCurrentFont(Global::editFont());
          flt->addRow(tr("Font:", "lbl text"), mfntcmbox);
          msboxFontPointSize = new QSpinBox(gbox);
            msboxFontPointSize->setMinimum(1);
            msboxFontPointSize->setMaximum(100);
            msboxFontPointSize->setValue(Global::editFontPointSize());
          flt->addRow(tr("Font size:", "lbl text"), msboxFontPointSize);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Tabulation and lines", "gbox title"), this);
        flt = new QFormLayout;
          msboxLineLength = new QSpinBox(gbox);
            msboxLineLength->setMinimum(10);
            msboxLineLength->setMaximum(1000);
            msboxLineLength->setSingleStep(10);
            msboxLineLength->setValue(Global::editLineLength());
            msboxLineLength->setEnabled(!mcboxSimple->isChecked());
            connect(mcboxSimple, SIGNAL(toggled(bool)), msboxLineLength, SLOT(setDisabled(bool)));
          flt->addRow(tr("Line length:", "lbl text"), msboxLineLength);
          mcmboxTabWidth = new QComboBox(gbox);
            mcmboxTabWidth->addItem(QString::number(BeQt::TabWidth2), BeQt::TabWidth2);
            mcmboxTabWidth->addItem(QString::number(BeQt::TabWidth4), BeQt::TabWidth4);
            mcmboxTabWidth->addItem(QString::number(BeQt::TabWidth8), BeQt::TabWidth8);
            mcmboxTabWidth->setCurrentIndex(mcmboxTabWidth->findData(Global::editTabWidth()));
          flt->addRow(tr("Tab width:", "lbl text"), mcmboxTabWidth);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Files", "gbox title"), this);
        flt = new QFormLayout;
          mcmboxEncoding = new BTextCodecComboBox;
            mcmboxEncoding->selectCodec(Global::defaultCodec());
          flt->addRow(tr("Default encoding:", "lbl text"), mcmboxEncoding);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
}

/*============================== Public methods ============================*/

QString CodeEditorSettingsTab::title() const
{
    return tr("Text editor", "title");
}

QIcon CodeEditorSettingsTab::icon() const
{
    return Application::icon("edit");
}

bool CodeEditorSettingsTab::hasDefault() const
{
    return true;
}

bool CodeEditorSettingsTab::restoreDefault()
{
    QFont fnt = Application::createMonospaceFont();
    mcboxSimple->setChecked(false);
    mfntcmbox->setCurrentFont(fnt);
    msboxFontPointSize->setValue( fnt.pointSize() );
    mcmboxEncoding->selectCodec(QTextCodec::codecForLocale());
    msboxLineLength->setValue(120);
    mcmboxTabWidth->setCurrentIndex(mcmboxTabWidth->findData(BeQt::TabWidth4));
    return true;
}

bool CodeEditorSettingsTab::saveSettings()
{
    Global::setEditorDocumentType(mcboxSimple->isChecked() ? BCodeEditor::SimpleDocument :
                                                             BCodeEditor::StandardDocument);
    Global::setEditFontFamily(mfntcmbox->currentFont().family());
    Global::setEditFontPointSize(msboxFontPointSize->value());
    Global::setDefaultCodec(mcmboxEncoding->selectedCodec());
    Global::setEditLineLength(msboxLineLength->value());
    Global::setEditTabWidth(mcmboxTabWidth->itemData(mcmboxTabWidth->currentIndex()).toInt());
    return true;
}

/*============================================================================
================================ ConsoleSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

ConsoleSettingsTab::ConsoleSettingsTab()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Compiler", "gbox title"), this);
        QFormLayout *flt = new QFormLayout;
          mcmboxCompiler = new QComboBox(gbox);
            foreach (TCompilerParameters::Compiler c, TCompilerParameters::allCompilers())
                mcmboxCompiler->addItem(TCompilerParameters::compilerToString(c), c);
            mcmboxCompiler->setCurrentIndex(mcmboxCompiler->findData(Global::compiler()));
          flt->addRow(tr("Compiler:", "label text"), mcmboxCompiler);
          mledtOptions = new QLineEdit(gbox);
            mledtOptions->setText(Global::compilerOptionsString());
            mledtOptions->setToolTip(tr("Separate options with spaces", "ledt toolTip"));
          flt->addRow(tr("Compiler options:", "lbl text"), mledtOptions);
          mledtCommands = new QLineEdit(this);
            mledtCommands->setText(Global::compilerCommandsString());
            mledtCommands->setToolTip(tr("Use quotes (\") to wrap commands that contain spaces", "ledt toolTip"));
          flt->addRow(tr("Compiler commands:", "lbl text"), mledtCommands);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Remote compiler", "gbox title"), this);
        flt = new QFormLayout;
          mcboxRemoteCompiler = new QCheckBox(gbox);
            mcboxRemoteCompiler->setChecked(Global::useRemoteCompiler());
            mcboxRemoteCompiler->setToolTip(tr("If checked and if you are connected to the TeXSample service, "
                                               "remote compilation system will be used", "cbox toolTip"));
          flt->addRow(tr("Remote compilation:", "lbl text"), mcboxRemoteCompiler);
          mcboxFallbackToLocalCompiler = new QCheckBox(gbox);
            mcboxFallbackToLocalCompiler->setEnabled(mcboxRemoteCompiler->isChecked());
            mcboxFallbackToLocalCompiler->setToolTip(tr("If checked and if the remote compiler is not available, "
                                                        "the local one will be used", "cbox toolTip"));
            mcboxFallbackToLocalCompiler->setChecked(Global::hasFallbackToLocalCompiler()
                                                     && Global::fallbackToLocalCompiler());
            connect(mcboxRemoteCompiler, SIGNAL(clicked(bool)), mcboxFallbackToLocalCompiler, SLOT(setEnabled(bool)));
          flt->addRow(tr("Fallback to remote compiler:", "lbl text"), mcboxFallbackToLocalCompiler);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Tools", "gbox title"), this);
        flt = new QFormLayout;
          mcboxMakeindex = new QCheckBox(gbox);
            mcboxMakeindex->setToolTip(tr("Run makeindex after compilation", "cbox toolTip"));
            mcboxMakeindex->setWhatsThis(tr("Check this option to run the makeindex utility after compliation",
                                            "cbox whatsThis"));
            mcboxMakeindex->setChecked(Global::makeindexEnabled());
          flt->addRow(tr("Makeindex:", "lbl text"), mcboxMakeindex);
          mcboxDvips = new QCheckBox(gbox);
            mcboxDvips->setToolTip(tr("Run dvips after compilation", "cbox toolTip"));
            mcboxDvips->setWhatsThis(tr("Check this option to run the dvips utility after compilation",
                                        "cbox whatsThis"));
            mcboxDvips->setChecked(Global::dvipsEnabled());
          flt->addRow(tr("Dvips:", "lbl text"), mcboxDvips);
          mcboxAlwaysLatin = new QCheckBox(gbox);
            mcboxAlwaysLatin->setToolTip(tr("If checked, Latin letters will always be entered, "
                                            "ignoring keyboard layout", "cbox toolTip"));
            mcboxAlwaysLatin->setWhatsThis(tr("Check this option if you always enter latin only characters "
                                              "into console, so you will not have to switch keyboard layout",
                                              "cbox whatsThis"));
            mcboxAlwaysLatin->setChecked(Global::alwaysLatinEnabled());
          flt->addRow(tr("Always Latin:", "lbl text"), mcboxAlwaysLatin);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
}

/*============================== Public methods ============================*/

QString ConsoleSettingsTab::title() const
{
    return tr("Console", "title");
}

QIcon ConsoleSettingsTab::icon() const
{
    return Application::icon("utilities_terminal");
}

bool ConsoleSettingsTab::hasDefault() const
{
    return true;
}

bool ConsoleSettingsTab::restoreDefault()
{
    mcmboxCompiler->setCurrentIndex( mcmboxCompiler->findText("pdflatex") );
    return true;
}

bool ConsoleSettingsTab::saveSettings()
{
    TCompilerParameters param;
    param.setCompiler(mcmboxCompiler->itemData(mcmboxCompiler->currentIndex()).toInt());
    param.setOptions(mledtOptions->text());
    param.setCommands(mledtCommands->text());
    param.setMakeindexEnabled(mcboxMakeindex->isChecked());
    param.setDvipsEnabled(mcboxDvips->isChecked());
    Global::setCompilerParameters(param);
    Global::setUseRemoteCompiler(mcboxRemoteCompiler->isChecked());
    Global::setAlwaysLatinEnabled(mcboxAlwaysLatin->isChecked());
    if (Global::hasFallbackToLocalCompiler() || mcboxFallbackToLocalCompiler->isChecked())
        Global::setFallbackToLocalCompiler(mcboxFallbackToLocalCompiler->isChecked());
    return true;
}

/*============================================================================
================================ GeneralSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

GeneralSettingsTab::GeneralSettingsTab() :
    BAbstractSettingsTab()
{
    QFormLayout *flt = new QFormLayout(this);
    mlcmbox = new BLocaleComboBox(true, this);
      mlcmbox->setCurrentLocale( Application::locale() );
    flt->addRow("Language (" + tr("language", "lbl text") + "):", mlcmbox);
    mcboxMultipleWindows = new QCheckBox(this);
      mcboxMultipleWindows->setChecked(Global::multipleWindowsEnabled());
    flt->addRow(tr("Enable multiple windows:", "lbl text"), mcboxMultipleWindows);
}

/*============================== Public methods ============================*/

QString GeneralSettingsTab::title() const
{
    return tr("General", "title");
}

QIcon GeneralSettingsTab::icon() const
{
    return Application::icon("configure");
}

bool GeneralSettingsTab::hasDefault() const
{
    return true;
}

bool GeneralSettingsTab::restoreDefault()
{
    mcboxMultipleWindows->setChecked(false);
    return true;
}

bool GeneralSettingsTab::saveSettings()
{
    if (Global::multipleWindowsEnabled() && !mcboxMultipleWindows->isChecked() && !Application::mergeWindows())
    {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Failed to change settings", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Information);
        msg.setText( tr("Can't disable multiple windows: documents merging failed", "msgbox text") );
        msg.setInformativeText( tr("Please, close all duplicate documents and try again", "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return false;
    }
    Application::setLocale(mlcmbox->currentLocale());
    Global::setMultipleWindowsEnabled(mcboxMultipleWindows->isChecked());
    return true;
}

/*============================================================================
================================ PasswordWidget ==============================
============================================================================*/

/*============================== Public constructors =======================*/

PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle( tr("TeXSample password", "windowTitle") );
    QVBoxLayout *vlt = new QVBoxLayout(this);
      mpwdwgt = new BPasswordWidget(this);
        mpwdwgt->restoreWidgetState(Global::passwordWidgetState());
      Application::addRow(vlt, tr("Password:", "lbl text"), mpwdwgt);
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        QPushButton *btnOk = dlgbbox->addButton(QDialogButtonBox::Ok);
        btnOk->setDefault(true);
        connect( btnOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
        connect( dlgbbox->addButton(QDialogButtonBox::Cancel), SIGNAL( clicked() ), this, SLOT( reject() ) );
      vlt->addWidget(dlgbbox);
    //
    setFixedSize( sizeHint() );
}

/*============================== Public methods ============================*/

void PasswordDialog::setPasswordState(const QByteArray &state)
{
    mpwdwgt->restorePasswordState(state);
}

QByteArray PasswordDialog::passwordState() const
{
    return mpwdwgt->savePasswordState(BPassword::AlwaysEncryptedMode);
}

/*============================================================================
================================ Application =================================
============================================================================*/

/*============================== Public constructors =======================*/

Application::Application() :
    BApplication()
{
    minitialWindowCreated = false;
    QStringList paths;
    foreach (const QString &path, BCoreApplication::locations("dictionaries"))
        paths << BDirTools::entryList(path, QStringList() << "??_??", QDir::Dirs);
    msc = new BSpellChecker(paths, location(DataPath, UserResources) + "/dictionaries/ignored.txt");
    msc->ignoreImplicitlyRegExp(QRegExp("\\\\|\\\\\\w+"));
    msc->considerLeftSurrounding(1);
    msc->considerRightSurrounding(0);
}

Application::~Application()
{
    delete msc;
}

/*============================== Static public methods =====================*/

void Application::createInitialWindow(const QStringList &args)
{
    if ( !testAppInit() )
        return;
    if ( !bTest(!bApp->minitialWindowCreated, "Application", "Initial window may be created only once") )
        return;
    bApp->addMainWindow(args);
    bApp->minitialWindowCreated = true;
    if (!Global::hasTexsample())
    {
        QMessageBox msg(mostSuitableWindow());
        msg.setWindowTitle( tr("TeXSample configuration", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Question);
        msg.setText( tr("It seems that you have not configured TeXSample service yet.\n"
                        "Would you like to do it now?", "msgbox text") );
        msg.setInformativeText( tr("To remove this notification, you have to configure or disable the service",
                                   "msgbox informativeText") );
        QPushButton *btn1 = msg.addButton(tr("Register", "btn text"), QMessageBox::AcceptRole);
        QPushButton *btn2 = msg.addButton(tr("I have an account", "btn text"), QMessageBox::AcceptRole);
        QPushButton *btn3 = msg.addButton(tr("Disable TeXSample", "btn text"), QMessageBox::RejectRole);
        msg.addButton(tr("Not right now", "btn text"), QMessageBox::RejectRole);
        msg.setDefaultButton(btn2);
        msg.exec();
        if (msg.clickedButton() == btn1)
        {
            if (!showRegisterDialog())
                return;
            Global::setAutoconnection(true);
        }
        else if (msg.clickedButton() == btn2)
        {
            if (BSettingsDialog(new TexsampleSettingsTab).exec() != BSettingsDialog::Accepted)
                return;
            sClient->connectToServer();
        }
        else if (msg.clickedButton() == btn3)
        {
            return Global::setAutoconnection(false);
        }
    }
    else if (Global::autoconnection())
    {
        sClient->connectToServer();
    }
}

QWidget *Application::mostSuitableWindow()
{
    if ( !testAppInit() )
        return 0;
    QWidget *wgt = QApplication::activeWindow();
    QList<MainWindow *> list = bApp->mmainWindows.values();
    foreach (MainWindow *mw, list)
        if (mw == wgt)
            return wgt;
    return !list.isEmpty() ? list.first() : 0;
}

QList<BCodeEditor *> Application::codeEditors()
{
    QList<BCodeEditor *> list;
    foreach (MainWindow *mw, bApp->mmainWindows)
        list << mw->codeEditor();
    return list;
}

bool Application::mergeWindows()
{
    if (bApp->mmainWindows.size() < 2)
        return true;
    QList<MainWindow *> list = bApp->mmainWindows.values();
    MainWindow *first = list.takeFirst();
    foreach (MainWindow *mw, list)
    {
        first->codeEditor()->mergeWith(mw->codeEditor());
        mw->close();
    }
    first->activateWindow();
    return true;
}

void Application::handleExternalRequest(const QStringList &args)
{
    if ( !testAppInit() )
        return;
    if (Global::multipleWindowsEnabled())
    {
        bApp->addMainWindow(args);
    }
    else
    {
        MainWindow *mw = !bApp->mmainWindows.isEmpty() ? bApp->mmainWindows.values().first() : 0;
        if (!mw)
            return;
        mw->codeEditor()->openDocuments(args);
    }
}

bool Application::showPasswordDialog(QWidget *parent)
{
    PasswordDialog pd(parent ? parent : mostSuitableWindow());
    if (pd.exec() != QDialog::Accepted)
        return false;
    Global::setPasswordSate(pd.passwordState());
    sClient->updateSettings();
    return true;
}

bool Application::showRegisterDialog(QWidget *parent)
{
    QDialog dlg(parent ? parent : mostSuitableWindow());
    dlg.setWindowTitle(tr("Registration", "dlg windowTitle"));
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      TUserWidget *uwgt = new TUserWidget(TUserWidget::RegisterMode);
        uwgt->restorePasswordWidgetState(Global::passwordWidgetState());
      vlt->addWidget(uwgt);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox;
        dlgbbox->addButton(QDialogButtonBox::Ok);
        dlgbbox->button(QDialogButtonBox::Ok)->setEnabled(false);
        connect(uwgt, SIGNAL(validityChanged(bool)), dlgbbox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
        connect(dlgbbox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dlg, SLOT(accept()));
        dlgbbox->addButton(QDialogButtonBox::Cancel);
        connect(dlgbbox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dlg, SLOT(reject()));
      vlt->addWidget(dlgbbox);
      dlg.setMinimumWidth(700);
      dlg.setFixedHeight(dlg.sizeHint().height());
    while (dlg.exec() == QDialog::Accepted)
    {
        TUserInfo info = uwgt->info();
        TOperationResult r = Client::registerUser(info, dlg.parentWidget());
        if (r)
        {
            Global::setLogin(info.login());
            Global::setPassword(uwgt->password());
            Global::setPasswordWidgetSate(uwgt->savePasswordWidgetState());
            sClient->updateSettings();
            sClient->connectToServer();
            return true;
        }
        else
        {
            QMessageBox msg(dlg.parentWidget());
            msg.setWindowTitle(tr("Registration error", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Failed to register due to the following error:", "msgbox text"));
            msg.setInformativeText(r.messageString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
        }
    }
    Global::setPasswordWidgetSate(uwgt->savePasswordWidgetState());
    return false;
}

bool Application::showSettings(Settings type, QWidget *parent)
{
    if (parent)
        parent = mostSuitableWindow();
    switch (type)
    {
    case AccountSettings:
    {
        BDialog dlg(parent);
          dlg.setWindowTitle(tr("Updating account", "dlg windowTitle"));
          TUserWidget *uwgt = new TUserWidget(TUserWidget::UpdateMode);
            TUserInfo info(TUserInfo::UpdateContext);
            sClient->getUserInfo(sClient->userId(), info, parent);
            uwgt->setInfo(info);
            uwgt->restorePasswordWidgetState(Global::passwordWidgetState());
            uwgt->restoreState(bSettings->value("UpdateUserDialog/user_widget_state").toByteArray());
            uwgt->setPassword(Global::password());
          dlg.setWidget(uwgt);
          dlg.addButton(QDialogButtonBox::Ok, SLOT(accept()));
          dlg.button(QDialogButtonBox::Ok)->setEnabled(uwgt->isValid());
          connect(uwgt, SIGNAL(validityChanged(bool)), dlg.button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
          dlg.addButton(QDialogButtonBox::Cancel, SLOT(reject()));
          dlg.setMinimumSize(600, dlg.sizeHint().height());
          if (dlg.exec() != BDialog::Accepted)
              return false;
        info = uwgt->info();
        TOperationResult r = sClient->updateAccount(info, parent);
        if (r)
        {
            Global::setPassword(uwgt->password());
            Global::setPasswordWidgetSate(uwgt->savePasswordWidgetState());
            if (!sClient->updateSettings())
                sClient->reconnect();
            return true;
        }
        else
        {
            QMessageBox msg(parent);
            msg.setWindowTitle(tr("Changing account failed", "msgbox windowTitle"));
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("The following error occured:", "msgbox text"));
            msg.setInformativeText(r.messageString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
            Global::setPasswordWidgetSate(uwgt->savePasswordWidgetState());
            return false;
        }
    }
    case ConsoleSettings:
    {
        BSettingsDialog sd(new ConsoleSettingsTab, parent ? parent : mostSuitableWindow());
        return sd.exec() == BSettingsDialog::Accepted;
    }
    default:
        return false;
    }
}

void Application::emitUseRemoteCompilerChanged()
{
    if (!bApp)
        return;
    QMetaObject::invokeMethod(bApp, "useRemoteCompilerChanged");
}

void Application::updateDocumentType()
{
    if (!bApp)
        return;
    foreach (MainWindow *mw, bApp->mmainWindows)
        mw->codeEditor()->setDocumentType(Global::editorDocumentType());
}

BSpellChecker *Application::spellChecker()
{
    return bApp ? bApp->msc : 0;
}

/*============================== Protected methods =========================*/

QList<BAbstractSettingsTab *> Application::createSettingsTabs() const
{
    QList<BAbstractSettingsTab *> list;
    list << new GeneralSettingsTab;
    list << new CodeEditorSettingsTab;
    list << new ConsoleSettingsTab;
    list << new TexsampleSettingsTab;
    return list;
}

/*============================== Static private methods ====================*/

bool Application::testAppInit()
{
    return bTest(bApp, "Application", "There must be an Application instance");
}

/*============================== Private methods ===========================*/

void Application::addMainWindow(const QStringList &fileNames)
{
    MainWindow *mw = new MainWindow;
    mw->setAttribute(Qt::WA_DeleteOnClose, true);
    connect( mw, SIGNAL( destroyed(QObject *) ), this, SLOT( mainWindowDestroyed(QObject *) ) );
    BCodeEditor *edr = mw->codeEditor();
    connect( edr, SIGNAL( fileHistoryChanged(QStringList) ), this, SLOT( fileHistoryChanged(QStringList) ) );
    if ( !fileNames.isEmpty() )
        edr->openDocuments(fileNames);
    mmainWindows.insert(mw, mw);
    mw->show();
}

/*============================== Private slots =============================*/

void Application::mainWindowDestroyed(QObject *obj)
{
    mmainWindows.remove(obj);
}

void Application::fileHistoryChanged(const QStringList &history)
{
    Global::setFileHistory(history);
}
