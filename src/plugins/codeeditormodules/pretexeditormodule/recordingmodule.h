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

#ifndef RECORDINGMODULE_H
#define RECORDINGMODULE_H

class BAbstractCodeEditorDocument;

class QEvent;
class QKeyEvent;
class QString;

#include <QObject>
#include <QStringList>

/*============================================================================
================================ RecordingModule =============================
============================================================================*/

class RecordingModule : public QObject
{
    Q_OBJECT
public:
    explicit RecordingModule(QObject *parent = 0);
    explicit RecordingModule(BAbstractCodeEditorDocument *doc, QObject *parent = 0);
public:
    bool eventFilter(QObject *o, QEvent *e);
    void setDocument(BAbstractCodeEditorDocument *doc);
    void startRecording();
    void stopRecording();
    BAbstractCodeEditorDocument *document() const;
    bool isValid() const;
    bool isRecording() const;
    QStringList commands() const;
private:
    static QString commandFromKeyPress(QKeyEvent *e, bool *ok = 0);
private:
    bool mrecording;
    BAbstractCodeEditorDocument *mdoc;
    QStringList mcommands;
};

#endif // RECORDINGMODULE_H
