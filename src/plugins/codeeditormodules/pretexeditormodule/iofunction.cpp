/****************************************************************************
**
** Copyright (C) 2014 TeXSample Team
**
** This file is part of the PreTeX Editor Module plugin of TeX Creator.
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

#include "iofunction.h"
#include "pretexbuiltinfunction.h"
#include "pretexvariant.h"
#include "executionstack.h"
#include "global.h"
#include "pretexeditormoduleplugin.h"

#include <BeQtGlobal>
#include <BDirTools>
#include <BAbstractCodeEditorDocument>
#include <BeQt>
#include <BTerminalIOHandler>
#include <BTextTools>

#include <QList>
#include <QString>
#include <QMessageBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QEvent>
#include <QKeySequence>
#include <QCoreApplication>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QStringList>
#include <QFileInfo>
#include <QMap>
#include <QProcess>
#include <QTextCodec>

#include <QDebug>

B_DECLARE_TRANSLATE_FUNCTION

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool searchOptions(const PretexVariant &v, QTextDocument::FindFlags *flags, bool *cyclic, QString *err = 0)
{
    if (v.type() != PretexVariant::String)
        return bRet(err, translate("searchOptions", "Expected a string", "error"), false);
    QString s = v.toString();
    QTextDocument::FindFlags f = 0;
    if (!s.isEmpty())
    {
        bool csb = false;
        bool wb = false;
        bool bwb = false;
        bool ncb = false;
        foreach (const QString &ss, s.split('+'))
        {
            if (!QString::compare(ss, "cs") || !QString::compare(ss, "case-sensitive"))
            {
                if (csb)
                    return bRet(err, translate("searchOptions", "Repeated option", "error"), false);
                csb = true;
                f |= QTextDocument::FindCaseSensitively;
            }
            else if (!QString::compare(ss, "ci") || !QString::compare(ss, "case-insensitive"))
            {
                if (csb)
                    return bRet(err, translate("searchOptions", "Repeated option", "error"), false);
                csb = true;
            }
            else if (!QString::compare(ss, "w") || !QString::compare(ss, "words"))
            {
                if (wb)
                    return bRet(err, translate("searchOptions", "Repeated option", "error"), false);
                wb = true;
                f |= QTextDocument::FindWholeWords;
            }
            else if (!QString::compare(ss, "bw") || !QString::compare(ss, "backward"))
            {
                if (bwb)
                    return bRet(err, translate("searchOptions", "Repeated option", "error"), false);
                bwb = true;
                f |= QTextDocument::FindBackward;
            }
            else if (!QString::compare(ss, "fw") || !QString::compare(ss, "forward"))
            {
                if (bwb)
                    return bRet(err, translate("searchOptions", "Repeated option", "error"), false);
                bwb = true;
            }
            else if (!QString::compare(ss, "c") || !QString::compare(ss, "cyclic"))
            {
                if (ncb)
                    return bRet(err, translate("searchOptions", "Repeated option", "error"), false);
                ncb = true;
                bSet(cyclic, true);
            }
            else if (!QString::compare(ss, "nc") || !QString::compare(ss, "non-cyclic"))
            {
                if (ncb)
                    return bRet(err, translate("searchOptions", "Repeated option", "error"), false);
                ncb = true;
                bSet(cyclic, false);
            }
        }
    }
    else
    {
        bSet(cyclic, true);
    }
    return bRet(flags, f, err, QString(), true);
}

static bool replaceOptions(const PretexVariant &v, Qt::CaseSensitivity *cs, QString *err = 0)
{
    if (v.type() != PretexVariant::String)
        return bRet(err, translate("replaceOptions", "Expected a string", "error"), false);
    QString s = v.toString();
    if (!s.isEmpty())
    {
        if (!QString::compare(s, "cs") || !QString::compare(s, "case-sensitive"))
            bSet(cs, Qt::CaseSensitive);
        else if (!QString::compare(s, "ci") || !QString::compare(s, "case-insensitive"))
            bSet(cs, Qt::CaseInsensitive);
        else
            return bRet(err, translate("replaceOptions", "Invalid argument", "error"), false);
    }
    else
    {
        bSet(cs, Qt::CaseInsensitive);
    }
    return bRet(err, QString(), true);
}

static bool replaceScope(const PretexVariant &v, bool *selection, QString *err = 0)
{
    if (v.type() != PretexVariant::String)
        return bRet(err, translate("replaceScope", "Expected a string", "error"), false);
    QString s = v.toString();
    if (!s.isEmpty())
    {
        if (!QString::compare(s, "s") || !QString::compare(s, "sel") || !QString::compare(s, "selection"))
            bSet(selection, true);
        else if (!QString::compare(s, "d") || !QString::compare(s, "doc") || !QString::compare(s, "document"))
            bSet(selection, false);
        else
            return bRet(err, translate("replaceScope", "Invalid argument", "error"), false);
    }
    else
    {
        bSet(selection, false);
    }
    return bRet(err, QString(), true);
}

static bool icon(const PretexVariant &v, QMessageBox::Icon *icon, QString *err = 0)
{
    if (v.type() != PretexVariant::String)
        return bRet(err, translate("icon", "Expected a string", "error"), false);
    QString s = v.toString();
    if (!s.isEmpty())
    {
        if (s.length() <= 6 && QString("noicon").startsWith(s, Qt::CaseInsensitive))
            bSet(icon, QMessageBox::NoIcon);
        else if (s.length() <= 8 && QString("question").startsWith(s, Qt::CaseInsensitive))
            bSet(icon, QMessageBox::Question);
        else if (s.length() <= 11 && QString("information").startsWith(s, Qt::CaseInsensitive))
            bSet(icon, QMessageBox::Information);
        else if (s.length() <= 7 && QString("warning").startsWith(s, Qt::CaseInsensitive))
            bSet(icon, QMessageBox::Warning);
        else if (s.length() <= 8 && QString("critical").startsWith(s, Qt::CaseInsensitive))
            bSet(icon, QMessageBox::Critical);
        else
            return bRet(err, translate("icon", "Invalid argument", "error"), false);
    }
    else
    {
        bSet(icon, QMessageBox::Information);
    }
    return bRet(err, QString(), true);
}

static bool inputMode(const PretexVariant &v, QInputDialog::InputMode *mode, QString *err = 0)
{
    if (v.isNull())
        return bRet(mode, QInputDialog::TextInput, err, QString(), true);
    if (v.type() != PretexVariant::String)
        return bRet(err, translate("inputMode", "Expected a string", "error"), false);
    QString s = v.toString();
    if (!s.isEmpty())
    {
        if (s.length() <= 4 && QString("text").startsWith(s, Qt::CaseInsensitive))
            bSet(mode, QInputDialog::TextInput);
        else if (s.length() <= 6 && QString("string").startsWith(s, Qt::CaseInsensitive))
            bSet(mode, QInputDialog::TextInput);
        else if (s.length() <= 7 && QString("integer").startsWith(s, Qt::CaseInsensitive))
            bSet(mode, QInputDialog::IntInput);
        else if (s.length() <= 4 && QString("real").startsWith(s, Qt::CaseInsensitive))
            bSet(mode, QInputDialog::DoubleInput);
        else
            return bRet(err, translate("inputMode", "Invalid argument", "error"), false);
    }
    else
    {
        bSet(mode, QInputDialog::TextInput);
    }
    return bRet(err, QString(), true);
}

static bool insert(ExecutionStack *stack, QString *err)
{
    QString text = !stack->obligArg().isNull() ? stack->obligArg().toString() : QString();
    stack->doc()->insertText(text);
    stack->setReturnValue(text);
    return bRet(err, QString(), true);
}

static bool find(ExecutionStack *stack, QString *err)
{
    QString what = stack->obligArg().toString();
    if (what.isEmpty())
    {
        stack->setReturnValue(0);
        return bRet(err, QString(), true);
    }
    QTextDocument::FindFlags flags = 0;
    bool cyclic = true;
    if (!stack->optArg().isNull() && !searchOptions(stack->optArg(), &flags, &cyclic, err))
        return false;
    bool b = stack->doc()->findNext(what, flags, cyclic);
    stack->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

static bool replace(ExecutionStack *stack, QString *err)
{
    QString what = stack->obligArg().toString();
    if (what.isEmpty())
    {
        stack->setReturnValue(0);
        return bRet(err, QString(), true);
    }
    QString newText = stack->obligArg(1).toString();
    bool selection = false;
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    if (!stack->optArg(0).isNull() && !replaceOptions(stack->optArg(0), &cs, err))
        return false;
    if (!stack->optArg(1).isNull() && !replaceScope(stack->optArg(1), &selection, err))
        return false;
    stack->setReturnValue(selection ? stack->doc()->replaceInSelection(what, newText, cs) :
                                      stack->doc()->replaceInDocument(what, newText, cs));
    return bRet(err, QString(), true);
}

static bool press(ExecutionStack *stack, QString *err)
{
    QKeySequence ks(stack->obligArg().toString());
    if (ks.isEmpty())
        return bRet(err, QString("Invalid key sequence"), false);
    int n = 1;
    if (!stack->optArg().isNull())
    {
        if (stack->optArg().type() != PretexVariant::Int)
            return bRet(err, QString("Repetition count must be an integer"), false);
        n = stack->optArg().toInt();
        if (n <= 0)
            return bRet(err, QString("Invalid repetition count"), false);
    }
    foreach (int i, bRangeD(0, n - 1))
    {
        Q_UNUSED(i)
        for (int j = 0; j < (int) ks.count(); ++j)
        {
            int key = ~Qt::KeyboardModifierMask & ks[j];
            Qt::KeyboardModifiers modifiers = static_cast<Qt::KeyboardModifiers>(Qt::KeyboardModifierMask & ks[j]);
            QKeyEvent ke(QEvent::KeyPress, key, modifiers);
            QCoreApplication::sendEvent(stack->doc()->findChild<QPlainTextEdit *>(), &ke);
            QCoreApplication::processEvents();
        }
    }
    stack->setReturnValue(1);
    return bRet(err, QString(), true);
}

static bool showMessage(ExecutionStack *stack, QString *err)
{
    QMessageBox msgbox(stack->doc());
    msgbox.setText(stack->obligArg().toString());
    QMessageBox::Icon icn = QMessageBox::Information;
    if (!stack->optArg(0).isNull() && !icon(stack->optArg(0), &icn, err))
        return false;
    msgbox.setIcon(icn);
    if (!stack->optArg(1).isNull())
        msgbox.setWindowTitle(stack->optArg(1).toString());
    msgbox.exec();
    stack->setReturnValue(1);
    return bRet(err, QString(), true);
}

static bool getInput(ExecutionStack *stack, QString *err)
{
    QInputDialog idlg(stack->doc());
    QInputDialog::InputMode mode = QInputDialog::TextInput;
    if (!inputMode(stack->obligArg(), &mode, err))
        return false;
    idlg.setInputMode(mode);
    if (!stack->optArg(0).isNull())
        idlg.setLabelText(stack->optArg(0).toString());
    if (!stack->optArg(1).isNull())
        idlg.setWindowTitle(stack->optArg(1).toString());
    idlg.exec();
    switch (mode)
    {
    case QInputDialog::TextInput:
        stack->setReturnValue(idlg.textValue());
        break;
    case QInputDialog::IntInput:
        stack->setReturnValue(idlg.intValue());
        break;
    case QInputDialog::DoubleInput:
        stack->setReturnValue(idlg.doubleValue());
        break;
    default:
        return bRet(err, translate("getInput", "Null argument(s)", "error"), false);
    }
    return bRet(err, QString(), true);
}

static bool readFile(ExecutionStack *stack, QString *err)
{
    if (stack->obligArg().type() != PretexVariant::String)
        return bRet(err, translate("readFile", "File name must be a string", "error"), false);
    QString fn = stack->obligArg().toString();
    if (fn.isEmpty())
        return bRet(err, translate("readFile", "File name can not be empty", "error"), false);
    QString codec;
    if (!stack->optArg().isNull())
    {
        if (stack->optArg().type() != PretexVariant::String)
            return bRet(err, translate("readFile", "Encoding name must be a string", "error"), false);
        codec = stack->optArg().toString();
    }
    if (!QFileInfo(fn).isAbsolute())
        fn.prepend(QFileInfo(stack->doc()->fileName()).path() + "/");
    bool ok = false;
    QByteArray ba = BDirTools::readFile(fn, -1, &ok);
    if (!ok)
        return bRet(err, translate("readFile", "Failed to read file", "error"), false);
    QTextCodec *c = !codec.isEmpty() ? BeQt::codec(codec) : BTextTools::guessTextCodec(ba);
    if (!c)
        c = BeQt::codec(QString("UTF-8"));
    stack->setReturnValue(c->toUnicode(ba));
    return bRet(err, QString(), true);
}

static bool run(ExecutionStack *stack, bool detached, QString *err)
{
    QString cmd = stack->obligArg().toString();
    QString cmd2;
    if (!cmd.isEmpty())
        cmd2 = PretexEditorModulePlugin::externalTools().value(cmd);
    if (!cmd2.isEmpty())
        cmd = cmd2;
    if (cmd.isEmpty())
        return bRet(err, translate("run", "Invali command", "error"), false);
    QStringList args;
    foreach (int i, bRangeD(0, stack->optArgCount() - 1))
        args << stack->optArg(i).toString();
    QString dir = QFileInfo(stack->doc()->fileName()).path();
    if (detached)
    {
        if (!args.isEmpty())
            cmd += " " + BTerminalIOHandler::mergeArguments(args);
        bool b = QProcess::startDetached(cmd, QStringList(), dir);
        stack->setReturnValue(b ? 1 : 0);
    }
    else
    {
        QProcess proc;
        proc.setWorkingDirectory(dir);
        BeQt::startProcess(&proc, cmd, args);
        QString out;
        BeQt::execProcess(dir, cmd, args, 5 * BeQt::Second, 5 * BeQt::Minute, &out);
        stack->setReturnValue(out);
    }
    return bRet(err, QString(), true);
}

/*============================================================================
================================ IOFunction ==================================
============================================================================*/

