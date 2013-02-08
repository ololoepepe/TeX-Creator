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
      QPixmap pm = QPixmap::fromImage(info.avatar);
      if ( !pm.isNull() )
      {
          int max = qMax( pm.height(), pm.width() );
          if (max <= MaxPixmapSize)
          {
              lbl = new QLabel(this);
                lbl->setPixmap(pm);
              vlt->addWidget(lbl);
          }
          else
          {
              pm = pm.scaled( MaxPixmapSize, MaxPixmapSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
              QToolButton *tbtn = new QToolButton(this);
                tbtn->setIconSize( QSize( pm.width(), pm.height() ) );
                tbtn->setIcon( QIcon(pm) );
                tbtn->setToolTip( tr("Click to show the avatar in full size", "tbtn text") );
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

/*============================== Private slots =============================*/

void UserInfoDialog::showFullAvatar()
{
    QPixmap pm = QPixmap::fromImage(Info.avatar);
    if ( pm.isNull() )
        return;
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Avatar:", "dlg windowTitle") + " " + Info.login);
    QVBoxLayout *vlt = new QVBoxLayout(&dlg);
      QLabel *lbl = new QLabel(&dlg);
        lbl->setPixmap(pm);
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
    resize(300, 250);
    QVBoxLayout *vlt = new QVBoxLayout(this);
      mlblTitle = new QLabel(this);
        QFont fnt = mlblTitle->font();
        fnt.setPointSize(fnt.pointSize() + 2);
        mlblTitle->setFont(fnt);
        mlblTitle->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse
                                           | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
      vlt->addWidget(mlblTitle);
      mlblAuthor = new QLabel(this);
        fnt.setPointSize(fnt.pointSize() - 1);
        mlblAuthor->setFont(fnt);
        mlblAuthor->setTextInteractionFlags( mlblTitle->textInteractionFlags() );
        mlblAuthor->setToolTip( tr("Click the nickname to show user details", "lbl toolTip") );
        connect( mlblAuthor, SIGNAL( linkActivated(QString) ), this, SLOT( showAuthorInfo(QString) ) );
      vlt->addWidget(mlblAuthor);
      mlblType = new QLabel(this);
        mlblType->setTextInteractionFlags( mlblTitle->textInteractionFlags() );
      vlt->addWidget(mlblType);
      mlblModified = new QLabel(this);
        mlblModified->setTextInteractionFlags( mlblTitle->textInteractionFlags() );
      vlt->addWidget(mlblModified);
      mlblComment = new QLabel(this);
        mlblComment->setWordWrap(true);
        mlblComment->setTextInteractionFlags( mlblTitle->textInteractionFlags() );
      vlt->addWidget(mlblComment);
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        dlgbbox->addButton(QDialogButtonBox::Close);
        dlgbbox->button(QDialogButtonBox::Close)->setDefault(true);
        connect( dlgbbox->button(QDialogButtonBox::Close), SIGNAL( clicked() ), this, SLOT( close() ) );
      vlt->addWidget(dlgbbox);
    //
    mlblTitle->setText(s->title() + " [" + s->idToString(7) + "]");
    mlblAuthor->setText(tr("Author:", "lbl text") + " <a href=\"" + s->author() + "\">" + s->author() + "<a>");
    mlblType->setText( s->typeToString(Sample::LocalizedFormat) + " " + tr("sample", "lbl text") +
                       " (" + tr("rating:", "lbl text") + " " + s->ratingToString() + ")" );
    mlblModified->setText( tr("Was modified on:", "lbl text") + " " +
                           s->lastModified().toString("yyyy.MM.dd hh:mm UTC") );
    mlblComment->setText( "<b>" + tr("Comment:", "lbl text") + "</b><br>" + s->comment() );
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
