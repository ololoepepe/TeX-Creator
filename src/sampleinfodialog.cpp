#include "sampleinfodialog.h"
#include "sample.h"
#include "client.h"
#include "application.h"

#include <QDialog>
#include <QString>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFont>
#include <QImage>
#include <QPixmap>
#include <QToolButton>
#include <QSize>

#include <QDebug>

/*============================================================================
================================ UserInfoDialog ==============================
============================================================================*/

class UserInfoDialog : public QDialog
{
    Q_DECLARE_TR_FUNCTIONS(UserInfoDialog)
public:
    explicit UserInfoDialog(const Client::UserInfo &info, QWidget *parent = 0);
private:
    static QString pixmapInfo(const QPixmap &pm);
private slots:
    void showFullAvatar();
private:
    static const int MaxPixmapSize;
private:
    const Client::UserInfo Info;
private:
    Q_DISABLE_COPY(UserInfoDialog)
};

/*============================================================================
================================ UserInfoDialog ==============================
============================================================================*/

/*============================== Public constructors =======================*/

UserInfoDialog::UserInfoDialog(const Client::UserInfo &info, QWidget *parent) :
    QDialog(parent), Info(info)
{
    setWindowTitle(tr("User:", "windowTitle") + " " + info.login);
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QLabel *lbl = new QLabel(this);
        QFont fnt = lbl->font();
        fnt.setPointSize(fnt.pointSize() + 2);
        lbl->setFont(fnt);
        lbl->setText(info.login + " ("+ Client::accessLevelToLocalizedString(info.accessLevel).toLower() + ")");
      vlt->addWidget(lbl);
        if ( !info.realName.isEmpty() )
        {
            lbl = new QLabel(this);
            lbl->setText(tr("Real name:", "lbl text") + " " + info.realName);
            vlt->addWidget(lbl);
        }
      QPixmap pm;
      if ( pm.loadFromData(info.avatar) && !pm.isNull() )
      {
          int max = qMax( pm.height(), pm.width() );
          if (max <= MaxPixmapSize)
          {
              lbl = new QLabel(this);
                lbl->setPixmap(pm);
                lbl->setFixedSize( lbl->sizeHint() );
                lbl->setToolTip( pixmapInfo(pm) );
              vlt->addWidget(lbl);
          }
          else
          {
              pm = pm.scaled( MaxPixmapSize, MaxPixmapSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
              QToolButton *tbtn = new QToolButton(this);
                tbtn->setIconSize( QSize( pm.width(), pm.height() ) );
                tbtn->setIcon( QIcon(pm) );
                tbtn->setToolTip(pixmapInfo(pm)
                                 + " (" + tr("Click to show the avatar in full size", "tbtn text") + ")");
                connect(tbtn, &QToolButton::clicked, this, &UserInfoDialog::showFullAvatar);
              vlt->addWidget(tbtn);
          }
      }
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        dlgbbox->addButton(QDialogButtonBox::Close);
        dlgbbox->button(QDialogButtonBox::Close)->setDefault(true);
        connect( dlgbbox->button(QDialogButtonBox::Close), SIGNAL( clicked() ), this, SLOT( close() ) );
      vlt->addWidget(dlgbbox);
    //
    resize( 300, height() );
}

/*============================== Static private methods ====================*/

QString UserInfoDialog::pixmapInfo(const QPixmap &pm)
{
    return QString::number( pm.width() ) + "x" + QString::number( pm.height() );
}

/*============================== Private slots =============================*/

void UserInfoDialog::showFullAvatar()
{
    QPixmap pm;
    if ( pm.loadFromData(Info.avatar) && pm.isNull() )
        return;
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Avatar:", "dlg windowTitle") + " " + Info.login);
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      QLabel *lbl = new QLabel(&dlg);
        lbl->setPixmap(pm);
        lbl->setFixedSize( lbl->sizeHint() );
        lbl->setToolTip( pixmapInfo(pm) );
      vlt->addWidget(lbl);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(&dlg);
        dlgbbox->addButton(QDialogButtonBox::Close);
        dlgbbox->button(QDialogButtonBox::Close)->setDefault(true);
        connect( dlgbbox->button(QDialogButtonBox::Close), SIGNAL( clicked() ), &dlg, SLOT( close() ) );
      vlt->addWidget(dlgbbox);
    dlg.exec();
}

/*============================== Static private constants ==================*/

const int UserInfoDialog::MaxPixmapSize = 256;

/*============================================================================
================================ SampleInfoDialog ============================
============================================================================*/

/*============================== Public constructors =======================*/

SampleInfoDialog::SampleInfoDialog(const Sample *s, QWidget *parent) :
    QDialog(parent), S(s)
{
    if ( !s->title().isEmpty() )
        setWindowTitle( tr("Sample:", "windowTitle") + " " + s->title() );
    else
        setWindowTitle( tr("Sample info", "windowTitle") );
    resize(350, 250);
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QLabel *lbl = new QLabel(this);
        QFont fnt = lbl->font();
        fnt.setPointSize(fnt.pointSize() + 2);
        lbl->setFont(fnt);
        Qt::TextInteractionFlags tiflags = Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse
                                           | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
        lbl->setTextInteractionFlags(tiflags);
        lbl->setText(s->title() + " [" + s->idToString(7) + "]");
      vlt->addWidget(lbl);
      lbl = new QLabel(this);
        fnt.setPointSize(fnt.pointSize() - 1);
        lbl->setFont(fnt);
        lbl->setTextInteractionFlags(tiflags);
        lbl->setText(tr("Author:", "lbl text") + " <a href=\"" + s->author() + "\">" + s->author() + "<a>");
        lbl->setToolTip( tr("Click the nickname to show user details", "lbl toolTip") );
        connect( lbl, SIGNAL( linkActivated(QString) ), this, SLOT( showAuthorInfo(QString) ) );
      vlt->addWidget(lbl);
      lbl = new QLabel(this);
        lbl->setTextInteractionFlags(tiflags);
        lbl->setText( s->typeToString(Sample::LocalizedFormat) + " " + tr("sample", "lbl text")
                      + " (" + tr("rating:", "lbl text") + " " + s->ratingToString() + ")" );
      vlt->addWidget(lbl);
      lbl = new QLabel(this);
        lbl->setTextInteractionFlags(tiflags);
        lbl->setText( tr("Was modified on:", "lbl text") + " " + s->lastModified().toString("yyyy.MM.dd hh:mm UTC") );
      vlt->addWidget(lbl);
      if ( !s->comment().isEmpty() )
      {
          lbl = new QLabel(this);
            lbl->setWordWrap(true);
            lbl->setTextInteractionFlags(tiflags);
            lbl->setText( "<b>" + tr("Comment:", "lbl text") + "</b><br>" + s->comment() );
          vlt->addWidget(lbl);
      }
      QStringList tags = s->tags();
      tags.removeAll("");
      if (!tags.isEmpty())
      {
          lbl = new QLabel(this);
            lbl->setWordWrap(true);
            lbl->setTextInteractionFlags(tiflags);
            lbl->setText( "<b>" + tr("Tags:", "lbl text") + "</b><br>" + s->tagsToString() );
          vlt->addWidget(lbl);
      }
      if ( !s->adminRemark().isEmpty() )
      {
          lbl = new QLabel(this);
            lbl->setWordWrap(true);
            lbl->setTextInteractionFlags(tiflags);
            lbl->setText( "<b>" + tr("Admin remark:", "lbl text") + "</b><br>" + s->adminRemark() );
          vlt->addWidget(lbl);
      }
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        dlgbbox->addButton(QDialogButtonBox::Close);
        dlgbbox->button(QDialogButtonBox::Close)->setDefault(true);
        connect( dlgbbox->button(QDialogButtonBox::Close), SIGNAL( clicked() ), this, SLOT( close() ) );
      vlt->addWidget(dlgbbox);
}

/*============================== Public methods ============================*/

bool SampleInfoDialog::isValid() const
{
    return S;
}

/*============================== Private slots =============================*/

void SampleInfoDialog::showAuthorInfo(const QString &login)
{
    if ( login.isEmpty() )
        return;
    Client::UserInfo info = sClient->getUserInfo(login, this);
    if ( login.isEmpty() )
        return;
    UserInfoDialog(info, this).exec();
}
