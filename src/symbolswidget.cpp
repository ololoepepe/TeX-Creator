#include "src/symbolswidget.h"
#include "src/macrosection.h"

#include <bcore.h>

#include <QTabWidget>
#include <QWidget>
#include <QSizePolicy>
#include <QString>
#include <QDomDocument>
#include <QFile>
#include <QVariant>

SymbolsWidget::SymbolsWidget(QWidget *parent) :
    QTabWidget(parent)
{
    setDocumentMode(true);
    QString bn = ":/res/symbols/";
    loadSection(bn + "arrows.xml");
    loadSection(bn + "greek.xml");
    loadSection(bn + "relations.xml");
    loadSection(bn + "separators.xml");
    loadSection(bn + "other.xml");
    retranslateUi();
    connect( BCore::instance(), SIGNAL( localeChanged() ), this, SLOT( retranslateUi() ) );
}

//

void SymbolsWidget::loadSection(const QString &fileName)
{
    QDomDocument doc("QATE_MACROS");
    QFile f(fileName);
    if ( !f.open(QFile::ReadOnly) )
        return;
    if ( !doc.setContent(&f) )
        return f.close();
    f.close();
    QDomElement el = doc.documentElement();
    MacroSection *section = new MacroSection(el);
    if ( section->isNull() )
        return section->deleteLater();
    connect( section, SIGNAL( insertText(QString) ), this, SIGNAL( insertText(QString) ) );
    addTab(section, "");
}

QString SymbolsWidget::sectionTitle(int index) const
{
    QString title;
    switch (index)
    {
    case 0:
        title = tr("Arrow symbols", "macroSection title");
        break;
    case 1:
        title = tr("Greek letters", "macroSection title");
        break;
    case 2:
        title = tr("Relations symbols", "macroSection title");
        break;
    case 3:
        title = tr("Separators", "macroSection title");
        break;
    case 4:
        title = tr("Other symbols", "macroSection title");
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
