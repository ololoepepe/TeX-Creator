#ifndef MACROEXECUTIONSTACK_H
#define MACROEXECUTIONSTACK_H

class QByteArray;

#include <QMap>
#include <QString>

/*============================================================================
================================ MacroExecutionStack =========================
============================================================================*/

class MacroExecutionStack
{
public:
    explicit MacroExecutionStack(MacroExecutionStack *parent = 0);
public:
    bool define(const QString &id, const QString &value, bool global = false);
    bool undefine(const QString &id);
    bool set(const QString &id, const QString &value);
    bool get(const QString &id, QString &value) const;
    bool isDefined(const QString &id) const;
    QByteArray save() const;
    void restore(const QByteArray &data);
private:
    MacroExecutionStack *mparent;
    QMap<QString, QString> mmap;
};

#endif // MACROEXECUTIONSTACK_H
