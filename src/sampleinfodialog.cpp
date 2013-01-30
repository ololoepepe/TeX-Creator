#include "sampleinfodialog.h"
#include "sample.h"

#include <QDialog>
#include <QString>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFont>

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
        mlblTitle->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
      vlt->addWidget(mlblTitle);
      mlblAuthor = new QLabel(this);
        fnt.setPointSize(fnt.pointSize() - 1);
        mlblAuthor->setFont(fnt);
        mlblAuthor->setTextInteractionFlags( mlblTitle->textInteractionFlags() );
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
    mlblAuthor->setText( tr("Author:", "lbl text") + " " + s->author() );
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
