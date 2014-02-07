#ifndef MACROSSETTINGSTAB_H
#define MACROSSETTINGSTAB_H

class QString;
class QIcon;
class QFormLayout;
class QLineEdit;
class QHBoxLayout;

#include <BAbstractSettingsTab>

#include <QObject>
#include <QString>
#include <QMap>

/*============================================================================
================================ MacrosSettingsTab ===========================
============================================================================*/

class MacrosSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit MacrosSettingsTab();
public:
    QString title() const;
    QIcon icon() const;
    bool saveSettings();
private slots:
    void addRow(const QString &name = QString(), const QString &path = QString());
    void removeRow();
    void search();
private:
    QFormLayout *flt;
    QMap<QObject *, QLineEdit *> pathMap;
    QMap<QObject *, QHBoxLayout *> layoutMap;
private:
    Q_DISABLE_COPY(MacrosSettingsTab)
};

#endif // MACROSSETTINGSTAB_H
