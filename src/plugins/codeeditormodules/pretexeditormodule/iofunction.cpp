/****************************************************************************
**
** Copyright (C) 2014 Andrey Bogdanov
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

#include "executioncontext.h"
#include "global.h"
#include "pretexbuiltinfunction.h"
#include "pretexeditormoduleplugin.h"
#include "pretexvariant.h"

#include <BAbstractCodeEditorDocument>
#include <BDirTools>
#include <BeQt>
#include <BProperties>
#include <BTextTools>

#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QTextDocument>

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
    switch (mtype) {
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
    switch (mtype) {
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
    switch (mtype) {
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

bool IOFunction::execute(ExecutionContext *context, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype) {
    case InsertType:
        return insert(context, err);
    case FindType:
        return find(context, err);
    case ReplaceType:
        return replace(context, err);
    case PressType:
        return press(context, err);
    case ShowMessageType:
        return showMessage(context, err);
    case GetInputType:
        return getInput(context, err);
    case ReadFileType:
        return readFile(context, err);
    case RunType:
        return run(context, false, err);
    case RunDetachedType:
        return run(context, true, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}

/*============================== Static private methods ====================*/

bool IOFunction::find(ExecutionContext *context, QString *err)
{
    QString what = context->obligArg().toString();
    if (what.isEmpty()) {
        context->setReturnValue(0);
        return bRet(err, QString(), true);
    }
    QTextDocument::FindFlags flags = 0;
    bool cyclic = true;
    bool regexp = false;
    if (!context->optArg().isNull() && !searchOptions(context->optArg(), &flags, &cyclic, &regexp, err))
        return false;
    Qt::CaseSensitivity cs = (QTextDocument::FindCaseSensitively & flags) ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool b = regexp ? context->doc()->findNextRegexp(QRegExp(what, cs), flags, cyclic) :
                      context->doc()->findNext(what, flags, cyclic);
    context->setReturnValue(b ? 1 : 0);
    return bRet(err, QString(), true);
}

bool IOFunction::getInput(ExecutionContext *context, QString *err)
{
    QInputDialog idlg(context->doc());
    QInputDialog::InputMode mode = QInputDialog::TextInput;
    if (!inputMode(context->obligArg(), &mode, err))
        return false;
    idlg.setInputMode(mode);
    if (!context->optArg(0).isNull())
        idlg.setLabelText(context->optArg(0).toString());
    if (!context->optArg(1).isNull())
        idlg.setWindowTitle(context->optArg(1).toString());
    idlg.exec();
    switch (mode) {
    case QInputDialog::TextInput:
        context->setReturnValue(idlg.textValue());
        break;
    case QInputDialog::IntInput:
        context->setReturnValue(idlg.intValue());
        break;
    case QInputDialog::DoubleInput:
        context->setReturnValue(idlg.doubleValue());
        break;
    default:
        return bRet(err, tr("Null argument(s)", "error"), false);
    }
    return bRet(err, QString(), true);
}

bool IOFunction::icon(const PretexVariant &v, QMessageBox::Icon *icon, QString *err)
{
    if (v.type() != PretexVariant::String)
        return bRet(err, tr("Expected a string", "error"), false);
    QString s = v.toString();
    if (!s.isEmpty()) {
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
            return bRet(err, tr("Invalid argument", "error"), false);
    } else {
        bSet(icon, QMessageBox::Information);
    }
    return bRet(err, QString(), true);
}

bool IOFunction::inputMode(const PretexVariant &v, QInputDialog::InputMode *mode, QString *err)
{
    if (v.isNull())
        return bRet(mode, QInputDialog::TextInput, err, QString(), true);
    if (v.type() != PretexVariant::String)
        return bRet(err, tr("Expected a string", "error"), false);
    QString s = v.toString();
    if (!s.isEmpty()) {
        if (s.length() <= 4 && QString("text").startsWith(s, Qt::CaseInsensitive))
            bSet(mode, QInputDialog::TextInput);
        else if (s.length() <= 6 && QString("string").startsWith(s, Qt::CaseInsensitive))
            bSet(mode, QInputDialog::TextInput);
        else if (s.length() <= 7 && QString("integer").startsWith(s, Qt::CaseInsensitive))
            bSet(mode, QInputDialog::IntInput);
        else if (s.length() <= 4 && QString("real").startsWith(s, Qt::CaseInsensitive))
            bSet(mode, QInputDialog::DoubleInput);
        else
            return bRet(err, tr("Invalid argument", "error"), false);
    } else {
        bSet(mode, QInputDialog::TextInput);
    }
    return bRet(err, QString(), true);
}

bool IOFunction::insert(ExecutionContext *context, QString *err)
{
    QString text = !context->obligArg().isNull() ? context->obligArg().toString() : QString();
    context->doc()->insertText(text);
    context->setReturnValue(text);
    return bRet(err, QString(), true);
}

