#include "editsampledialog.h"
#include "sample.h"
#include "client.h"
#include "application.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVariant>
#include <QString>
#include <QPushButton>
#include <QStringList>
#include <QChar>
#include <QMessageBox>

/*============================================================================
================================ EditSampleDialog ============================
============================================================================*/

/*============================== Public constructors =======================*/

EditSampleDialog::EditSampleDialog(const Sample *s, QWidget *parent) :
    QDialog(parent), S(s)
{
    resize(550, 450);
    QVBoxLayout *vlt = new QVBoxLayout(this);
      QGroupBox *gbox = new QGroupBox(tr("General", "gbox title"), this);
        QVBoxLayout *vltg = new QVBoxLayout;
          QFormLayout *flt = new QFormLayout;
            mledtTitle = new QLineEdit(gbox);
              mledtTitle->setText( s->title() );
              connect(mledtTitle, SIGNAL(textChanged(QString)), this, SLOT(checkButtons()));
            flt->addRow(tr("Title:", "lbl text"), mledtTitle);
            mledtTags = new QLineEdit(gbox);
              mledtTags->setText( s->tagsToString() );
            flt->addRow(tr("Tags:", "lbl text"), mledtTags);
          vltg->addLayout(flt);
          vltg->addWidget(new QLabel(tr("Comment:", "lbl text"), gbox));
          mptedtComment = new QPlainTextEdit(gbox);
            mptedtComment->setTabChangesFocus(true);
            mptedtComment->setPlainText( s->comment() );
          vltg->addWidget(mptedtComment);
        gbox->setLayout(vltg);
      vlt->addWidget(gbox);
      if (sClient->accessLevel() >= Client::ModeratorLevel)
      {
          gbox = new QGroupBox(tr("Admin", "gbox title"), this);
            vltg = new QVBoxLayout;
              flt = new QFormLayout;
                mcmboxType = new QComboBox(gbox);
                  mcmboxType->addItem(Sample::typeToLocalizedString(Sample::Unverified), Sample::Unverified);
                  mcmboxType->addItem(Sample::typeToLocalizedString(Sample::Approved), Sample::Approved);
                  mcmboxType->addItem(Sample::typeToLocalizedString(Sample::Rejected), Sample::Rejected);
                  mcmboxType->setCurrentIndex( mcmboxType->findData( s->type() ) );
                flt->addRow(tr("Type:", "lbl text"), mcmboxType);
                msboxRating = new QSpinBox(gbox);
                  msboxRating->setMinimum(0);
                  msboxRating->setMaximum(100);
                  msboxRating->setValue( s->rating() );
                flt->addRow(tr("Rating:", "lbl text"), msboxRating);
              vltg->addLayout(flt);
              vltg->addWidget(new QLabel(tr("Remark:", "lbl text"), gbox));
              mptedtRemark = new QPlainTextEdit(gbox);
                mptedtRemark->setTabChangesFocus(true);
                mptedtRemark->setPlainText( s->adminRemark() );
              vltg->addWidget(mptedtRemark);
            gbox->setLayout(vltg);
          vlt->addWidget(gbox);
      }
      else
      {
          mcmboxType = 0;
          msboxRating = 0;
          mptedtRemark = 0;
      }
      vlt->addStretch();
      QDialogButtonBox *dlgbbox = new QDialogButtonBox(this);
        connect(dlgbbox->addButton(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(dlgbbox->addButton(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
      vlt->addWidget(dlgbbox);
    //
    checkButtons();
    connect(this, SIGNAL(accepted()), this, SLOT(updateSample()));
}

/*============================== Private slots =============================*/

void EditSampleDialog::updateSample()
{
    Sample s;
    s.setId( S->id() );
    s.setTitle( mledtTitle->text() );
    s.setTags( Sample::stringToTags( mledtTags->text() ) );
    s.setComment( mptedtComment->toPlainText().replace(QChar::ParagraphSeparator, '\n') );
    bool b = s.title() != S->title() || s.tags() != S->tags() || s.comment() != S->comment();
    if (mcmboxType)
    {
        s.setType( static_cast<Sample::Type>( mcmboxType->itemData( mcmboxType->currentIndex() ).toInt() ) );
        s.setRating( msboxRating->value() );
        s.setAdminRemark( mptedtRemark->toPlainText().replace(QChar::ParagraphSeparator, '\n') );
        b = b || s.type() != S->type() || s.rating() != S->rating() || s.adminRemark() != S->adminRemark();
    }
    if (!b)
        return;
    if ( !sClient->updateSample(s, mcmboxType, this) )
    {
        QMessageBox msg(this);
        msg.setWindowTitle( tr("Editing sample failed", "msgbox windowTitle") );
        msg.setIcon(QMessageBox::Critical);
        msg.setText( tr("Failed to edit the sample", "msgbox text") );
        msg.setInformativeText( tr("This may be due to connection error", "msgbox informativeText") );
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setDefaultButton(QMessageBox::Ok);
        msg.exec();
    }
}

void EditSampleDialog::checkButtons()
{
    findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->setEnabled( !mledtTitle->text().isEmpty() );
}