/*============================== Public constructors =======================*/

IOFunction::IOFunction(Type t)
{
    mtype = t;
}

/*============================== Public methods ============================*/

QString IOFunction::name() const
{
    switch (mtype)
    {
    case InsertType:
        return "insert";
    case FindType:
        return "find";
    case ReplaceType:
        return "replace";
    case PressType:
        return "press";
    case ShowMessageType:
        return "showMessage";
    case GetInputType:
        return "getInput";
    case ReadFileType:
        return "readFile";
    case RunType:
        return "run";
    case RunDetachedType:
        return "runDetached";
    default:
        break;
    }
    return QString();
}

int IOFunction::obligatoryArgumentCount() const
{
    switch (mtype)
    {
    case InsertType:
    case FindType:
    case PressType:
    case ShowMessageType:
    case GetInputType:
    case ReadFileType:
    case RunType:
    case RunDetachedType:
        return 1;
    case ReplaceType:
        return 2;
    default:
        break;
    }
    return 0;
}

int IOFunction::optionalArgumentCount() const
{
    switch (mtype)
    {
    case InsertType:
        return 0;
    case FindType:
    case PressType:
    case ReadFileType:
        return 1;
    case ReplaceType:
    case ShowMessageType:
    case GetInputType:
        return 2;
    case RunType:
    case RunDetachedType:
        return -1;
    default:
        break;
    }
    return 0;
}

/*============================== Protected methods =========================*/

bool IOFunction::execute(ExecutionStack *stack, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case InsertType:
        return insert(stack, err);
    case FindType:
        return find(stack, err);
    case ReplaceType:
        return replace(stack, err);
    case PressType:
        return press(stack, err);
    case ShowMessageType:
        return showMessage(stack, err);
    case GetInputType:
        return getInput(stack, err);
    case ReadFileType:
        return readFile(stack, err);
    case RunType:
        return run(stack, false, err);
    case RunDetachedType:
        return run(stack, true, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
