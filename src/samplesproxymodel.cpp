#include "samplesproxymodel.h"
#include "samplesmodel.h"
#include "sample.h"
#include "client.h"
#include "application.h"
#include "texsamplesettingstab.h"

#include <BeQtGlobal>

#include <QSortFilterProxyModel>
#include <QVariant>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QRegExp>

/*============================================================================
================================ SamplesProxyModel ===========================
============================================================================*/

/*============================== Public constructors =======================*/

SamplesProxyModel::SamplesProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    msamplesModel = 0;
    msampleType = Sample::Approved;
    connect( this, SIGNAL( sourceModelChanged() ), SLOT( sourceModelChangedSlot() ) );
    connect( sClient, SIGNAL( loginChanged(QString) ), this, SLOT( invalidate() ) );
}

/*============================== Public methods ============================*/

QVariant SamplesProxyModel::data(const QModelIndex &index, int role) const
{
    if (Qt::ToolTipRole == role)
        return data(index, Qt::DisplayRole);
    else
        return QSortFilterProxyModel::data(index, role);
}

void SamplesProxyModel::setSampleType(int type)
{
    if (type < CurrentUserSample || type > Sample::Rejected)
        return;
    msampleType = type;
    invalidate();
}

void SamplesProxyModel::setSearchKeywords(const QStringList &list)
{
    msearchKeywords = list;
    msearchKeywords.removeAll("");
    msearchKeywords.removeDuplicates();
    invalidate();
}

/*============================== Public slots ==============================*/

void SamplesProxyModel::setSearchKeywordsString(const QString &string)
{
    static QRegExp rx("\\,\\s*");
    setSearchKeywords( string.split(rx, QString::SkipEmptyParts) );
}

/*============================== Protected methods =========================*/

bool SamplesProxyModel::filterAcceptsColumn(int column, const QModelIndex &) const
{
    return msamplesModel && bRange(1, 2).contains(column);
}

bool SamplesProxyModel::filterAcceptsRow(int row, const QModelIndex &) const
{
    const Sample *s = msamplesModel ? msamplesModel->sample(row) : 0;
    if (!s)
        return false;
    if (CurrentUserSample == msampleType)
        return s->author() == TexsampleSettingsTab::getLogin();
    else
        return (s->type() == msampleType) && s->matchesKeywords(msearchKeywords);
}

/*============================== Private slots =============================*/

void SamplesProxyModel::sourceModelChangedSlot()
{
    msamplesModel = static_cast<SamplesModel *>( sourceModel() );
}
