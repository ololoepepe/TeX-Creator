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

#include "pretexobject.h"

#include "pretexeditormoduleplugin.h"

#include <BAbstractCodeEditorDocument>
#include <BDirTools>
#include <BProperties>
#include <BTextTools>

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QObject>
#include <QPlainTextEdit>
#include <QProcess>
#include <QRegExp>
#include <QScriptContext>
#include <QScriptValue>
#include <QString>
#include <QTextCodec>
#include <QTextDocument>
#include <QVariant>
#include <QVariantMap>

/*============================================================================
================================ RecordingModule =============================
============================================================================*/

/*============================== Public constructors =======================*/

PretexObject::PretexObject(BAbstractCodeEditorDocument *doc) :
    QObject(0)
{
    mdoc = doc;
}

/*============================== Public slots ==============================*/

bool PretexObject::find(const QScriptValue &what, const QVariantMap &options)
{
    if (what.isNull() || what.isUndefined())
        return false;
    QTextDocument::FindFlags flags = 0;
    if (options.value("cs").toBool() || options.value("case-sensitive").toBool())
        flags |= QTextDocument::FindCaseSensitively;
    if (options.value("w").toBool() || options.value("words").toBool())
        flags |= QTextDocument::FindWholeWords;
    if (options.value("bw").toBool() || options.value("backward").toBool())
        flags |= QTextDocument::FindBackward;
    bool cyclic = (!options.value("nc").toBool() && !options.value("nonCyclic").toBool());
    return what.isRegExp() ? mdoc->findNextRegexp(what.toRegExp(), flags, cyclic) :
                             mdoc->findNext(what.toString(), flags, cyclic);
}

QScriptValue PretexObject::getInput(const QString &mode, const QString &label, const QString &title)
{
    QInputDialog idlg(mdoc);
    QInputDialog::InputMode m = QInputDialog::TextInput;
    if (mode.length() <= 4 && QString("text").startsWith(mode, Qt::CaseInsensitive))
        m = QInputDialog::TextInput;
    else if (mode.length() <= 6 && QString("string").startsWith(mode, Qt::CaseInsensitive))
        m = QInputDialog::TextInput;
    else if (mode.length() <= 7 && QString("integer").startsWith(mode, Qt::CaseInsensitive))
        m = QInputDialog::IntInput;
    else if (mode.length() <= 4 && QString("real").startsWith(mode, Qt::CaseInsensitive))
        m = QInputDialog::DoubleInput;
    idlg.setInputMode(m);
    if (!label.isEmpty())
        idlg.setLabelText(label);
    if (!title.isEmpty())
        idlg.setWindowTitle(title);
    idlg.exec();
    switch (m) {
    case QInputDialog::TextInput:
        return idlg.textValue();
    case QInputDialog::IntInput:
        return idlg.intValue();
    case QInputDialog::DoubleInput:
        return idlg.doubleValue();
    default:
        return context()->throwError("Internal error");
    }
}

void PretexObject::insert(const QString &text)
{
    mdoc->insertText(text);
}

void PretexObject::log(const QString &what)
{
    qDebug() << what;
}

void PretexObject::press(const QString &sequence, int count)
{
    QKeySequence ks(sequence);
    if (ks.isEmpty()) {
        context()->throwError("Invalid key sequence");
        return;
    }
    if (count < 1)
        count = 1;
    foreach (int i, bRangeD(0, count - 1)) {
        Q_UNUSED(i)
        for (int j = 0; j < (int) ks.count(); ++j) {
            int key = ~Qt::KeyboardModifierMask & ks[j];
            Qt::KeyboardModifiers modifiers = static_cast<Qt::KeyboardModifiers>(Qt::KeyboardModifierMask & ks[j]);
            QKeyEvent ke(QEvent::KeyPress, key, modifiers);
            QCoreApplication::sendEvent(mdoc->findChild<QPlainTextEdit *>(), &ke);
            QCoreApplication::processEvents();
        }
    }
}

QString PretexObject::readFile(const QString &fileName, const QString &codecName)
{
    if (fileName.isEmpty()) {
        context()->throwError("Invalid file name");
        return "";
    }
    QString fn = fileName;
    if (!QFileInfo(fn).isAbsolute())
        fn.prepend(QFileInfo(mdoc->fileName()).path() + "/");
    bool ok = false;
    QByteArray ba = BDirTools::readFile(fn, -1, &ok);
    if (!ok) {
        context()->throwError("Failed to read file");
        return "";
    }
    QTextCodec *c = !codecName.isEmpty() ? BeQt::codec(codecName) : BTextTools::guessTextCodec(ba);
    if (!c)
        c = BeQt::codec(QString("UTF-8"));
    return c->toUnicode(ba);
}

