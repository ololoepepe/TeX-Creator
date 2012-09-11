#ifndef SYMBOLSWIDGET_H
#define SYMBOLSWIDGET_H

class QWidget;
class QEvent;

#include <QTabWidget>

class SymbolsWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit SymbolsWidget(QWidget *parent = 0);
protected:
    void changeEvent(QEvent *event);
private:
    void retranslateUi();
    void loadSection(const QString &fileName);
    QString sectionTitle(int index) const;
signals:
    void insertText(const QString &text);
};

#endif // SYMBOLSWIDGET_H
