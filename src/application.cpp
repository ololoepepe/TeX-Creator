#include "application.h"
#include "client.h"
#include "codeeditorsettingstab.h"
#include "generalsettingstab.h"
#include "texsamplesettingstab.h"
#include "mainwindow.h"
#include "consolesettingstab.h"
#include "registerdialog.h"

#include <BApplication>
#include <BSettingsDialog>
#include <BCodeEditor>
#include <BPasswordWidget>

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
    mclient = new Client(this);
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
    if ( TexsampleSettingsTab::getAutoconnection() )
    {
        if ( TexsampleSettingsTab::getLogin().isEmpty() )
        {
            QMessageBox msg( mostSuitableWindow() );
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
            }
            else if (msg.clickedButton() == btn3)
            {
                return TexsampleSettingsTab::setAutoconnection(false);
            }
            else
            {
                return;
            }
        }
        if ( TexsampleSettingsTab::getAutoconnection() )
        {
            if ( TexsampleSettingsTab::getPassword().isEmpty() )
                showPasswordDialog();
            sClient->connectToServer();
        }
    }
}

Client *Application::clientInstance()
{
    if ( !testAppInit() )
        return 0;
    return bApp->mclient;
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
    return RegisterDialog(parent ? parent : mostSuitableWindow()).exec() == RegisterDialog::Accepted;
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