int PretexObject::replace(const QScriptValue &what, const QString &newText, const QVariantMap &options)
{
    if (what.isNull() || what.isUndefined())
        return 0;
    QTextDocument::FindFlags flags = 0;
    if (options.value("cs").toBool() || options.value("case-sensitive").toBool())
        flags |= QTextDocument::FindCaseSensitively;
    if (options.value("w").toBool() || options.value("words").toBool())
        flags |= QTextDocument::FindWholeWords;
    bool selectionScope = (options.value("s").toBool() || options.value("sel").toBool()
                           || options.value("selection").toBool());
    if (selectionScope) {
        return what.isRegExp() ? mdoc->replaceInSelectionRegexp(what.toRegExp(), newText)
                               : mdoc->replaceInSelection(what.toString(), newText, flags);
    } else {
        return what.isRegExp() ? mdoc->replaceInDocumentRegexp(what.toRegExp(), newText)
                               : mdoc->replaceInDocument(what.toString(), newText, flags);
    }
}

QString PretexObject::run(const QString &command, const QVariantMap &options)
{
    if (command.isEmpty()) {
        context()->throwError("Invalid command");
        return "";
    }
    QString cmd = PretexEditorModulePlugin::externalTools().value(cmd);
    if (cmd.isEmpty())
        cmd = command;
    QString dir = options.value("workingDirectory").toString();
    if (dir.isEmpty())
        dir = options.value("workingDir").toString();
    if (dir.isEmpty())
        dir = options.value("dir").toString();
    if (dir.isEmpty())
        dir = options.value("wd").toString();
    if (dir.isEmpty())
        dir = QFileInfo(mdoc->fileName()).path();
    QProcess proc;
    proc.setWorkingDirectory(dir);
    QString out;
    QStringList arguments = options.value("arguments").toStringList();
    if (arguments.isEmpty())
        arguments = options.value("args").toStringList();
    BeQt::execProcess(dir, cmd, arguments, 5 * BeQt::Second, 5 * BeQt::Minute, &out);
    return out;
}

bool PretexObject::runDetached(const QString &command, const QVariantMap &options)
{
    if (command.isEmpty()) {
        context()->throwError("Invalid command");
        return false;
    }
    QString cmd = PretexEditorModulePlugin::externalTools().value(cmd);
    if (cmd.isEmpty())
        cmd = command;
    QString dir = options.value("workingDirectory").toString();
    if (dir.isEmpty())
        dir = options.value("workingDir").toString();
    if (dir.isEmpty())
        dir = options.value("dir").toString();
    if (dir.isEmpty())
        dir = options.value("wd").toString();
    if (dir.isEmpty())
        dir = QFileInfo(mdoc->fileName()).path();
    QStringList arguments = options.value("arguments").toStringList();
    if (arguments.isEmpty())
        arguments = options.value("args").toStringList();
    return BeQt::startProcessDetached(cmd, dir, arguments);
}

void PretexObject::showMessage(const QString &text, const QString &type, const QString &title)
{
    QMessageBox msgbox(mdoc);
    msgbox.setText(text);
    QMessageBox::Icon icon = QMessageBox::Information;
    if (type.length() <= 6 && QString("noicon").startsWith(type, Qt::CaseInsensitive))
        icon = QMessageBox::NoIcon;
    else if (type.length() <= 8 && QString("question").startsWith(type, Qt::CaseInsensitive))
        icon = QMessageBox::Question;
    else if (type.length() <= 11 && QString("information").startsWith(type, Qt::CaseInsensitive))
        icon = QMessageBox::Information;
    else if (type.length() <= 7 && QString("warning").startsWith(type, Qt::CaseInsensitive))
        icon = QMessageBox::Warning;
    else if (type.length() <= 8 && QString("critical").startsWith(type, Qt::CaseInsensitive))
        icon = QMessageBox::Critical;
    msgbox.setIcon(icon);
    if (!title.isNull())
        msgbox.setWindowTitle(title);
    msgbox.exec();
}
