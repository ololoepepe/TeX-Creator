/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
**
** This file is part of the MacrosEditorModule plugin of TeX Creator.
**
** TeX Creator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** TeX Creator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with TeX Creator.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "documentmacrocommands.h"
#include "macrocommand.h"
#include "macrocommandargument.h"
#include "executionstack.h"
#include "global.h"

#include <BeQtGlobal>
#include <BAbstractCodeEditorDocument>

#include <QList>
#include <QMap>
#include <QString>
#include <QTextDocument>
#include <QKeyEvent>
#include <QKeySequence>
#include <QPlainTextEdit>
#include <QCoreApplication>
#include <QKeySequence>

#include <QDebug>

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static QString getReplaceMndParameters(const QList<MacroCommandArgument> &args, BAbstractCodeEditorDocument *doc,
                                       ExecutionStack *stack, QString &first, QString *second = 0)
{
    QString err;
    QString s1 = Global::toRawText(args.first().toText(doc, stack, &err));
    if (!err.isEmpty())
        return err;
    if (second)
    {
        QString s2 = Global::toRawText(args.at(1).toText(doc, stack, &err));
        if (!err.isEmpty())
            return err;
        *second = s2;
    }
    first = s1;
    return "";
}

static QString getReplaceOptParameters(const QList<MacroCommandArgument> &args, BAbstractCodeEditorDocument *doc,
                                       ExecutionStack *stack, int from, Qt::CaseSensitivity &cs,
                                       bool *w = 0, bool *bw = 0, bool *nc = 0)
{
    Qt::CaseSensitivity csx = Qt::CaseInsensitive;
    bool wx = false;
    bool bwx = false;
    bool ncx = false;
    bool csb = false;
    bool wb = false;
    bool bwb = false;
    bool ncb = false;
    for (int i = from; i < args.size(); ++i)
    {
        QString err;
        QString s = args.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return err;
        if (!s.compare("cs", Qt::CaseInsensitive) || !s.compare("case-sensitive", Qt::CaseInsensitive))
        {
            if (csb)
                return "Duplicate parameter";
            csx = Qt::CaseSensitive;
            csb = true;
        }
        else if (!s.compare("ci", Qt::CaseInsensitive) || !s.compare("case-insensitive", Qt::CaseInsensitive))
        {
            if (csb)
                return "Duplicate parameter";
            csx = Qt::CaseInsensitive;
            csb = true;
        }
        else if (!s.compare("w", Qt::CaseInsensitive) || !s.compare("words", Qt::CaseInsensitive)
                || !s.compare("whole-words", Qt::CaseInsensitive))
        {
            if (!w)
                return "Unknown parameter";
            if (wb)
                return "Duplicate parameter";
            wx = true;
            wb = true;
        }
        else if (!s.compare("bw", Qt::CaseInsensitive) || !s.compare("backward", Qt::CaseInsensitive)
                || !s.compare("backward-order", Qt::CaseInsensitive))
        {
            if (!bw)
                return "Unknown parameter";
            if (bwb)
                return "Duplicate parameter";
            bwx = true;
            bwb = true;
        }
        else if (!s.compare("nc", Qt::CaseInsensitive) || !s.compare("non-cyclic", Qt::CaseInsensitive))
        {
            if (!nc)
                return "Unknown parameter";
            if (ncb)
                return "Duplicate parameter";
            ncx = true;
            ncb = true;
        }
        else
        {
            return "Unknown parameter";
        }
    }
    cs = csx;
    bSet(w, wx);
    bSet(bw, bwx);
    bSet(nc, ncx);
    return "";
}

static QTextDocument::FindFlags createFindFlags(Qt::CaseSensitivity cs, bool w, bool bw)
{
    QTextDocument::FindFlags flags = 0;
    if (Qt::CaseSensitive == cs)
        flags |= QTextDocument::FindCaseSensitively;
    if (w)
        flags |= QTextDocument::FindWholeWords;
    if (bw)
        flags |= QTextDocument::FindBackward;
    return flags;
}

/*============================================================================
================================ InsertMacroCommand ==========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *InsertMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 1) ? new InsertMacroCommand(args) : 0;
}

/*============================== Public constructors =======================*/

InsertMacroCommand::InsertMacroCommand(const QString &txt) :
    AbstractMacroCommand(QList<MacroCommandArgument>() << MacroCommandArgument(txt))
{
    //
}

/*============================== Protected constructors ====================*/

InsertMacroCommand::InsertMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString InsertMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString text = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    doc->insertText(Global::toRawText(text));
    return bRet(error, QString(), QString("true"));
}

QString InsertMacroCommand::name() const
{
    return "insert";
}

QString InsertMacroCommand::toText() const
{
    return isValid() ? ("\\insert{" + margs.first().toText() + "}") : QString();
}

AbstractMacroCommand *InsertMacroCommand::clone() const
{
    return new InsertMacroCommand(margs);
}

