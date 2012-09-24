#include "src/symbolswidget.h"

#include <bcore.h>
#include <bflowlayout.h>

#include <QTabWidget>
#include <QWidget>
#include <QSizePolicy>
#include <QString>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QVariant>
#include <QScrollArea>
#include <QSignalMapper>
#include <QToolButton>
#include <QIcon>
#include <QSize>
#include <QStringList>

const QSize SymbolsWidget::TBtnIconSize = QSize(32, 32);

//

SymbolsWidget::SymbolsWidget(QWidget *parent) :
    QTabWidget(parent)
{
    mmapper = new QSignalMapper(this);
      connect( mmapper, SIGNAL( mapped(QString) ), this, SIGNAL( insertText(QString) ) );
    mtexts << "";
    QFile f(":/res/symbols/symbols.txt");
    f.open(QFile::ReadOnly);
    QTextStream in(&f);
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        if ( !line.isEmpty() && '#' != line.at(0) )
            mtexts << line;
    }
    f.close();
    //
    setDocumentMode(true);
    setTabPosition(West);
    loadSection(1, 226); //relations
    loadSection(227, 247); //separators
    loadSection(248, 314); //arrows
    loadSection(315, 372); //other
    loadSection(373, 412); //greek
    //
    retranslateUi();
    connect( BCore::instance(), SIGNAL( localeChanged() ), this, SLOT( retranslateUi() ) );
}

//

void SymbolsWidget::loadSection(int lbound, int ubound)
{
    QScrollArea *sa = new QScrollArea;
      sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      sa->setWidgetResizable(true);
      QWidget *wgt = new QWidget;
        BFlowLayout *fll = new BFlowLayout;
          fll->setContentsMargins(0, 0, 0, 0);
          fll->setSpacing(0);
          for (int i = lbound; i <= ubound; ++i)
          {
              QToolButton *tb = new QToolButton;
              QString tt = "\\" + mtexts.at(i);
              tb->setToolTip(tt);
              tb->setIconSize(TBtnIconSize);
              tb->setIcon( QIcon(":/res/ico/symbols/img" + QString::number(i) + ".png") );
              mmapper->setMapping(tb, tt);
              connect( tb, SIGNAL( clicked() ), mmapper, SLOT( map() ) );
              fll->addWidget(tb);
          }
        wgt->setLayout(fll);
      sa->setWidget(wgt);
      addTab(sa, "");
}

QString SymbolsWidget::sectionTitle(int index) const
{
    QString title;
    switch (index)
    {
    case 0:
        title = tr("Relations symbols", "macroSection title");
        break;
    case 1:
        title = tr("Separators", "macroSection title");
        break;
    case 2:
        title = tr("Arrow symbols", "macroSection title");
        break;
    case 3:
        title = tr("Other symbols", "macroSection title");
        break;
    case 4:
        title = tr("Greek letters", "macroSection title");
        break;
    default:
        break;
    }
    return title;
}

//

void SymbolsWidget::retranslateUi()
{
    for (int i = 0; i < count(); ++i)
        setTabText( i, sectionTitle(i) );
}