bool IOFunction::press(ExecutionContext *context, QString *err)
{
    QKeySequence ks(context->obligArg().toString());
    if (ks.isEmpty())
        return bRet(err, tr("Invalid key sequence", "error"), false);
    int n = 1;
    if (!context->optArg().isNull()) {
        if (context->optArg().type() != PretexVariant::Int)
            return bRet(err, tr("Repetition count must be an integer", "error"), false);
        n = context->optArg().toInt();
        if (n <= 0)
            return bRet(err, tr("Invalid repetition count", "error"), false);
    }
    foreach (int i, bRangeD(0, n - 1)) {
        Q_UNUSED(i)
        for (int j = 0; j < (int) ks.count(); ++j) {
            int key = ~Qt::KeyboardModifierMask & ks[j];
            Qt::KeyboardModifiers modifiers = static_cast<Qt::KeyboardModifiers>(Qt::KeyboardModifierMask & ks[j]);
            QKeyEvent ke(QEvent::KeyPress, key, modifiers);
            QCoreApplication::sendEvent(context->doc()->findChild<QPlainTextEdit *>(), &ke);
            QCoreApplication::processEvents();
        }
    }
    context->setReturnValue(1);
    return bRet(err, QString(), true);
}

bool IOFunction::readFile(ExecutionContext *context, QString *err)
{
    if (context->obligArg().type() != PretexVariant::String)
        return bRet(err, tr("File name must be a string", "error"), false);
    QString fn = context->obligArg().toString();
    if (fn.isEmpty())
        return bRet(err, tr("File name can not be empty", "error"), false);
    QString codec;
    if (!context->optArg().isNull()) {
        if (context->optArg().type() != PretexVariant::String)
            return bRet(err, tr("Encoding name must be a string", "error"), false);
        codec = context->optArg().toString();
    }
    if (!QFileInfo(fn).isAbsolute())
        fn.prepend(QFileInfo(context->doc()->fileName()).path() + "/");
    bool ok = false;
    QByteArray ba = BDirTools::readFile(fn, -1, &ok);
    if (!ok)
        return bRet(err, tr("Failed to read file", "error"), false);
    QTextCodec *c = !codec.isEmpty() ? BeQt::codec(codec) : BTextTools::guessTextCodec(ba);
    if (!c)
        c = BeQt::codec(QString("UTF-8"));
    context->setReturnValue(c->toUnicode(ba));
    return bRet(err, QString(), true);
}

bool IOFunction::replace(ExecutionContext *context, QString *err)
{
    QString what = context->obligArg().toString();
    if (what.isEmpty()) {
        context->setReturnValue(0);
        return bRet(err, QString(), true);
    }
    QString newText = context->obligArg(1).toString();
    bool selection = false;
    QTextDocument::FindFlags flags = 0;
    bool regexp = false;
    if (!context->optArg(0).isNull() && !replaceOptions(context->optArg(0), &flags, &regexp, err))
        return false;
    Qt::CaseSensitivity cs = (QTextDocument::FindCaseSensitively & flags) ? Qt::CaseSensitive : Qt::CaseInsensitive;
    if (!context->optArg(1).isNull() && !replaceScope(context->optArg(1), &selection, err))
        return false;
    if (selection) {
        context->setReturnValue(regexp ? context->doc()->replaceInSelectionRegexp(QRegExp(what, cs), newText) :
                                         context->doc()->replaceInSelection(what, newText, flags));
    } else {
        context->setReturnValue(regexp ? context->doc()->replaceInDocumentRegexp(QRegExp(what, cs), newText) :
                                         context->doc()->replaceInDocument(what, newText, flags));
    }
    return bRet(err, QString(), true);
}

bool IOFunction::replaceOptions(const PretexVariant &v, QTextDocument::FindFlags *flags, bool *regexp, QString *err)
{
    if (v.type() != PretexVariant::String)
        return bRet(err, tr("Expected a string", "error"), false);
    QString s = v.toString();
    QTextDocument::FindFlags f = 0;
    if (!s.isEmpty()) {
        bool csb = false;
        bool wb = false;
        bool rxb = false;
        foreach (const QString &ss, s.split('+')) {
            if (!QString::compare(ss, "cs") || !QString::compare(ss, "case-sensitive")) {
                if (csb)
                    return bRet(err, tr("Repeated option", "error"), false);
                csb = true;
                f |= QTextDocument::FindCaseSensitively;
            } else if (!QString::compare(ss, "ci") || !QString::compare(ss, "case-insensitive")) {
                if (csb)
                    return bRet(err, tr("Repeated option", "error"), false);
                csb = true;
            } else if (!QString::compare(ss, "w") || !QString::compare(ss, "words")) {
                if (wb)
                    return bRet(err, tr("Repeated option", "error"), false);
                wb = true;
                f |= QTextDocument::FindWholeWords;
            } else if (!QString::compare(ss, "r") || !QString::compare(ss, "rx") || !QString::compare(ss, "regexp")) {
                if (rxb)
                    return bRet(err, tr("Repeated option", "error"), false);
                rxb = true;
                bSet(regexp, true);
            }
        }
    }
    return bRet(flags, f, err, QString(), true);
}

