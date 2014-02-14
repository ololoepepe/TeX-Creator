#include "macro.h"
#include "macrocommand.h"
#include "macroexecutionstack.h"

#include <BeQtGlobal>
#include <BDirTools>

#include <QString>
#include <QList>
#include <QCoreApplication>

/*============================================================================
================================ AbstractMacroCommand ========================
============================================================================*/


Macro::Macro()
{
    //
}

Macro::Macro(const QString &fileName)
{
    fromFile(fileName);
}

Macro::Macro(const Macro &other)
{
    *this = other;
}

Macro::~Macro()
{
    clear();
}


void Macro::clear()
{
    foreach (AbstractMacroCommand *mc, mcommands)
        delete mc;
    mcommands.clear();
}

void Macro::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !isValid())
        return bSet(error, QString("Internal error"));
    MacroExecutionStack localStack(stack);
    foreach (const AbstractMacroCommand *mc, mcommands)
    {
        QString err;
        mc->execute(doc, &localStack, &err);
        QCoreApplication::processEvents();
        if (!err.isEmpty())
            return bSet(error, err);
    }
    return bSet(error, QString());
}

bool Macro::recordKeyPress(const QKeyEvent *e, QString *error)
{
    AbstractMacroCommand *prev = !mcommands.isEmpty() ? mcommands.last() : 0;
    QString err;
    AbstractMacroCommand *mc = AbstractMacroCommand::fromKeyPressEvent(e, &err, prev);
    if (!mc)
        return bRet(error, err, false);
    if (prev != mc)
        mcommands << mc;
    return bRet(error, QString(), true);
}

bool Macro::fromText(const QString &text, QString *error)
{
    clear();
    QList<AbstractMacroCommand *> list;
    foreach (const QString &line, text.split('\n', QString::SkipEmptyParts))
    {
        if (line.startsWith("%"))
            continue;
        QString err;
        AbstractMacroCommand *mc = AbstractMacroCommand::fromText(line, &err);
        if (!mc)
        {
            foreach (AbstractMacroCommand *mc, list)
                delete mc;
            return bRet(error, err, false);
        }
        list << mc;
    }
    mcommands = list;
    return bRet(error, QString(), true);
}

bool Macro::fromFile(const QString &fileName, QString *error)
{
    clear();
    if (fileName.isEmpty())
        return bRet(error, QString("Internal error"), false);
    bool ok = false;
    QString text = BDirTools::readTextFile(fileName, "UTF-8", &ok);
    if (!ok)
        return bRet(error, QString("Failed to read file"), false);
    return fromText(text, error);
}

QString Macro::toText(QString *error) const
{
    if (!isValid())
        return bRet(error, QString("Internal error"), QString());
    QString s;
    foreach (const AbstractMacroCommand *mc, mcommands)
        s += (mc->toText() + "\n");
    if (!s.isEmpty())
        s.remove(s.length() - 1, 1);
    return bRet(error, QString(), s);
}

bool Macro::toFile(const QString &fileName, QString *error) const
{
    if (fileName.isEmpty() || !isValid())
        return bRet(error, QString("Internal error"), false);
    QString err;
    QString text = toText(&err);
    if (!err.isEmpty())
        return bRet(error, err, false);
    bool b = BDirTools::writeTextFile(fileName, text, "UTF-8");
    return bRet(error, QString(b ? "" : "Failed to write file"), b);
}

bool Macro::isValid() const
{
    foreach (const AbstractMacroCommand *mc, mcommands)
        if (!mc->isValid())
            return false;
    return true;
}

bool Macro::isEmpty() const
{
    return mcommands.isEmpty();
}

Macro &Macro::operator=(const Macro &other)
{
    clear();
    foreach (const AbstractMacroCommand *mc, other.mcommands)
        mcommands << mc->clone();
    return *this;
}
