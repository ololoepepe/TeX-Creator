#ifndef SYMBOLSWIDGET_H
#define SYMBOLSWIDGET_H

class QWidget;
class QSignalMapper;

#include <QTabWidget>
#include <QStringList>

/*============================================================================
================================ SymbolsWidget ===============================
============================================================================*/

class SymbolsWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit SymbolsWidget(QWidget *parent = 0);
private:
    void loadSection(int lbound, int ubound);
    QString sectionTitle(int index) const;
private slots:
    void retranslateUi();
signals:
    void insertText(const QString &text);
private:
    QSignalMapper *mmpr;
    QStringList mtexts;
private:
    Q_DISABLE_COPY(SymbolsWidget)
};

#endif // SYMBOLSWIDGET_H
