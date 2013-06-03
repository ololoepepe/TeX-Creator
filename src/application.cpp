#include "application.h"
#include "client.h"
#include "texsamplesettingstab.h"
#include "mainwindow.h"
#include "userwidget.h"
#include "global.h"

#include <TUserInfo>
#include <TOperationResult>

#include <BApplication>
#include <BSettingsDialog>
#include <BCodeEditor>
#include <BPasswordWidget>
#include <BAbstractSettingsTab>
#include <BLocaleComboBox>

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

#include <QDebug>

/*============================================================================
================================ AccountSettingsTab ==========================
============================================================================*/

class AccountSettingsTab : public BAbstractSettingsTab
{
    Q_DECLARE_TR_FUNCTIONS(AccountSettingsTab)
public:
    explicit AccountSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool saveSettings();
private:
    UserWidget *muwgt;
private:
    Q_DISABLE_COPY(AccountSettingsTab)
};

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
    bool restoreDefault();
    bool saveSettings();
private:
    QFontComboBox *mfntcmbox;
    QSpinBox *msboxFontPointSize;
    QSpinBox *msboxLineLength;
    QComboBox *mcmboxTabWidth;
    QComboBox *mcmboxEncoding;
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
    bool hasAdvancedMode() const;
    void setAdvancedMode(bool enabled);
    bool restoreDefault();
    bool saveSettings();
private:
    QComboBox *mcmboxName;
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
================================ AccountSettingsTab ==========================
============================================================================*/

/*============================== Public constructors =======================*/

AccountSettingsTab::AccountSettingsTab() :
    BAbstractSettingsTab()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      muwgt = new UserWidget(sClient->accessLevel() >= TAccessLevel::AdminLevel ? UserWidget::EditMode :
                                                                                  UserWidget::UpdateMode);
      vlt->addWidget(muwgt);
    TUserInfo info(TUserInfo::UpdateContext);
    sClient->getUserInfo(sClient->userId(), info, this);
    muwgt->setInfo(info);
    muwgt->setPasswordState(Global::passwordState());
}

/*============================== Public methods ============================*/

QString AccountSettingsTab::title() const
{
    return tr("Account", "title");
}

QIcon AccountSettingsTab::icon() const
{
    return Application::icon("user");
}

bool AccountSettingsTab::saveSettings()
{
    if (!muwgt->passwordsMatch())
    {
        //TODO: Show message
        return false;
    }
    TUserInfo info = muwgt->info();
    if (!info.isValid())
    {
        //TODO: Show message
        return false;
    }
    TOperationResult r = sClient->updateAccount(info, this);
    if (r)
    {
        Global::setPasswordSate(muwgt->passwordState());
        if (!sClient->updateSettings())
            sClient->reconnect();
        return true;
    }
    else
    {
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Changing account failed", "msgbox windowTitle"));
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("The following error occured:", "msgbox text"));
        msg.setInformativeText(r.errorString());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
        return false;
    }
}

/*============================================================================
================================ CodeEditorSettingsTab =======================
============================================================================*/

/*============================== Public constructors =======================*/

CodeEditorSettingsTab::CodeEditorSettingsTab()
{
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("Font", "gbox title"), this);
        QFormLayout *flt = new QFormLayout;
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
          flt->addRow(tr("Line length:", "lbl text"), msboxLineLength);
          mcmboxTabWidth = new QComboBox(gbox);
            mcmboxTabWidth->addItem(QString::number(BCodeEdit::TabWidth2), BCodeEdit::TabWidth2);
            mcmboxTabWidth->addItem(QString::number(BCodeEdit::TabWidth4), BCodeEdit::TabWidth4);
            mcmboxTabWidth->addItem(QString::number(BCodeEdit::TabWidth8), BCodeEdit::TabWidth8);
            mcmboxTabWidth->setCurrentIndex(mcmboxTabWidth->findData(Global::editTabWidth()));
          flt->addRow(tr("Tab width:", "lbl text"), mcmboxTabWidth);
        gbox->setLayout(flt);
      vlt->addWidget(gbox);
      gbox = new QGroupBox(tr("Files", "gbox title"), this);
        flt = new QFormLayout;
          mcmboxEncoding = BCodeEditor::createStructuredCodecsComboBox(gbox);
            BCodeEditor::selectCodec(mcmboxEncoding, Global::defaultCodec());
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

