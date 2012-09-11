#include "src/macrosection.h"

#include <QScrollArea>
#include <QDomElement>
#include <QWidget>
#include <QString>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDomNodeList>
#include <QList>
#include <QDomNode>
#include <QToolButton>
#include <QFileInfo>
#include <QDir>
#include <QPixmap>
#include <QTextStream>
#include <QSize>

MacroSection::MacroSection(const QDomElement &element, QWidget *parent) :
    QScrollArea(parent)
{
    mIsNull = true;
    if (element.isNull() || element.tagName() != "section")
        return;
    mMapper = new QSignalMapper(this);
    connect( mMapper, SIGNAL( mapped(QString) ), this, SLOT( macroTriggered(QString) ) );
    //
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mWgt = new QWidget;
      mVLt = new QVBoxLayout;
        mVLt->setContentsMargins(0, 0, 0, 0);
        mVLt->setSpacing(0);
      mWgt->setLayout(mVLt);
    loadSection(element);
    if (!mIsNull)
        setWidget(mWgt);
    else
        mWgt->deleteLater();
}

//

bool MacroSection::isNull() const
{
    return mIsNull;
}

//

void MacroSection::loadSection(const QDomElement &element)
{
    QDomNodeList nodeList = element.childNodes();
    DomElementList elementList;
    for (int i = 0; i < nodeList.size(); ++i)
    {
        QDomNode n = nodeList.at(i);
        if ( n.isNull() )
            continue;
        QDomElement el = n.toElement();
        if (el.isNull() || el.tagName() != "button")
            continue;
        elementList << el;
    }
    if ( elementList.isEmpty() )
        return;
    if (elementList.size() % 2)
        elementList << QDomElement();
    int mid = elementList.size() / 2;
    loadLine( elementList.mid(0, mid) );
    loadLine( elementList.mid(mid, mid) );
}

void MacroSection::loadLine(const DomElementList &list)
{
    QHBoxLayout *lt = new QHBoxLayout;
      lt->setContentsMargins(0, 0, 0, 0);
      lt->setSpacing(0);
    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement &el = list.at(i);
        if (el.isNull() || el.tagName() != "button")
            continue;
        QToolButton *tb = new QToolButton(mWgt);
        QString t = el.attribute("title");
        QString m = el.attribute("macro");
        if ( m.isEmpty() || !setButtonIcon( tb, el.attribute("icon") ) )
        {
            tb->deleteLater();
            continue;
        }
        if ( t.isEmpty() )
            tb->setToolTip(m);
        else
            tb->setToolTip(t);
        mMapper->setMapping(tb, m);
        connect( tb, SIGNAL( clicked() ), mMapper, SLOT( map() ) );
        lt->addWidget(tb);
        mIsNull = false;
    }
    if (!mIsNull)
    {
        lt->addStretch();
        mVLt->addLayout(lt);
    }
    else
    {
        lt->deleteLater();
    }
}

bool MacroSection::setButtonIcon(QToolButton *button, const QString &fileName)
{
    if ( !button || fileName.isEmpty() )
        return false;
    QPixmap pm(":/res/ico/symbols/" + fileName);
    if ( pm.isNull() )
        return false;
    button->setIcon( QIcon(pm) );
    button->setIconSize( QSize(32, 32) );
    return true;
}

//

void MacroSection::macroTriggered(const QString &text)
{
    QString t = text;
    QFile f(text);
    if ( f.open(QFile::ReadOnly) )
    {
        QTextStream in(&f);
        in.setCodec("UTF-8");
        t = in.readAll();
        if ( t.isEmpty() )
            return;
        f.close();
    }
    emit insertText(t);
}
