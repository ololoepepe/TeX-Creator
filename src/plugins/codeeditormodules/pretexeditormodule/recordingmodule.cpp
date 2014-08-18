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

#include "recordingmodule.h"

#include <BAbstractCodeEditorDocument>

#include <QObject>
#include <QStringList>
#include <QEvent>
#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QString>
#include <QKeySequence>

/*============================================================================
================================ RecordingModule =============================
============================================================================*/

/*============================== Public constructors =======================*/

RecordingModule::RecordingModule(QObject *parent) :
    QObject(parent)
{
    mrecording = false;
    mdoc = 0;
}

RecordingModule::RecordingModule(BAbstractCodeEditorDocument *doc, QObject *parent) :
    QObject(parent)
{
    mrecording = false;
    mdoc = doc;
}

/*============================== Public methods ============================*/

bool RecordingModule::eventFilter(QObject *, QEvent *e)
{
    if (!e || e->type() != QEvent::KeyPress)
        return false;
    bool b = false;
    QString cmd = commandFromKeyPress(static_cast<QKeyEvent *>(e), &b);
    if (!b)
        return false;
    if (cmd.startsWith("\\insert"))
    {
        if (mcommands.isEmpty() || !mcommands.last().startsWith("\\insert"))
            mcommands << cmd;
        else
            mcommands.last().insert(mcommands.last().length() - 2, cmd.mid(9, cmd.length() - 11));
    }
    else
    {
        mcommands << cmd;
    }
    return false;
}

void RecordingModule::setDocument(BAbstractCodeEditorDocument *doc)
{
    if (mrecording)
        return;
    mdoc = doc;
}

void RecordingModule::startRecording()
{
    if (mrecording || !mdoc)
        return;
   mrecording = true;
   mcommands.clear();
   mdoc->findChild<QPlainTextEdit *>()->installEventFilter(this);
}

void RecordingModule::stopRecording()
{
    if (!mrecording)
        return;
    mdoc->findChild<QPlainTextEdit *>()->removeEventFilter(this);
    mrecording = false;
}

BAbstractCodeEditorDocument *RecordingModule::document() const
{
    return mdoc;
}

bool RecordingModule::isValid() const
{
    return mdoc;
}

bool RecordingModule::isRecording() const
{
    return mrecording;
}

QStringList RecordingModule::commands() const
{
    return mcommands;
}

/*============================== Static private methods ====================*/

QString RecordingModule::commandFromKeyPress(QKeyEvent *e, bool *ok)
{
    int key = e->key();
    Qt::KeyboardModifiers modifiers = e->modifiers();
    QString text = e->text();
    if (key <= 0)
        return bRet(ok, false, QString());
    if (Qt::Key_Control == key || Qt::Key_Alt == key || Qt::Key_Shift == key)
        return bRet(ok, false, QString());
    if ((modifiers & Qt::ControlModifier) && (modifiers & Qt::ShiftModifier) && (modifiers & Qt::AltModifier)
            && (modifiers & Qt::MetaModifier))
        return bRet(ok, false, QString());
    if (!(modifiers & Qt::ControlModifier) && !(modifiers & Qt::AltModifier) && Qt::Key_Return == key)
        text = "\n";
    if (text.isEmpty() || (!text.at(0).isPrint() && text.at(0) != '\n')
            || (modifiers & Qt::ControlModifier) || (modifiers & Qt::AltModifier))
        return bRet(ok, true,
                    "\\press{\"" + QKeySequence(key | modifiers).toString(QKeySequence::PortableText) + "\"}");
    text.replace('%', "\\%");
    text.replace('\t', "\\t");
    text.replace('\n', "\\n");
    return bRet(ok, true, "\\insert{\"" + text + "\"}");
}
