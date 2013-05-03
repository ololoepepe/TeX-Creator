#ifndef SAMPLESMODEL_H
#define SAMPLESMODEL_H

class QStringList;
class QVariant;

#include <TSampleInfo>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QString>
#include <QList>
#include <QMap>

#define sModel SamplesModel::instance()

/*============================================================================
================================ SamplesModel ================================
============================================================================*/

class SamplesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    static SamplesModel *instance();
public:
    explicit SamplesModel(QObject *parent = 0);
public:
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void insertSample(const TSampleInfo &s);
    void insertSamples(const TSampleInfo::SamplesList &list);
    void removeSample(quint64 id);
    void removeSamples(const QList<quint64> &list);
    void clear();
    const TSampleInfo* sample(int index) const;
    const TSampleInfo* sample(quint64 id) const;
    const QList<TSampleInfo> *samples() const;
    quint64 indexAt(int row) const;
    bool isEmpty() const;
private slots:
    void retranslateUi();
private:
    static SamplesModel *minstance;
private:
    QList<TSampleInfo> msamples;
    QMap<quint64, TSampleInfo *> msampleMap;
private:
    Q_DISABLE_COPY(SamplesModel)
};

#endif // SAMPLESMODEL_H
