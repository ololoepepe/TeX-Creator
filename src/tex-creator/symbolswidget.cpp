/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
**
** This file is part of TeX Creator.
**
** TeX Creator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** TeX Creator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with TeX Creator.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "symbolswidget.h"
#include "application.h"

#include <BFlowLayout>
#include <BApplication>
#include <BDirTools>

#include <QTabWidget>
#include <QWidget>
#include <QSizePolicy>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QVariant>
#include <QScrollArea>
#include <QSignalMapper>
#include <QToolButton>
#include <QIcon>
#include <QSize>
#include <QStringList>

/*============================================================================
================================ SymbolsWidget ===============================
============================================================================*/

/*============================== Public constructors =======================*/

SymbolsWidget::SymbolsWidget(QWidget *parent) :
    QTabWidget(parent)
{
    mmpr = new QSignalMapper(this);
      connect( mmpr, SIGNAL( mapped(QString) ), this, SIGNAL( insertText(QString) ) );
    mtexts << "";
    QFile f( BDirTools::findResource("symbols/symbols.txt", BDirTools::GlobalOnly) );
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
    loadSection(373, 412); //greek letters
    //
    retranslateUi();
    connect( bApp, SIGNAL( languageChanged() ), this, SLOT( retranslateUi() ) );
}

/*============================== Private methods ===========================*/

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
          foreach (int i, bRange(lbound, ubound))
          {
              QToolButton *tb = new QToolButton;
              tb->setToolTip( mtexts.at(i) );
              tb->setIconSize( QSize(32, 32) );
              QIcon icn( BDirTools::findResource("symbols/img" + QString::number(i) + ".png", BDirTools::GlobalOnly) );
              tb->setIcon(icn);
              bSetMapping(mmpr, tb, SIGNAL(clicked()), mtexts.at(i));
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
        title = tr("Relations", "macroSection title");
        break;
    case 1:
        title = tr("Separators", "macroSection title");
        break;
    case 2:
        title = tr("Arrows", "macroSection title");
        break;
    case 3:
        title = tr("Other", "macroSection title");
        break;
    case 4:
        title = tr("Greek letters", "macroSection title");
        break;
    default:
        break;
    }
    return title;
}

/*============================== Private slots =============================*/

void SymbolsWidget::retranslateUi()
{
    foreach (int i, bRangeD(0, count() - 1))
        setTabText( i, sectionTitle(i) );
}