bool CodeEditorSettingsTab::restoreDefault()
{
    QFont fnt = Application::createMonospaceFont();
    mfntcmbox->setCurrentFont(fnt);
    msboxFontPointSize->setValue( fnt.pointSize() );
    BCodeEditor::selectCodec(mcmboxEncoding, QTextCodec::codecForLocale());
    msboxLineLength->setValue(120);
    mcmboxTabWidth->setCurrentIndex( mcmboxTabWidth->findData(BCodeEdit::TabWidth4) );
    return true;
}

bool CodeEditorSettingsTab::saveSettings()
{
    Global::setEditFontFamily(mfntcmbox->currentFont().family());
    Global::setEditFontPointSize(msboxFontPointSize->value());
    Global::setDefaultCodec(BCodeEditor::selectedCodec(mcmboxEncoding));
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
          mcmboxName = new QComboBox(gbox);
            QStringList sl;
            sl << "pdflatex";
            sl << "pdftex";
            sl << "latex";
            sl << "tex";
            mcmboxName->addItems(sl);
            mcmboxName->setCurrentIndex(mcmboxName->findText(Global::compilerName()));
          flt->addRow(tr("Compiler:", "label text"), mcmboxName);
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
    //
    setRowVisible(mledtOptions, false);
    setRowVisible(mledtCommands, false);
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

bool ConsoleSettingsTab::hasAdvancedMode() const
{
    return true;
}

void ConsoleSettingsTab::setAdvancedMode(bool enabled)
{
    setRowVisible(mledtOptions, enabled);
    setRowVisible(mledtCommands, enabled);
}

bool ConsoleSettingsTab::restoreDefault()
{
    mcmboxName->setCurrentIndex( mcmboxName->findText("pdflatex") );
    return true;
}

bool ConsoleSettingsTab::saveSettings()
{
    Global::setCompilerName(mcmboxName->currentText());
    Global::setCompilerOptions(mledtOptions->text());
    Global::setCompilerCommands(mledtCommands->text());
    Global::setMakeindexEnabled(mcboxMakeindex->isChecked());
    Global::setDvipsEnabled(mcboxDvips->isChecked());
    Global::setUseRemoteCompiler(mcboxRemoteCompiler->isChecked());
    if (Global::hasFallbackToLocalCompiler() || mcboxFallbackToLocalCompiler->isChecked())
        Global::setFallbackToLocalCompiler(mcboxFallbackToLocalCompiler->isChecked());
    Global::setAlwaysLatinEnabled(mcboxAlwaysLatin->isChecked());
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
    flt->addRow("Language:", mlcmbox); //Yep, this label should never be translated
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
        mpwdwgt->restoreState(Global::passwordState());
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
    mpwdwgt->restoreState(state);
}

QByteArray PasswordDialog::passwordState() const
{
    return mpwdwgt->saveStateEncrypted();
}

/*============================================================================
================================ Application =================================
============================================================================*/

/*============================== Public constructors =======================*/

Application::Application() :
    BApplication()
{
    minitialWindowCreated = false;
}

Application::~Application()
{
    BPasswordWidget::PasswordWidgetData pwd = BPasswordWidget::stateToData(Global::passwordState());
    if (!pwd.save)
    {
        pwd.password.clear();
        pwd.encryptedPassword.clear();
        Global::setPasswordSate( BPasswordWidget::dataToState(pwd) );
    }
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
        if ( !first->codeEditor()->mergeWith( mw->codeEditor() ) )
            return false;
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
      UserWidget *uwgt = new UserWidget(UserWidget::RegisterMode);
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
      dlg.setFixedSize(640, 260);
    while (dlg.exec() == QDialog::Accepted)
    {
        TUserInfo info = uwgt->info();
        TOperationResult r = Client::registerUser(info, uwgt->invite(), dlg.parentWidget());
        if (r)
        {
            Global::setLogin(info.login());
            Global::setPasswordSate(info.password());
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
            msg.setInformativeText(r.errorString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setDefaultButton(QMessageBox::Ok);
            msg.exec();
        }
    }
    return false;
}

bool Application::showSettings(Settings type, QWidget *parent)
{
    BAbstractSettingsTab *tab = 0;
    switch (type)
    {
    case AccountSettings:
        tab = new AccountSettingsTab;
        break;
    case ConsoleSettings:
        tab = new ConsoleSettingsTab;
        break;
    default:
        break;
    }
    BSettingsDialog sd(tab, parent ? parent : mostSuitableWindow());
    return sd.exec() == BSettingsDialog::Accepted;
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
