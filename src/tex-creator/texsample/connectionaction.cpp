/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
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

#include "connectionaction.h"

#include "application.h"

#include <BDirTools>

#include <QIcon>
#include <QLabel>
#include <QMovie>
#include <QObject>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QWidgetAction>

/*============================================================================
================================ ConnectionAction ============================
============================================================================*/

/*============================== Public constructors =======================*/

ConnectionAction::ConnectionAction(QObject *parent) :
    QWidgetAction(parent)
{
    //
}

/*============================== Public methods ============================*/

void ConnectionAction::resetIcon(const QString &toolTip, const QString &iconName, bool animated)
{
    setToolTip(toolTip);
    setIcon(Application::icon(iconName));
    foreach (QWidget *wgt, createdWidgets()) {
        if (QString("QLabel") != wgt->metaObject()->className()) {
            QToolButton *tbtn = static_cast<QToolButton *>(wgt);
            tbtn->setIcon(Application::icon(iconName));
            tbtn->setToolTip(toolTip);
            QLabel *lbl = tbtn->findChild<QLabel *>();
            if (animated) {
                tbtn->setText("");
                tbtn->setIcon(QIcon());
                lbl->setVisible(true);
                lbl->movie()->start();
            } else {
                lbl->movie()->stop();
                lbl->setVisible(false);
            }
        }
    }
}

/*============================== Protected methods =========================*/

QWidget *ConnectionAction::createWidget(QWidget *parent)
{
    if (!parent || QString("QMenu") == parent->metaObject()->className())
        return 0;
    QToolButton *tbtn = new QToolButton(parent);
      tbtn->setMenu(this->menu());
      tbtn->setPopupMode(QToolButton::InstantPopup);
      tbtn->setLayout(new QVBoxLayout);
      tbtn->layout()->setContentsMargins(0, 0, 0, 0);
        QLabel *lbl = new QLabel(tbtn);
        lbl->setAlignment(Qt::AlignCenter);
        QMovie *mov = new QMovie(BDirTools::findResource("icons/process.gif", BDirTools::GlobalOnly));
        mov->setScaledSize(tbtn->iconSize());
        lbl->setMovie(mov);
      tbtn->layout()->addWidget(lbl);
    return tbtn;
}

void ConnectionAction::deleteWidget(QWidget *widget)
{
    if (!widget)
        return;
    widget->deleteLater();
}
