﻿#include "application.h"
#include "client.h"
#include "codeeditorsettingstab.h"
#include "generalsettingstab.h"
#include "texsamplesettingstab.h"
#include "mainwindow.h"
#include "consolesettingstab.h"
#include "userwidget.h"

#include <TUserInfo>
#include <TOperationResult>

#include <BApplication>
#include <BSettingsDialog>
#include <BCodeEditor>
#include <BPasswordWidget>
#include <BAbstractSettingsTab>

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

#include <QDebug>

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
================================ PasswordWidget ==============================
============================================================================*/

/*============================== Public constructors =======================*/

PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle( tr("TeXSample password", "windowTitle") );
    QVBoxLayout *vlt = new QVBoxLayout(this);
      mpwdwgt = new BPasswordWidget(this);
        mpwdwgt->restoreState( TexsampleSettingsTab::getPasswordState() );
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
    BPasswordWidget::PasswordWidgetData pwd = BPasswordWidget::stateToData( TexsampleSettingsTab::getPasswordState() );
    if (!pwd.save)
    {
        pwd.password.clear();
        pwd.encryptedPassword.clear();
        TexsampleSettingsTab::setPasswordSate( BPasswordWidget::dataToState(pwd) );
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
    if (!TexsampleSettingsTab::hasTexsample())
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
            return TexsampleSettingsTab::setAutoconnection(false);
        }
    }
    else if (TexsampleSettingsTab::getAutoconnection())
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
    if ( GeneralSettingsTab::getMultipleWindowsEnabled() )
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
    TexsampleSettingsTab::setPasswordSate(pd.passwordState());
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
      dlg.setFixedSize(640, 240);
    while (dlg.exec() == QDialog::Accepted)
    {
        TUserInfo info = uwgt->info();
        TOperationResult r = Client::registerUser(info, uwgt->invite(), dlg.parentWidget());
        if (r)
        {
            TexsampleSettingsTab::setLogin(info.login());
            TexsampleSettingsTab::setPasswordSate(info.password());
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
    CodeEditorSettingsTab::setFileHistory(history);
}
