#ifndef SAMPLESMODEL_H
#define SAMPLESMODEL_H

#include "sample.h"

class QStringList;
class QVariant;

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QString>
#include <QList>
#include <QMap>

/*============================================================================
================================ SamplesModel ================================
============================================================================*/

class SamplesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SamplesModel(QObject *parent = 0);
public:
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void setSamples(const QList<Sample> &list);
    void insertSample(const Sample &s);
    void insertSamples(const QList<Sample> &list);
    void removeSample(quint64 id);
    void removeSamples(const QList<quint64> &list);
    void removeSamples(const QStringList &list);
    void clear();
    const Sample* sample(int index) const;
    const Sample* sample(quint64 id) const;
    QList<const Sample *> samples() const;
    quint64 indexAt(int row) const;
    bool isEmpty() const;
private slots:
    void retranslateUi();
private:
    QList<Sample> msamples;
    QMap<quint64, Sample *> msampleMap;
private:
    Q_DISABLE_COPY(SamplesModel)
};

#endif // SAMPLESMODEL_H
