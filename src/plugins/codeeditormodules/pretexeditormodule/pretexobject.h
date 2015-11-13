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

#ifndef PRETEXOBJECT_H
#define PRETEXOBJECT_H

class BAbstractCodeEditorDocument;

#include <QObject>
#include <QScriptable>
#include <QScriptValue>
#include <QString>
#include <QVariantMap>

/*============================================================================
================================ RecordingModule =============================
============================================================================*/

class PretexObject : public QObject, protected QScriptable
{
    Q_OBJECT
private:
    BAbstractCodeEditorDocument *mdoc;
public:
    explicit PretexObject(BAbstractCodeEditorDocument *doc = 0);
public slots:
    bool find(const QScriptValue &what, const QVariantMap &options = QVariantMap());
    QScriptValue getInput(const QString &mode, const QString &label = QString(), const QString &title = QString());
    void insert(const QString &text);
    void log(const QString &what);
    void press(const QString &sequence, int count = 1);
    QString readFile(const QString &fileName, const QString &codecName = QString());
    int replace(const QScriptValue &what, const QString &newText, const QVariantMap &options = QVariantMap());
    QString run(const QString &command, const QVariantMap &options = QVariantMap());
    bool runDetached(const QString &command, const QVariantMap &options = QVariantMap());
    void showMessage(const QString &text, const QString &type = QString(), const QString &title = QString());
};

#endif // PRETEXOBJECT_H
