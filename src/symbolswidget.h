#ifndef SYMBOLSWIDGET_H
#define SYMBOLSWIDGET_H

class QWidget;
class QSignalMapper;

#include <QTabWidget>
#include <QSize>
#include <QStringList>

class SymbolsWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit SymbolsWidget(QWidget *parent = 0);
private:
    static const QSize TBtnIconSize;
    //
    QSignalMapper *mmapper;
    QStringList mtexts;
    //
    void loadSection(int lbound, int ubound);
    QString sectionTitle(int index) const;
private slots:
    void retranslateUi();
signals:
    void insertText(const QString &text);
};

#endif // SYMBOLSWIDGET_H
