#ifndef MACROSECTION_H
#define MACROSECTION_H

class QSignalMapper;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;

#include <QScrollArea>
#include <QDomElement>
#include <QList>
#include <QString>

class MacroSection : public QScrollArea
{
    Q_OBJECT
public:
    explicit MacroSection(const QDomElement &element, QWidget *parent = 0);
    //
    bool isNull() const;
private:
    typedef QList<QDomElement> DomElementList;
    //
    bool mIsNull;
    QSignalMapper *mMapper;
    //
    QWidget *mWgt;
      QVBoxLayout *mVLt;
        //line
        //line
    //
    void loadSection(const QDomElement &element);
    void loadLine(const DomElementList &list);
    bool setButtonIcon(QToolButton *button, const QString &fileName);
private slots:
    void macroTriggered(const QString &text);
signals:
    void insertText(const QString &text);
};

#endif // MACROSECTION_H
