#include "samplesmodel.h"
#include "sample.h"

#include <BApplication>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QString>

#include <QDebug>

/*============================================================================
================================ SamplesModel ================================
============================================================================*/

/*============================== Public constructors =======================*/

SamplesModel::SamplesModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    connect( bApp, SIGNAL( languageChanged() ), this, SLOT( retranslateUi() ) );
}

/*============================== Public methods ============================*/

int SamplesModel::rowCount(const QModelIndex &) const
{
    return msamples.size();
}

int SamplesModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant SamplesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || Qt::DisplayRole != role)
        return QVariant();
    const Sample *s = sample( index.row() );
    if (!s)
        return QVariant();
    switch ( index.column() )
    {
    case 0:
        return s->id();
    case 1:
        return s->title();
    case 2:
        return s->author();
    default:
        return QVariant();
    }
}

QVariant SamplesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::Horizontal != orientation || Qt::DisplayRole != role)
        return QVariant();
    switch (section)
    {
    case 1:
        return tr("Title", "headerData");
    case 2:
        return tr("Author", "headerData");
    default:
        return QVariant();
    }
}

void SamplesModel::setSamples(const QList<Sample> &list)
{
    clear();
    insertSamples(list);
}

void SamplesModel::insertSample(const Sample &s)
{
    insertSamples(QList<Sample>() << s);
}

void SamplesModel::insertSamples(const QList<Sample> &list)
{
    if ( list.isEmpty() )
        return;
    QList<Sample> nlist = list;
    foreach ( int i, bRange(nlist.size() - 1, 0, -1) )
    {
        const Sample &s = nlist.at(i);
        if ( !s.isValid() )
            nlist.removeAt(i);
        else if ( msampleMap.contains( s.id() ) )
            removeSample( s.id() );
    }
    if ( nlist.isEmpty() )
        return;
    int ind = msamples.size();
    beginInsertRows(QModelIndex(), ind, ind + nlist.size() - 1);
    foreach (const Sample &s, nlist)
    {
        msamples.append(s);
        msampleMap.insert( s.id(), &msamples.last() );
    }
    endInsertRows();
}

void SamplesModel::removeSample(quint64 id)
{
    if ( !id || !msampleMap.contains(id) )
        return;
    Sample *s = msampleMap.take(id);
    int ind = msamples.indexOf(*s);
    beginRemoveRows(QModelIndex(), ind, ind);
    msamples.removeAt(ind);
    endRemoveRows();
}

void SamplesModel::removeSamples(const QList<quint64> &list)
{
    foreach (const quint64 &s, list)
        removeSample(s);
}

void SamplesModel::removeSamples(const QStringList &list)
{
    QList<quint64> idlist;
    foreach (const QString ids, list)
    {
        bool ok = false;
        quint64 id = ids.toULongLong(&ok);
        if (ok)
            idlist << id;
    }
    removeSamples(idlist);
}

void SamplesModel::clear()
{
    if ( msamples.isEmpty() )
        return;
    beginRemoveRows(QModelIndex(), 0, msamples.size() - 1);
    msampleMap.clear();
    msamples.clear();
    endRemoveRows();
}

const Sample *SamplesModel::sample(int index) const
{
    return ( index >= 0 && index < msamples.size() ) ? &msamples.at(index) : 0;
}

const Sample *SamplesModel::sample(quint64 id) const
{
    return id ? msampleMap.value(id) : 0;
}

QList<const Sample *> SamplesModel::samples() const
{
    QList<const Sample *> list;
    foreach( const Sample *s, msampleMap.values() )
        list << s;
    return list;
}

quint64 SamplesModel::indexAt(int row) const
{
    const Sample *s = sample(row);
    return s ? s->id() : 0;
}

bool SamplesModel::isEmpty() const
{
    return msamples.isEmpty();
}

/*============================== Private slots =============================*/

void SamplesModel::retranslateUi()
{
    headerDataChanged(Qt::Horizontal, 1, 2);
}