bool IOFunction::replaceScope(const PretexVariant &v, bool *selection, QString *err)
{
    if (v.type() != PretexVariant::String)
        return bRet(err, tr("Expected a string", "error"), false);
    QString s = v.toString();
    if (!s.isEmpty()) {
        if (!QString::compare(s, "s") || !QString::compare(s, "sel") || !QString::compare(s, "selection"))
            bSet(selection, true);
        else if (!QString::compare(s, "d") || !QString::compare(s, "doc") || !QString::compare(s, "document"))
            bSet(selection, false);
        else
            return bRet(err, tr("Invalid argument", "error"), false);
    } else {
        bSet(selection, false);
    }
    return bRet(err, QString(), true);
}

bool IOFunction::run(ExecutionContext *context, bool detached, QString *err)
{
    QString cmd = context->obligArg().toString();
    QString cmd2;
    if (!cmd.isEmpty())
        cmd2 = PretexEditorModulePlugin::externalTools().value(cmd);
    if (!cmd2.isEmpty())
        cmd = cmd2;
    if (cmd.isEmpty())
        return bRet(err, tr("Invalid command", "error"), false);
    QStringList args;
    foreach (int i, bRangeD(0, context->optArgCount() - 1))
        args << context->optArg(i).toString();
    QString dir = QFileInfo(context->doc()->fileName()).path();
    if (detached) {
        context->setReturnValue(BeQt::startProcessDetached(cmd, dir, args) ? 1 : 0);
    } else {
        QProcess proc;
        proc.setWorkingDirectory(dir);
        QString out;
        BeQt::execProcess(dir, cmd, args, 5 * BeQt::Second, 5 * BeQt::Minute, &out);
        context->setReturnValue(out);
    }
    return bRet(err, QString(), true);
}

bool IOFunction::searchOptions(const PretexVariant &v, QTextDocument::FindFlags *flags, bool *cyclic, bool *regexp,
                               QString *err)
{
    if (v.type() != PretexVariant::String)
        return bRet(err, tr("Expected a string", "error"), false);
    QString s = v.toString();
    QTextDocument::FindFlags f = 0;
    if (!s.isEmpty()) {
        bool csb = false;
        bool wb = false;
        bool bwb = false;
        bool ncb = false;
        bool rxb = false;
        foreach (const QString &ss, s.split('+')) {
            if (!QString::compare(ss, "cs") || !QString::compare(ss, "case-sensitive")) {
                if (csb)
                    return bRet(err, tr("Repeated option", "error"), false);
                csb = true;
                f |= QTextDocument::FindCaseSensitively;
            } else if (!QString::compare(ss, "ci") || !QString::compare(ss, "case-insensitive")) {
                if (csb)
                    return bRet(err, tr("Repeated option", "error"), false);
                csb = true;
            } else if (!QString::compare(ss, "w") || !QString::compare(ss, "words")) {
                if (wb)
                    return bRet(err, tr("Repeated option", "error"), false);
                wb = true;
                f |= QTextDocument::FindWholeWords;
            } else if (!QString::compare(ss, "bw") || !QString::compare(ss, "backward")) {
                if (bwb)
                    return bRet(err, tr("Repeated option", "error"), false);
                bwb = true;
                f |= QTextDocument::FindBackward;
            } else if (!QString::compare(ss, "fw") || !QString::compare(ss, "forward")) {
                if (bwb)
                    return bRet(err, tr("Repeated option", "error"), false);
                bwb = true;
            } else if (!QString::compare(ss, "c") || !QString::compare(ss, "cyclic")) {
                if (ncb)
                    return bRet(err, tr("Repeated option", "error"), false);
                ncb = true;
                bSet(cyclic, true);
            } else if (!QString::compare(ss, "nc") || !QString::compare(ss, "non-cyclic")) {
                if (ncb)
                    return bRet(err, tr("Repeated option", "error"), false);
                ncb = true;
                bSet(cyclic, false);
            } else if (!QString::compare(ss, "r") || !QString::compare(ss, "rx") || !QString::compare(ss, "regexp")) {
                if (rxb)
                    return bRet(err, tr("Repeated option", "error"), false);
                rxb = true;
                bSet(regexp, true);
            }
        }
    }
    return bRet(flags, f, err, QString(), true);
}

bool IOFunction::showMessage(ExecutionContext *context, QString *err)
{
    QMessageBox msgbox(context->doc());
    msgbox.setText(context->obligArg().toString());
    QMessageBox::Icon icn = QMessageBox::Information;
    if (!context->optArg(0).isNull() && !icon(context->optArg(0), &icn, err))
        return false;
    msgbox.setIcon(icn);
    if (!context->optArg(1).isNull())
        msgbox.setWindowTitle(context->optArg(1).toString());
    msgbox.exec();
    context->setReturnValue(1);
    return bRet(err, QString(), true);
}