bool InsertMacroCommand::append(const QString &txt)
{
    if (txt.isEmpty() || !isValid() || !margs.first().isText())
        return false;
    return margs.first().fromText(margs.first().toText() + txt);
}

/*============================================================================
================================ PressMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *PressMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 1) ? new PressMacroCommand(args) : 0;
}

/*============================== Public constructors =======================*/

PressMacroCommand::PressMacroCommand(int k, Qt::KeyboardModifiers m) :
    AbstractMacroCommand(QList<MacroCommandArgument>()
                         << MacroCommandArgument(QKeySequence(k | m).toString(QKeySequence::PortableText)))
{
    //
}

/*============================== Protected constructors ====================*/

PressMacroCommand::PressMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString PressMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QPlainTextEdit *ptedt = doc->findChild<QPlainTextEdit *>();
    if (!ptedt)
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QKeySequence ks(margs.first().toText(doc, stack, &err));
    if (ks.isEmpty() || !err.isEmpty())
        return bRet(error, !err.isEmpty() ? err : QString("Invalid parameter"), QString("false"));
    for (int i = 0; i < (int) ks.count(); ++i)
    {
        int key = ~Qt::KeyboardModifierMask & ks[i];
        Qt::KeyboardModifiers modifiers = static_cast<Qt::KeyboardModifiers>(Qt::KeyboardModifierMask & ks[i]);
        QKeyEvent ke(QEvent::KeyPress, key, modifiers);
        QCoreApplication::sendEvent(ptedt, &ke);
        QCoreApplication::processEvents();
    }
    return bRet(error, QString(), QString("true"));
}

QString PressMacroCommand::name() const
{
    return "press";
}

QString PressMacroCommand::toText() const
{
    return isValid() ? ("\\press{" + margs.first().toText() + "}") : QString();
}

AbstractMacroCommand *PressMacroCommand::clone() const
{
    return new PressMacroCommand(margs);
}


/*============================================================================
================================ FindMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *FindMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 5) ? new FindMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

FindMacroCommand::FindMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString FindMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    bool w = false;
    bool bw = false;
    bool nc = false;
    QString what;
    QString err = getReplaceMndParameters(margs, doc, stack, what);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    err = getReplaceOptParameters(margs, doc, stack, 1, cs, &w, &bw, &nc);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    bool b = doc->findNext(what, createFindFlags(cs, w, bw), !nc);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString FindMacroCommand::name() const
{
    return "find";
}

QString FindMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\find{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *FindMacroCommand::clone() const
{
    return new FindMacroCommand(margs);
}

/*============================================================================
================================ ReplaceMacroCommand =========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ReplaceMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 6) ? new ReplaceMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ReplaceMacroCommand::ReplaceMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ReplaceMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack,
                                     QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    bool w = false;
    bool bw = false;
    bool nc = false;
    QString what;
    QString ntext;
    QString err = getReplaceMndParameters(margs, doc, stack, what, &ntext);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    err = getReplaceOptParameters(margs, doc, stack, 2, cs, &w, &bw, &nc);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    bool b = doc->findNext(what, createFindFlags(cs, w, bw), !nc);
    if (!b)
        bRet(error, QString(), QString("false"));
    b = doc->replaceNext(ntext);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString ReplaceMacroCommand::name() const
{
    return "replace";
}

QString ReplaceMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\replace{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ReplaceMacroCommand::clone() const
{
    return new ReplaceMacroCommand(margs);
}

/*============================================================================
================================ ReplaceSelMacroCommand ======================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ReplaceSelMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new ReplaceSelMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ReplaceSelMacroCommand::ReplaceSelMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ReplaceSelMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack,
                                        QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("-1"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    QString what;
    QString ntext;
    QString err = getReplaceMndParameters(margs, doc, stack, what, &ntext);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    err = getReplaceOptParameters(margs, doc, stack, 2, cs);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    int n = doc->replaceInSelection(what, ntext, cs);
    return bRet(error, QString(), QString::number(n));
}

QString ReplaceSelMacroCommand::name() const
{
    return "replaceSel";
}

QString ReplaceSelMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\replaceSel{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ReplaceSelMacroCommand::clone() const
{
    return new ReplaceSelMacroCommand(margs);
}

/*============================================================================
================================ ReplaceDocMacroCommand ======================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ReplaceDocMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new ReplaceDocMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ReplaceDocMacroCommand::ReplaceDocMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ReplaceDocMacroCommand::execute(BAbstractCodeEditorDocument *doc, ExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("-1"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    QString what;
    QString ntext;
    QString err = getReplaceMndParameters(margs, doc, stack, what, &ntext);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    err = getReplaceOptParameters(margs, doc, stack, 2, cs);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    int n = doc->replaceInDocument(what, ntext, cs);
    return bRet(error, QString(), QString::number(n));
}

QString ReplaceDocMacroCommand::name() const
{
    return "replaceDoc";
}

QString ReplaceDocMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\replaceDoc{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ReplaceDocMacroCommand::clone() const
{
    return new ReplaceDocMacroCommand(margs);
}
