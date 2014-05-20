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

#include "recordingmodule.h"

RecordingModule::RecordingModule()
{
}

/*AbstractMacroCommand *AbstractMacroCommand::fromKeyPressEvent(const QKeyEvent *e, QString *error,
                                                              AbstractMacroCommand *previousCommand)
{
    if (!e || e->type() != QEvent::KeyPress)
        return bRet(error, QString("The event is not a key press"), (AbstractMacroCommand *) 0);
    int key = e->key();
    Qt::KeyboardModifiers modifiers = e->modifiers();
    QString text = e->text();
    if (key <= 0)
        return bRet(error, QString("Invalid key press"), (AbstractMacroCommand *) 0);
    if (Qt::Key_Control == key || Qt::Key_Alt == key || Qt::Key_Shift == key)
        return bRet(error, QString("The event is a modifier key press"), (AbstractMacroCommand *) 0);
    if ((modifiers & Qt::ControlModifier) && (modifiers & Qt::ShiftModifier) && (modifiers & Qt::AltModifier)
            && (modifiers & Qt::MetaModifier))
        return bRet(error, QString("The event is a locale change key press"), (AbstractMacroCommand *) 0);
    if (!(modifiers & Qt::ControlModifier) && !(modifiers & Qt::AltModifier) && Qt::Key_Return == key)
        text = "\n";
    if (text.isEmpty() || (!text.at(0).isPrint() && text.at(0) != '\n')
            || (modifiers & Qt::ControlModifier) || (modifiers & Qt::AltModifier))
        return bRet(error, QString(), new PressMacroCommand(key, modifiers));
    if (!previousCommand || previousCommand->name() != "insert")
        return bRet(error, QString(), new InsertMacroCommand(Global::toVisibleText(text)));
    if (!dynamic_cast<InsertMacroCommand *>(previousCommand)->append(Global::toVisibleText(text)))
        return bRet(error, QString(), new InsertMacroCommand(Global::toVisibleText(text)));
    return bRet(error, QString(), previousCommand);
}*/

/*
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
  */

/*============================================================================
================================ SpontaneousEventEater =======================
============================================================================*/

/*class SpontaneousEventEater : public QObject
{
public:
    explicit SpontaneousEventEater(BAbstractCodeEditorDocument *doc);
public:
    bool eventFilter(QObject *o, QEvent *e);
private:
    QPlainTextEdit *mptedt;
};*/

/*============================================================================
================================ SpontaneousEventEater =======================
============================================================================*/

/*============================== Public constructors =======================*/

/*SpontaneousEventEater::SpontaneousEventEater(BAbstractCodeEditorDocument *doc)
{
    mptedt = doc ? doc->findChild<QPlainTextEdit *>() : 0;
    if (!mptedt)
        return;
    mptedt->installEventFilter(this);
    mptedt->viewport()->installEventFilter(this);
}*/

/*============================== Public methods ============================*/

/*bool SpontaneousEventEater::eventFilter(QObject *o, QEvent *e)
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
}*/
