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

#include <BeQtGlobal>
#include <BDirTools>
#include <BAbstractCodeEditorDocument>

#include <QList>
#include <QString>
#include <QMessageBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QEvent>
#include <QKeySequence>
#include <QCoreApplication>
#include <QPlainTextEdit>

#include <QDebug>

B_DECLARE_TRANSLATE_FUNCTION

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool insert(ExecutionStack *stack, QString *err)
{
    QString text = stack->obligArg().toString();
    stack->doc()->insertText(text);
    stack->setReturnValue(text);
    return bRet(err, QString(), true);
}

static bool find(ExecutionStack *stack, QString *err)
{
    //
}

static bool replace(ExecutionStack *stack, QString *err)
{
    //
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
    //msgbox.setWindowTitle(stack->optArg());
    //icon
    msgbox.exec();
    stack->setReturnValue(1);
    return bRet(err, QString(), true);
}

static bool getInput(ExecutionStack *stack, QString *err)
{
    //
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
    if (codec.isEmpty())
        codec = "UTF-8";
    bool ok = false;
    QString s = BDirTools::readTextFile(fn, codec, &ok);
    if (!ok)
        return bRet(err, translate("readFile", "Failed to read file", "error"), false);
    stack->setReturnValue(s);
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
    default:
        break;
    }
    return 0;
}

bool IOFunction::execute(ExecutionStack *stack, QString *err)
{
    //Argument count is checked in PretexBuiltinFunction
    switch (mtype)
    {
    case InsertType:
        return insert(stack, err);
    case FindType:
    case ReplaceType:
    case PressType:
        return press(stack, err);
    case ShowMessageType:
        return showMessage(stack, err);
    case GetInputType:
        break;
    case ReadFileType:
        return readFile(stack, err);
    default:
        break;
    }
    return bRet(err, tr("Internal error: failed to find builtin function", "error"), false);
}
