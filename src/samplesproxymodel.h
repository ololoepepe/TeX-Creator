#ifndef SAMPLESPROXYMODEL_H
#define SAMPLESPROXYMODEL_H

class SamplesModel;

class QVariant;
class QModelIndex;
class QString;

#include <QSortFilterProxyModel>
#include <QStringList>

/*============================================================================
================================ SamplesProxyModel ===========================
============================================================================*/

class SamplesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum ExtraSampleType
    {
        CurrentUserSample = -1
    };
public:
    explicit SamplesProxyModel(QObject *parent = 0);
public:
    QVariant data(const QModelIndex &index, int role) const;
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    void setSourceModel(QAbstractItemModel *sourceModel);
#endif
    void setSampleType(int type);
    void setSearchKeywords(const QStringList &list);
public slots:
    void setSearchKeywordsString(const QString &string);
protected:
    bool filterAcceptsColumn(int column, const QModelIndex &parent) const;
    bool filterAcceptsRow(int row, const QModelIndex &parent) const;
private slots:
    void sourceModelChangedSlot();
private:
    SamplesModel *msamplesModel;
    int msampleType;
    QStringList msearchKeywords;
private:
    Q_DISABLE_COPY(SamplesProxyModel)
};

#endif // SAMPLESPROXYMODEL_H
