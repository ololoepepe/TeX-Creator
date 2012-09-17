#ifndef SYMBOLSWIDGET_H
#define SYMBOLSWIDGET_H

class QWidget;

#include <QTabWidget>

class SymbolsWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit SymbolsWidget(QWidget *parent = 0);
private:
    void loadSection(const QString &fileName);
    QString sectionTitle(int index) const;
private slots:
    void retranslateUi();
signals:
    void insertText(const QString &text);
};

#endif // SYMBOLSWIDGET_H
