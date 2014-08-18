/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
**
** This file is part of TeX Creator.
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

#ifndef SYMBOLSWIDGET_H
#define SYMBOLSWIDGET_H

class QSignalMapper;
class QWidget;

#include <QStringList>
#include <QTabWidget>

/*============================================================================
================================ SymbolsWidget ===============================
============================================================================*/

class SymbolsWidget : public QTabWidget
{
    Q_OBJECT
private:
    QSignalMapper *mmpr;
    QStringList mtexts;
public:
    explicit SymbolsWidget(QWidget *parent = 0);
private:
    void loadSection(int lbound, int ubound);
    QString sectionTitle(int index) const;
private slots:
    void retranslateUi();
signals:
    void insertText(const QString &text);
private:
    Q_DISABLE_COPY(SymbolsWidget)
};

#endif // SYMBOLSWIDGET_H
