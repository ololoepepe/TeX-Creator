#include "macro.h"
#include "macrocommand.h"
#include "macroexecutionstack.h"

#include <BeQtGlobal>
#include <BDirTools>
#include <BCodeEditor>
#include <BAbstractCodeEditorDocument>

#include <QString>
#include <QList>
#include <QCoreApplication>
#include <QTabBar>
#include <QObject>
#include <QEvent>
#include <QPlainTextEdit>
#include <QSet>
#include <QMetaObject>

#include <QDebug>

#include <cstring>

/*============================================================================
================================ SpontaneousEventEater =======================
============================================================================*/

class SpontaneousEventEater : public QObject
{
public:
    explicit SpontaneousEventEater(BAbstractCodeEditorDocument *doc);
public:
    bool eventFilter(QObject *o, QEvent *e);
private:
    QPlainTextEdit *mptedt;
};

/*============================================================================
================================ SpontaneousEventEater =======================
============================================================================*/

/*============================== Public constructors =======================*/

SpontaneousEventEater::SpontaneousEventEater(BAbstractCodeEditorDocument *doc)
{
    mptedt = doc ? doc->findChild<QPlainTextEdit *>() : 0;
    if (!mptedt)
        return;
    mptedt->installEventFilter(this);
    mptedt->viewport()->installEventFilter(this);
}

/*============================== Public methods ============================*/

bool SpontaneousEventEater::eventFilter(QObject *o, QEvent *e)
{
    typedef QSet<int> IntSet;
    init_once(IntSet, mouseEvents, IntSet())
    {
        mouseEvents.insert(QEvent::MouseButtonDblClick);
        mouseEvents.insert(QEvent::MouseButtonPress);
        mouseEvents.insert(QEvent::MouseButtonRelease);
        mouseEvents.insert(QEvent::MouseMove);
        mouseEvents.insert(QEvent::MouseTrackingChange);

    }
    if ((o != mptedt && mouseEvents.contains(e->type())) || (o == mptedt && e->spontaneous()))
    {
        e->ignore();
        return true;
    }
    else
    {
        return false;
    }
}

/*============================================================================
================================ Macro =======================================
============================================================================*/

/*============================== Public constructors =======================*/

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

/*============================== Public methods ============================*/

void Macro::clear()
{
    foreach (AbstractMacroCommand *mc, mcommands)
        delete mc;
    mcommands.clear();
}

void Macro::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, BCodeEditor *edtr,
                    QString *error) const
{
    if (!doc || !edtr || !isValid())
        return bSet(error, QString("Internal error"));
    BAbstractCodeEditorDocument *ddoc = edtr->currentDocument();
    if (!ddoc)
        return bSet(error, QString("Internal error"));
    edtr->findChild<QTabBar *>()->setVisible(false);
    ddoc->setReadOnly(true);
    SpontaneousEventEater sef(doc);
    foreach (const AbstractMacroCommand *mc, mcommands)
    {
        QString err;
        mc->execute(doc, stack, &err);
        QCoreApplication::processEvents();
        if (!err.isEmpty())
        {
            edtr->findChild<QTabBar *>()->setVisible(true);
            ddoc->setReadOnly(false);
            return bSet(error, err);
        }
    }
    edtr->findChild<QTabBar *>()->setVisible(true);
    ddoc->setReadOnly(false);
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
